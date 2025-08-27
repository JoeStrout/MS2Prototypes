// This program transpiles (a limited subset of) C# code into C++.

import "stringUtil"

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

Converter.match = function(inp, hOut, cppOut)
	self.m = self.line.match(inp)
	if not self.m then return false
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
	if not line then
		if self.context != Context.METHOD then self.hLines.push indentation + comment
		self.cppLines.push indentation + comment
		return
	end if
	
	// And trailing whitespace
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
	
	// using --> #include
	if self.match("using {lib};", 
	  "#include ""{lib}.h""", 
	  "#include ""{lib}.h""") then return
	  
	// namespace line
	if self.match("namespace {name}",
	  "namespace ""{name}""",
	  "namespace ""{name}""") then
		self.context = Context.NAMESPACE
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	// class declaration
	if self.match("{qualifiers} class {name}",
	  "class {name}", "class {name}") then
		self.context = Context.CLASS
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
	// class field, with and without default value
	if self.context == Context.CLASS and self.match("{scope} {type} {name};",
	  "{scope}: {type} {name};", "") then return
	if self.context == Context.CLASS and self.match("{scope} {type} {name} = {value};",
	  "{scope}: {type} {name} = {value};", "") then return

	// class property with expression body
	if self.context == Context.CLASS and self.match("{scope} {type} {name} => {expr};",
	  "{scope}: {type} {name}() ⟪ {expr} ⟫", "") then
		// ToDo: note {name} in this class as something that will need () where used.
		// Also, maybe we can make a more elegant solution for using curly braces than:
		self.hLines[-1] = self.hLines[-1].replace("⟪", "{").replace("⟫", "}")
		return
	end if

	// method declaration
	if self.match("{scope} {rtype} {name}({params})",
	  "{scope}: {rtype} {name}({params});",
	  "{rtype} {name}({params})") then
		self.context = Context.METHOD
		print "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
	// foreach statement
	if self.match("foreach ({type} {var} in {seq})", "",
	  "for (int __{var}_idx=0, __{var}_qty={seq}.Count(); __{var}_idx < __{var}_qty; __{var}_idx++)") then
		self.cppLines.push indentation + TAB + 
		  "{type} {var} = {seq}[__{var}_idx];".fill(self.m)
		return
	end if
	  
	// Anything else: copy to header and/or CPP file according to context
	curly = " {" * self.addCurly
	fullLine = self.indentation + line + curly + self.comment
	self.cppLines.push fullLine
	if self.context != Context.METHOD then self.hLines.push fullLine
	
	// Closing curly: close the open code block or scope
	if line == "}" then
		if self.context == Context.METHOD and self.codeBlockDepth > 0 then
			self.codeBlockDepth -= 1
		else if self.context == Context.METHOD then
			self.context = Context.CLASS
		else if self.context == Context.CLASS then
			self.context = Context.NAMESPACE
		else
			self.context = Context.START
		end if
		print "Context down: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
	end if

end function
	
Converter.convertClass = function(sourcePath)
	self.hLines = []
	self.cppLines = []
	for line in file.readLines(sourcePath)
		//print line
		self.processLine line
	end for
	
	fileNameBase = file.name(sourcePath) - ".cs"
	outFolder = "generated/"
	print "HEADER:"
	print self.hLines.join(char(10))
	file.writeLines outFolder + fileNameBase + ".g.h", self.hLines
	print
	print "CPP:"
	print self.cppLines.join(char(10))
	file.writeLines outFolder + fileNameBase + ".g.cpp", self.cppLines
	print
	
end function

if locals == globals then
	cvt = new Converter
	cvt.convertClass "tests/Sample.cs"
end if
