using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

sealed class CppRewriter : CSharpSyntaxRewriter {
    public readonly SymbolTable Symbols = new();
    private readonly SemanticModel _model;
    public CppRewriter(SemanticModel model) { _model = model; }

    public override SyntaxNode? VisitPropertyDeclaration(PropertyDeclarationSyntax node) {
        // Convert auto-props to get_/set_ pair in symbol table; leave syntax mostly intact.
        var propSym = _model.GetDeclaredSymbol(node) as IPropertySymbol;
        if (propSym is null) return base.VisitPropertyDeclaration(node);
        Symbols.Properties.Add(propSym, new(PropertyToGetter(propSym), PropertyToSetter(propSym)));
        return node; // keep original formatting for analysis; emitter will consult symbol table
    }


	public override SyntaxNode? VisitVariableDeclaration(VariableDeclarationSyntax node) {
		if (node.Type.IsVar) {
			if (node.Variables.Count != 1 || node.Variables[0].Initializer is null)
				throw new Exception("Portable: 'var' requires a single declarator with initializer.");
			var t = _model.GetTypeInfo(node.Variables[0].Initializer!.Value).ConvertedType!;
			var typeName = t.ToDisplayString(SymbolDisplayFormat.MinimallyQualifiedFormat);
			var newType = SyntaxFactory.ParseTypeName(typeName).WithTriviaFrom(node.Type);
			return base.VisitVariableDeclaration(node.WithType(newType));
		}
		return base.VisitVariableDeclaration(node);
	}


	static bool IsMiniListOrMap(ITypeSymbol? t)
		=> t is INamedTypeSymbol nt && (nt.Name is "List" or "Map") &&
		   nt.ContainingNamespace.ToDisplayString().Contains("Mini"); // adjust
	
	public override SyntaxNode? VisitIdentifierName(IdentifierNameSyntax node) {
		// Handle simple property references like "Consts" -> "__prop_Consts"
		var sym = _model.GetSymbolInfo(node).Symbol;
		if (sym is IPropertySymbol p && p.ContainingType.Name == "Bytecode") { // adjust as needed
			// Map to backing field: Consts -> __prop_Consts
			var backingField = $"__prop_{p.Name}";
			var fieldRef = SyntaxFactory.IdentifierName(backingField);
			return fieldRef.WithTriviaFrom(node);
		}
		return base.VisitIdentifierName(node);
	}

	public override SyntaxNode? VisitMemberAccessExpression(MemberAccessExpressionSyntax node) {
		// First, check if the left side (Expression) is a property that needs rewriting
		var newExpression = (ExpressionSyntax?)Visit(node.Expression) ?? node.Expression;
		
		// Check if this member access itself is a property
		var sym = _model.GetSymbolInfo(node).Symbol;
		if (sym is IPropertySymbol p) {
			if (p.Name == "Count" && IsMiniListOrMap(p.ContainingType)) {
				// Rewrite: someProperty.Count -> someProperty.Count()
				var newNode = node.WithExpression(newExpression);
				var call = SyntaxFactory.InvocationExpression(
					newNode.WithName(SyntaxFactory.IdentifierName("Count")));
				return call.WithTriviaFrom(node);
			}
			var getName = $"get_{p.Name}";
			var newNode2 = node.WithExpression(newExpression);
			var call2 = SyntaxFactory.InvocationExpression(
				newNode2.WithName(SyntaxFactory.IdentifierName(getName)));
			return call2.WithTriviaFrom(node);
		}
		
		// If just the left side changed, return the updated node
		if (newExpression != node.Expression) {
			return node.WithExpression(newExpression);
		}
		
		return base.VisitMemberAccessExpression(node);
	}
	
	public override SyntaxNode? VisitAssignmentExpression(AssignmentExpressionSyntax node) {
		if (node.Kind() == SyntaxKind.SimpleAssignmentExpression &&
			node.Left is MemberAccessExpressionSyntax ma &&
			_model.GetSymbolInfo(ma).Symbol is IPropertySymbol p) {
	
			// obj.set_Name(rhs)
			var setName = $"set_{p.Name}";
			var call = SyntaxFactory.InvocationExpression(
				ma.WithName(SyntaxFactory.IdentifierName(setName)),
				SyntaxFactory.ArgumentList(SyntaxFactory.SingletonSeparatedList(
					SyntaxFactory.Argument((ExpressionSyntax)Visit(node.Right)!))));
			// As an expression, replace with the call; subset checker forbids using its value.
			return call.WithTriviaFrom(node);
		}
		return base.VisitAssignmentExpression(node);
	}

    static string PropertyToGetter(IPropertySymbol p) => $"get_{p.Name}";
    static string PropertyToSetter(IPropertySymbol p) => $"set_{p.Name}";
    
    private int _tempCounter = 0;
	private string Fresh(string pfx) => $"__{pfx}{_tempCounter++}";
	
	private bool TryGetListElementType(ExpressionSyntax expr, out ITypeSymbol elemType) {
		var t = _model.GetTypeInfo(expr).ConvertedType as INamedTypeSymbol;
		if (t != null && t.Name == "List" &&
			t.ContainingNamespace.ToDisplayString().Contains("Mini") &&
			t.TypeArguments.Length == 1) { elemType = t.TypeArguments[0]; return true; }
		elemType = _model.Compilation.GetSpecialType(SpecialType.System_Object);
		return false;
	}
	
	public override SyntaxNode? VisitForEachStatement(ForEachStatementSyntax node) {
		if (!TryGetListElementType(node.Expression, out var elem))
			throw new Exception("Portable: foreach supported only over Mini.List<T>.");
	
		var idx = Fresh("i");
		var n   = Fresh("n");
		var coll = (ExpressionSyntax)Visit(node.Expression)!; // rewrite inside expr too
		var elemTypeName = elem.ToDisplayString(SymbolDisplayFormat.MinimallyQualifiedFormat);
	
		// Build:
		// for (int __i=0, __n=coll.Count(); __i<__n; ++__i) {
		//   <ElemType> <id> = coll[__i];
		//   /* body */
		// }
		var header = SyntaxFactory.ParseStatement(
			$"for (int {idx}=0, {n}={coll}.Count(); {idx}<{n}; ++{idx}) {{}}");
	
		var forStmt = (ForStatementSyntax)header;
		var visitedBody = (StatementSyntax)(Visit(node.Statement) ?? node.Statement); // rewrite body first
		
		// Try to extract indentation from the original foreach body
		var bodyIndent = "";
		if (visitedBody is BlockSyntax block && block.Statements.Count > 0) {
			var firstStmt = block.Statements[0];
			var leadingTrivia = firstStmt.GetLeadingTrivia().ToFullString();
			// Extract just the whitespace from the beginning
			var lines = leadingTrivia.Split('\n');
			if (lines.Length > 0) {
				var lastLine = lines[lines.Length - 1];
				bodyIndent = new string(' ', lastLine.TakeWhile(c => c == ' ').Count());
			}
		}
		if (string.IsNullOrEmpty(bodyIndent)) {
			bodyIndent = "          "; // fallback: 10 spaces
		}
		
		var decl = SyntaxFactory.ParseStatement(
			$"{bodyIndent}{elemTypeName} {node.Identifier.Text} = {coll}[{idx}];\n");
	
		var newBody = visitedBody is BlockSyntax b
			? b.WithStatements(b.Statements.Insert(0, ((LocalDeclarationStatementSyntax)decl)))
			: SyntaxFactory.Block((LocalDeclarationStatementSyntax)decl, visitedBody);
	
		return forStmt.WithStatement(newBody).WithTriviaFrom(node);
	}

}

sealed class SymbolTable {
    public readonly Dictionary<IPropertySymbol, (string getter, string setter)> Properties = new(SymbolEqualityComparer.Default);
}
