// This program transpiles (a limited subset of) C# code into C++.

import "listUtil"
import "stringUtil"
import "fancyMatch"  // overrides standard string.match and string.fill

TAB = char(9)

Context = {}
Context.START = "START"  // before or after namespace, file scope
Context.NAMESPACE = "NAMESPACE"  // within a namespace, but not a class
Context.CLASS = "CLASS"  // within a class, but not within a method
Context.METHOD = "METHOD" // within a method

Converter = {}
Converter.context = Context.START
Converter.codeBlockDepth = 0  // code blocks *within* a method
Converter.hLines = null
Converter.cppLines = null
Converter.addCurly = false
Converter.className = ""
Converter.classNames = null // all class names we know about
Converter.didIncludes = false
Converter.failed = false
Converter.lineNum = 0
Converter.line = ""
Converter.addCurly = false
Converter.csOnly = false  // when true, skip conversion entirely

Converter.Make = function
	noob = new self
	noob.classNames = ["string", "List<string>"]
	return noob
end function

Converter.match = function(inp, hOut, cppOut)
	self.m = self.line.match(inp)
	if not self.m then return false
	self.m.className = self.className
	print "--> " + inp
	curly = " {" * self.addCurly
	if cppOut then
		self.cppLines.push self.indentation + cppOut.fill(self.m) + curly + self.comment
	end if
	if hOut then
		if hOut[-1] == ";" then curly = "" // header file may have ; instead of {
		self.hLines.push self.indentation + hOut.fill(self.m) + curly + self.comment
	end if
	return true
end function

Converter.fail = function(errMsg)
	print "ERROR: " + errMsg
	print "on line " + self.lineNum + ": " + self.line + " {" * self.addCurly
	self.failed = true
end function

Converter.processLine = function(line)
	// Start by splitting off the comment, if any
	// (ToDo: lex this properly in a string-literal-savvy way)
	parts = line.split("//", 2)
	if parts.len == 2 then
		line = parts[0]
		comment = "//" + parts[1]
	else
		comment = ""
	end if
	
	// Also split off leading whitespace (indentation)
	indentation = ""
	while line and (line[0] == " " or line[0] == TAB)
		indentation += line[0]
		line = line[1:]
	end while

	// If our comment starts with "// CPP:", then ignore the rest of the line
	// and substitute what appears after that in the CPP code.
	if comment.startsWith("// CPP:") then
		line = comment[7:].trimLeft
		self.cppLines.push indentation + line
		return
	end if
	
	// Check for other special control comments
	if comment == "//*** BEGIN CS_ONLY ***" then
		self.csOnly = true
	else if comment == "//*** END CS_ONLY ***" then
		self.csOnly = false
		return
	end if
	if self.csOnly then return

	// If it's an empty line, or just a comment, then push and we're done
	if not line then
		if self.context != Context.METHOD then self.hLines.push indentation + comment
		self.cppLines.push indentation + comment
		return
	end if
	
	// Trim trailing whitespace
	while line and (line[-1] == " " or line[-1] == TAB)
		comment = line[-1] + comment
		line = line[:-1]
	end while
	
	print line
	
	// And split off an opening curly too
	if line[-1] == "{" then
		self.addCurly = true
		line = line[:-1]
		line = line.trimRight
		if self.context == Context.METHOD then self.codeBlockDepth += 1
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
	else
		self.addCurly = false
	end if
	
	// Now, process the line
	self.line = line
	self.comment = comment
	self.indentation = indentation
	
	// Enforce coding standards
	self.enforceCodeStandards line + " {" * self.addCurly
	if self.failed then return
	
	// Standard syntax transformations
	self.line = self.tweakSyntax(line)
	
	// using: ignored, but our chance to output standard includes
	if self.match("using ≤lib≥;", "", "") then
		if not self.didIncludes then
			self.hLines.push "#include ""core_includes.h"""
			self.cppLines.push "#include """ + (self.filename - ".cs" + ".g.h") + """"
			if self.filename == "Program.cs" then  // ToDo: find better way to decide this
				self.cppLines.push "#include ""IOHelper.g.h"""
			end if
			self.didIncludes = true
		end if
		return
	end if
	  
	// namespace line
	if self.match("namespace ≤name≥", "namespace ""≤name≥""", "namespace ""≤name≥""") then
		self.context = Context.NAMESPACE
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	// class declaration
	if self.match("≤qualifiers≥ class ≤name:w≥", "class ≤name≥", "") or
	  self.match("class ≤name≥", "class ≤name≥", "") then
		self.context = Context.CLASS
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		self.className = self.m.name
		if not self.classNames.contains(self.className) then
			self.classNames.push self.className
		end if
		print "Got class name: " + self.className + "; all classes: " + self.classNames
		return
	end if
	
	// enum declaration (becomes a class in C++)
	if self.match("≤scope≥ enum ≤name≥ : ≤type≥", "enum class ≤name≥ : ≤type≥", "") then
		self.context = Context.CLASS
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		self.className = self.m.name
		if not self.classNames.contains(self.className) then
			self.classNames.push self.className
		end if
		print "Got class (enum) name: " + self.className + "; all classes: " + self.classNames
		return
	end if
	
	// class field, with and without default value
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥;",
	  "≤scope≥: ≤type≥ ≤name≥;", "") then return
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥ = ≤value≥;",
	  "≤scope≥: ≤type≥ ≤name≥ = ≤value≥;", "") then return

	// class property with expression body
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥ => ≤expr≥;",
	  "≤scope≥: ≤type≥ ≤name≥() { return ≤expr≥; }", "") then
		// ToDo: note ≤name≥ in this class as something that will need () where used.
		return
	end if
	
	// expression-bodied method
	if self.match("≤scope:w≥ static ≤rtype:w≥ ≤name:w≥(≤params≥) => ≤expr≥;",
	  "≤scope≥: static ≤rtype≥ ≤name≥(≤params≥) { return ≤expr≥; }", "") then
		return
	end if

	if self.match("≤scope:w≥ ≤rtype:w≥ ≤name:w≥(≤params≥) => ≤expr≥;",
	  "≤scope≥: ≤rtype≥ ≤name≥(≤params≥) { return ≤expr≥; }", "") then
		return
	end if

	// constructor declaration
	if self.match("≤scope:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: ≤name≥(≤params≥);",
	  "≤className≥::≤name≥(≤params≥)") then
		self.context = Context.METHOD
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	// method declaration
	if self.match("≤scope:w≥ static ≤rtype:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: static ≤rtype≥ ≤name≥(≤params≥);",
	  "≤rtype≥ ≤className≥::≤name≥(≤params≥)") then
		self.context = Context.METHOD
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	if self.match("≤scope:w≥ ≤rtype:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: ≤rtype≥ ≤name≥(≤params≥);",
	  "≤rtype≥ ≤className≥::≤name≥(≤params≥)") then
	  	if self.m.scope == "static" then return self.fail("scope qualifier required on static method")
		self.context = Context.METHOD
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
	// foreach statement
	if self.match("foreach (≤type:w≥ ≤var:w≥ in ≤seq≥)", "",
	  "for (int __≤var≥_idx=0, __≤var≥_qty=≤seq≥.Count(); __≤var≥_idx < __≤var≥_qty; __≤var≥_idx++)") then
		self.cppLines.push indentation + TAB + 
		  "≤type≥ ≤var≥ = ≤seq≥[__≤var≥_idx];".fill(self.m)
		return
	end if
	  
	// Anything else: copy to header and/or CPP file according to context
	curly = " {" * self.addCurly
	fullLine = self.indentation + self.line + curly + self.comment
	self.cppLines.push fullLine
	if self.context != Context.METHOD then self.hLines.push fullLine
	
	// Closing curly: close the open code block or scope
	if self.line == "}" then
		if self.context == Context.METHOD and self.codeBlockDepth > 0 then
			self.codeBlockDepth -= 1
		else if self.context == Context.METHOD then
			self.context = Context.CLASS
		else if self.context == Context.CLASS then
			// add ; after class declaration in .h file; omit from .cpp file
			self.hLines[-1] = self.hLines[-1].replace("}", "}; // end of class " + self.className)
			self.cppLines.pop // no class curlies in cpp file!
			self.context = Context.NAMESPACE
		else
			self.context = Context.START
		end if
		print "Context down: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
	end if

end function

// Converter.tweakSyntax: do simple syntax transformations that can be done
// within pretty much any line of code.
Converter.tweakSyntax = function(line)
	// array syntax (especially as seen in parameters)
	line = line.replace("[] ", " *")
	
	// static class methods (invoked on the class name itself)
	for cls in self.classNames
		line = line.replace(cls+".", cls+"::")
	end for
	
	// Split to list
	m = line.match("≤pre≥.Split(≤args≥).ToList()≤post≥")
	if m then line = "≤pre≥.SplitToList(≤args≥)≤post≥".fill(m)
	
	return line
end function

// Converter.enforceCodeStandards: examine the given line for things that a
// C# programmer might do, that we can't convert or just don't want to allow.
Converter.enforceCodeStandards = function(line)
	if line == " {" then
		self.fail "Opening brace on its own line is disallowed"
	else if line.endsWith("=>") or line.startsWith("=>") then
		self.fail "Expression-bodied properties or methods must be all on one line"
	end if
end function

Converter.convertClass = function(sourcePath)
	self.hLines = []
	self.cppLines = []
	self.didIncludes = false
	self.filename = file.name(sourcePath)
	self.lineNum = 0
	self.failed = false

	for line in file.readLines(sourcePath)
		self.lineNum += 1
		//print line
		self.processLine line
		if self.failed then return
	end for

	if self.className == "Program" then
		// main-program boilerplate goes with the program
		self.cppLines.push ""
		self.cppLines.push "int main() {"
		self.cppLines.push TAB + "Program::Main(nullptr);"
		self.cppLines.push "}"
	end if

	
	fileNameBase = file.name(sourcePath) - ".cs"
	outFolder = "generated/"
	print "-----------------------------------"
	print "HEADER:"
	print self.hLines.join(char(10))
	file.writeLines outFolder + fileNameBase + ".g.h", self.hLines
	print
	print "-----------------------------------"
	print "CPP:"
	print self.cppLines.join(char(10))
	file.writeLines outFolder + fileNameBase + ".g.cpp", self.cppLines
	print
	
	
end function

if locals == globals then
	cvt = Converter.Make
//	cvt.convertClass "tests/IOHelper.cs"
//	if not cvt.failed then cvt.convertClass "tests/Program.cs"
	cvt.convertClass "tests/VM.cs"
end if
