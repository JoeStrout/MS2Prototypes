using System;
using System.Collections.Generic;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

sealed class PortableSubsetChecker : CSharpSyntaxWalker {
    private readonly SemanticModel _model;
    private readonly List<string> _errors = new();
    public PortableSubsetChecker(SemanticModel model) : base(SyntaxWalkerDepth.StructuredTrivia) { _model = model; }

    void Err(SyntaxNode n, string msg) => _errors.Add($"{n.GetLocation().GetLineSpan().StartLinePosition}: {msg}");

    public void ThrowIfErrors() {
        if (_errors.Count > 0) throw new Exception("Portable subset violations:\n - " + string.Join("\n - ", _errors));
    }

    public override void VisitInvocationExpression(InvocationExpressionSyntax node) {
        // Example: ban LINQ
        var sym = _model.GetSymbolInfo(node).Symbol as IMethodSymbol;
        if (sym?.ContainingNamespace.ToDisplayString().StartsWith("System.Linq") == true)
            Err(node, "LINQ is not allowed in portable code.");
        base.VisitInvocationExpression(node);
    }

    public override void VisitAttribute(AttributeSyntax node) {
        var sym = _model.GetSymbolInfo(node).Symbol;
        if (sym?.ContainingType.Name is "ObsoleteAttribute" or "DynamicAttribute")
            Err(node, "Obsolete/Dynamic attributes not allowed.");
        base.VisitAttribute(node);
    }

    public override void VisitYieldStatement(YieldStatementSyntax node) => Err(node, "yield not allowed.");
    public override void VisitAnonymousMethodExpression(AnonymousMethodExpressionSyntax node) => Err(node, "anonymous methods not allowed.");
    public override void VisitUnsafeStatement(UnsafeStatementSyntax node) => Err(node, "unsafe not allowed.");
    public override void VisitLockStatement(LockStatementSyntax node) => Err(node, "lock not allowed (use your VM sync APIs).");
    public override void VisitCheckedStatement(CheckedStatementSyntax node) => Err(node, "checked/unchecked not allowed.");

	public override void VisitAssignmentExpression(AssignmentExpressionSyntax node) {
		// Ban compound assignments on properties, e.g. obj.X += 1, ++obj.X
		if (node.Left is MemberAccessExpressionSyntax ma) {
			var sym = _model.GetSymbolInfo(ma).Symbol;
			if (sym is IPropertySymbol && node.Kind() != SyntaxKind.SimpleAssignmentExpression)
				Err(node, "Only simple assignment (=) allowed on properties.");
		}
		base.VisitAssignmentExpression(node);
	}
	
	public override void VisitPostfixUnaryExpression(PostfixUnaryExpressionSyntax node) {
		if (node.Operand is MemberAccessExpressionSyntax && _model.GetSymbolInfo(node.Operand).Symbol is IPropertySymbol)
			Err(node, "++/-- on properties not allowed.");
		base.VisitPostfixUnaryExpression(node);
	}
	public override void VisitPrefixUnaryExpression(PrefixUnaryExpressionSyntax node) {
		if (node.Operand is MemberAccessExpressionSyntax && _model.GetSymbolInfo(node.Operand).Symbol is IPropertySymbol)
			Err(node, "++/-- on properties not allowed.");
		base.VisitPrefixUnaryExpression(node);
	}

    // Add more bans as needed (async/await, dynamic, stackalloc, etc.)
}
