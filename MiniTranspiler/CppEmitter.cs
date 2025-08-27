using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

sealed class CppEmitter {
    private readonly SymbolTable _symbols;
    public CppEmitter(SymbolTable symbols) { _symbols = symbols; }

    public string EmitTranslationUnit(Document? doc, SyntaxNode root) {
        var sb = new StringBuilder();
        sb.AppendLine("// generated — DO NOT EDIT");
        sb.AppendLine("#include \"mini_runtime.hpp\"");
        foreach (var cls in root.DescendantNodes().OfType<ClassDeclarationSyntax>()) {
            EmitClass(sb, cls);
        }
        return sb.ToString();
    }

	void EmitClass(StringBuilder sb, ClassDeclarationSyntax cls) {
		var name = cls.Identifier.Text;
		
		// Emit any leading comments/trivia before the class
		var leadingTrivia = cls.GetLeadingTrivia().ToFullString().Trim();
		if (!string.IsNullOrEmpty(leadingTrivia)) {
			sb.AppendLine(leadingTrivia);
		}
		
		sb.AppendLine($"class {name} {{");
		
		// Group fields by access modifier
		var fields = cls.Members.OfType<FieldDeclarationSyntax>().ToList();
		var publicFields = new List<FieldDeclarationSyntax>();
		var privateFields = new List<FieldDeclarationSyntax>();
		
		foreach (var f in fields) {
			if (f.Modifiers.Any(m => m.IsKind(SyntaxKind.PrivateKeyword))) {
				privateFields.Add(f);
			} else {
				publicFields.Add(f); // default to public if no explicit modifier
			}
		}
		
		// Emit public fields
		if (publicFields.Count > 0) {
			sb.AppendLine("public:");
			foreach (var f in publicFields) {
				var type = f.Declaration.Type.ToString();
				foreach (var variable in f.Declaration.Variables) {
					var fieldName = variable.Identifier.Text;
					var initializer = variable.Initializer?.Value.ToString();
					if (initializer != null) {
						sb.AppendLine($"{type} {fieldName} = {initializer};");
					} else {
						sb.AppendLine($"{type} {fieldName};");
					}
				}
			}
		}
		
		// Emit private fields  
		if (privateFields.Count > 0) {
			sb.AppendLine("private:");
			foreach (var f in privateFields) {
				var type = f.Declaration.Type.ToString();
				foreach (var variable in f.Declaration.Variables) {
					var fieldName = variable.Identifier.Text;
					var initializer = variable.Initializer?.Value.ToString();
					if (initializer != null) {
						sb.AppendLine($"{type} {fieldName} = {initializer};");
					} else {
						sb.AppendLine($"{type} {fieldName};");
					}
				}
			}
		}
	
		// Auto-prop backing fields (private)
		var props = cls.Members.OfType<PropertyDeclarationSyntax>().ToList();
		if (props.Count > 0) {
			sb.AppendLine("private:");
			foreach (var p in props) {
				if (IsAutoProperty(p)) {
					// For rewritten properties, just use the declared type from syntax
					var propType = p.Type.ToString();
					sb.AppendLine($"{propType} __prop_{p.Identifier.Text};");
				}
			}
			sb.AppendLine("public:");
		}
	
		// Methods
		foreach (var m in cls.Members.OfType<MethodDeclarationSyntax>())
			EmitMethod(sb, m);
	
		// Property accessors → methods
		foreach (var p in props) EmitPropertyAsMethods(sb, p, name);
	
		sb.AppendLine("};");
		sb.AppendLine();
	}
	
	bool IsAutoProperty(PropertyDeclarationSyntax p)
		=> p.AccessorList != null &&
		   p.AccessorList.Accessors.All(a => a.Body == null && a.ExpressionBody == null);
	
	void EmitPropertyAsMethods(StringBuilder sb, PropertyDeclarationSyntax p, string owner) {
		// Use syntax-based approach to avoid semantic model issues with rewritten trees
		var tname = p.Type.ToString();
		// GET
		var getAcc = p.AccessorList?.Accessors.FirstOrDefault(a => a.Kind() == SyntaxKind.GetAccessorDeclaration);
		if (getAcc != null) {
			if (IsAutoProperty(p)) {
				sb.AppendLine($"inline {tname} get_{p.Identifier.Text}() const {{ return __prop_{p.Identifier.Text}; }}");
			} else {
				// emit body as-is
				var body = getAcc.ExpressionBody?.Expression.ToFullString()
						   ?? getAcc.Body?.ToFullString() ?? "{ /*missing*/ }";
				if (getAcc.ExpressionBody != null) body = "{ return " + body + "; }";
				sb.AppendLine($"inline {tname} {owner}::get_{p.Identifier.Text}() const {body}");
			}
		}
		// SET
		var setAcc = p.AccessorList?.Accessors.FirstOrDefault(a => a.Kind() == SyntaxKind.SetAccessorDeclaration);
		if (setAcc != null) {
			if (IsAutoProperty(p)) {
				sb.AppendLine($"inline void set_{p.Identifier.Text}({tname} value) {{ __prop_{p.Identifier.Text} = value; }}");
			} else {
				var body = setAcc.ExpressionBody?.Expression.ToFullString()
						   ?? setAcc.Body?.ToFullString() ?? "{ /*missing*/ }";
				if (setAcc.ExpressionBody != null) body = "{ " + body + "; }";
				sb.AppendLine($"inline void {owner}::set_{p.Identifier.Text}({tname} value) {body}");
			}
		}
	}

    void EmitMethod(StringBuilder sb, MethodDeclarationSyntax m) {
        // Use syntax-based approach
        var ret = m.ReturnType.ToString();
        var name = m.Identifier.Text;
        var parms = string.Join(", ", m.ParameterList.Parameters.Select(p => $"{p.Type} {p.Identifier}"));
        var className = ((ClassDeclarationSyntax)m.Parent!).Identifier.Text;
        sb.AppendLine($"{ret} {className}::{name}({parms}) {{");

        // naive: emit body tokens as-is (after rewrites) for a first pass
        sb.AppendLine("  // body");
        sb.AppendLine("  " + m.Body?.ToFullString().Trim().Replace("\n", "\n  ") ?? "{}");
        sb.AppendLine("}");
    }

}
