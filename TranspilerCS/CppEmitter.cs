using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

sealed class CppEmitter {
    private readonly SymbolTable _symbols;
    public CppEmitter(SymbolTable symbols) { _symbols = symbols; }

    public (string header, string implementation) EmitTranslationUnit(Document? doc, SyntaxNode root) {
        var headerSb = new StringBuilder();
        var implSb = new StringBuilder();
        
        // Header file
        headerSb.AppendLine("// generated — DO NOT EDIT");
        headerSb.AppendLine("#pragma once");
        headerSb.AppendLine("#include \"mini_runtime.hpp\"");
        headerSb.AppendLine();
        
        // Implementation file  
        implSb.AppendLine("// generated — DO NOT EDIT");
        implSb.AppendLine("#include \"Sample.g.h\""); // TODO: derive from input filename
        implSb.AppendLine();
        
        foreach (var cls in root.DescendantNodes().OfType<ClassDeclarationSyntax>()) {
            EmitClass(headerSb, implSb, cls);
        }
        
        return (headerSb.ToString(), implSb.ToString());
    }

	void EmitClass(StringBuilder headerSb, StringBuilder implSb, ClassDeclarationSyntax cls) {
		var name = cls.Identifier.Text;
		
		// === HEADER: Class declaration ===
		
		// Emit any leading comments/trivia before the class
		var leadingTrivia = cls.GetLeadingTrivia().ToFullString().Trim();
		if (!string.IsNullOrEmpty(leadingTrivia)) {
			headerSb.AppendLine(leadingTrivia);
		}
		
		headerSb.AppendLine($"class {name} {{");
		
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
		
		// Emit public fields in header
		if (publicFields.Count > 0) {
			headerSb.AppendLine("public:");
			foreach (var f in publicFields) {
				var type = f.Declaration.Type.ToString();
				foreach (var variable in f.Declaration.Variables) {
					var fieldName = variable.Identifier.Text;
					var initializer = variable.Initializer?.Value.ToString();
					if (initializer != null) {
						headerSb.AppendLine($"    {type} {fieldName} = {initializer};");
					} else {
						headerSb.AppendLine($"    {type} {fieldName};");
					}
				}
			}
		}
		
		// Emit private fields in header
		if (privateFields.Count > 0) {
			headerSb.AppendLine("private:");
			foreach (var f in privateFields) {
				var type = f.Declaration.Type.ToString();
				foreach (var variable in f.Declaration.Variables) {
					var fieldName = variable.Identifier.Text;
					var initializer = variable.Initializer?.Value.ToString();
					if (initializer != null) {
						headerSb.AppendLine($"    {type} {fieldName} = {initializer};");
					} else {
						headerSb.AppendLine($"    {type} {fieldName};");
					}
				}
			}
		}
	
		// Auto-prop backing fields (private) in header
		var props = cls.Members.OfType<PropertyDeclarationSyntax>().ToList();
		if (props.Count > 0) {
			headerSb.AppendLine("private:");
			foreach (var p in props) {
				if (IsAutoProperty(p)) {
					var propType = p.Type.ToString();
					headerSb.AppendLine($"    {propType} __prop_{p.Identifier.Text};");
				}
			}
			headerSb.AppendLine("public:");
		}
	
		// Method declarations in header
		foreach (var m in cls.Members.OfType<MethodDeclarationSyntax>()) {
			EmitMethodDeclaration(headerSb, m);
		}
	
		// Property accessor declarations in header
		foreach (var p in props) {
			EmitPropertyDeclarations(headerSb, p);
		}
	
		headerSb.AppendLine("};");
		headerSb.AppendLine();
		
		// === IMPLEMENTATION: Method definitions ===
		
		foreach (var m in cls.Members.OfType<MethodDeclarationSyntax>()) {
			EmitMethodImplementation(implSb, m, name);
		}
	}
	
	bool IsAutoProperty(PropertyDeclarationSyntax p)
		=> p.AccessorList != null &&
		   p.AccessorList.Accessors.All(a => a.Body == null && a.ExpressionBody == null);

    void EmitMethodDeclaration(StringBuilder sb, MethodDeclarationSyntax m) {
        // Use syntax-based approach
        var ret = m.ReturnType.ToString();
        var name = m.Identifier.Text;
        var parms = string.Join(", ", m.ParameterList.Parameters.Select(p => $"{p.Type} {p.Identifier}"));
        sb.AppendLine($"    {ret} {name}({parms});");
    }

    void EmitMethodImplementation(StringBuilder sb, MethodDeclarationSyntax m, string className) {
        // Use syntax-based approach
        var ret = m.ReturnType.ToString();
        var name = m.Identifier.Text;
        var parms = string.Join(", ", m.ParameterList.Parameters.Select(p => $"{p.Type} {p.Identifier}"));
        sb.AppendLine($"{ret} {className}::{name}({parms}) {{");

        // naive: emit body tokens as-is (after rewrites) for a first pass
        sb.AppendLine("  // body");
        sb.AppendLine("  " + m.Body?.ToFullString().Trim().Replace("\n", "\n  ") ?? "{}");
        sb.AppendLine("}");
    }

	void EmitPropertyDeclarations(StringBuilder sb, PropertyDeclarationSyntax p) {
		// Use syntax-based approach to avoid semantic model issues with rewritten trees
		var tname = p.Type.ToString();
		// GET
		var getAcc = p.AccessorList?.Accessors.FirstOrDefault(a => a.Kind() == SyntaxKind.GetAccessorDeclaration);
		if (getAcc != null) {
			if (IsAutoProperty(p)) {
				sb.AppendLine($"    inline {tname} get_{p.Identifier.Text}() const {{ return __prop_{p.Identifier.Text}; }}");
			} else {
				sb.AppendLine($"    {tname} get_{p.Identifier.Text}() const;");
			}
		}
		// SET
		var setAcc = p.AccessorList?.Accessors.FirstOrDefault(a => a.Kind() == SyntaxKind.SetAccessorDeclaration);
		if (setAcc != null) {
			if (IsAutoProperty(p)) {
				sb.AppendLine($"    inline void set_{p.Identifier.Text}({tname} value) {{ __prop_{p.Identifier.Text} = value; }}");
			} else {
				sb.AppendLine($"    void set_{p.Identifier.Text}({tname} value);");
			}
		}
	}
}