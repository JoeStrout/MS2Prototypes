// Program.cs
using System;
using System.IO;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;

class Program {
    static async System.Threading.Tasks.Task Main(string[] args) {
        var input = args.Length > 0 ? args[0] : "tests/Sample.cs";
        
        // Single-file quick path (no MSBuild)
        var text = await File.ReadAllTextAsync(input);
        var treeSingle = CSharpSyntaxTree.ParseText(text, new CSharpParseOptions(LanguageVersion.Preview));
        // Add Mini stub types as source
        var miniStubText = await File.ReadAllTextAsync("MiniStubs.cs");
        var miniStubTree = CSharpSyntaxTree.ParseText(miniStubText, new CSharpParseOptions(LanguageVersion.Preview));
        
        var compSingle = CSharpCompilation.Create("InMem",
            new[] { treeSingle, miniStubTree },
            new[] {
                MetadataReference.CreateFromFile(typeof(object).Assembly.Location),
                MetadataReference.CreateFromFile(typeof(System.Collections.Generic.List<>).Assembly.Location),
            },
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary));
        var modelSingle = compSingle.GetSemanticModel(treeSingle);
        var rootSingle = await treeSingle.GetRootAsync();

        var subset2 = new PortableSubsetChecker(modelSingle);
        subset2.Visit(rootSingle);
        subset2.ThrowIfErrors();

        var rewriter2 = new CppRewriter(modelSingle);
        var rewritten2 = rewriter2.Visit(rootSingle);

        var cpp2 = new CppEmitter(rewriter2.Symbols).EmitTranslationUnit(null, rewritten2);
        var outputPath = "generated/Sample.g.cpp";
        await File.WriteAllTextAsync(outputPath, cpp2);
        Console.WriteLine($"Wrote {outputPath}");
    }
}
