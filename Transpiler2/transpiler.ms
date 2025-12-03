import "stringUtil"
import "listUtil"
import "mapUtil"
import "fancyMatch"

//--------------------------------------------------------------------------------
// Constants
TAB = char(9)

Scope = {}						// Used for VariableInfo.scope and MethodInfo.scope
Scope.PUBLIC = "public"
Scope.PROTECTED = "protected"
Scope.PRIVATE = "private"
Scope.LOCAL = "local"

ClassType = {}					// Used for ClassInfo.type
ClassType.CLASS = "class"
ClassType.STRUCT = "struct"
ClassType.ENUM = "enum"
ClassType.INTERFACE = "interface"

State = {}						// used for Transpiler.state
State.START = "START"			// at start of file, before namespace
State.NAMESPACE = "NAMESPACE"	// within the namespace, but not in a class/struct/etc.
State.CLASS = "CLASS"			// within a class definition
State.METHOD = "METHOD"			// within a function/method definition
State.DONE = "DONE"				// past the end of the namespace (pretty much done)

//--------------------------------------------------------------------------------
// Global data & methods
debugMode = false

debugPrint = function(s)
	if debugMode then print s
end function

fixStringInterpolation = function(s)
	// Look for C#-style string interpolation, and replace it with a
	// call to our Interp function.  Example:
	//		$"{greeting}, {name}!"  -->  Interp("{}, {}!", greeting, name)
	pos0 = -1
	while true
		pos0 = s.indexOf("$""", pos0)
		if pos0 == null then return s
		pos1 = s.indexOf("""", pos0+1)
		while pos1 and s[pos1-1] == "\"
			pos1 = s.indexOf("""", pos1)
		end while
		content = s[pos0+2:pos1]  // string content, not including quotes
		print "content: " + content
		args = [content]
		i = -1
		while true
			i = content.indexOf("{", i)
			if i == null then break
			i2 = content.indexOf("}", i)
			args.push content[i+1:i2]
			content = content[:i+1] + content[i2:]
		end while
		args[0] = """" + content + """"
		revisedContent = "Interp(" + args.join(", ") + ")"
		print "revisedContent: " + revisedContent
		s = s[:pos0] + revisedContent + s[pos1+1:]
	end while
	return s
end function

//--------------------------------------------------------------------------------
Line = {}
Line.full = ""					// full text of line
Line.m = null					// match map (set by .match method)

Line.indentation = function
	line = self.full
	i = 0
	while i < line.len and (line[i] == " " or line[i] == TAB)
		i += 1
	end while
	self.indentation = line[:i]  // memoize on the instance
	return self.indentation
end function

Line.comment = function
	line = self.full
	i = 0
	comment = ""
	inQuote = false
	while i < line.len-2
		if line[i] == """" then inQuote = not inQuote
		if line[i:i+2] == "//" and not inQuote then
			comment = line[i:]
			break
		end if
		i += 1
	end while
	self.comment = comment  // memoize on the instance
	return comment
end function

Line.trimmed = function
	comment = self.comment
	if comment.len > 0 then
		line = self.full[self.indentation.len : -comment.len]
	else
		line = self.full[self.indentation.len:]
	end if
	line = line.trimRight
	self.trimmed = line  // again, memoize
	return line
end function

Line.match = function(pattern, againstFull=false)
	if againstFull then s = self.full else s = self.trimmed
	self.m = s.match(pattern)
	return self.m != null
end function

Line.Make = function(fullLine)
	noob = new Line
	noob.full = fullLine
	return noob
end function

//--------------------------------------------------------------------------------
ClassInfo = {}
ClassInfo.name = ""
ClassInfo.type = ClassType.CLASS
ClassInfo.fields = null			// list of VariableInfo
ClassInfo.methods = null		// list of MethodInfo
ClassInfo.declLines = null		// class declaration lines (header file)
ClassInfo.inlineLines = null	// inline definition lines (header file)
ClassInfo.defLines = null		// out-of-line definition lines (cpp file)
ClassInfo.methodLines = null	// reference to either .inlineLines or .defLines
ClassInfo.nextMethodInline = false

ClassInfo.Make = function(name="")
	noob = new ClassInfo
	noob.name = name
	noob.fields = []
	noob.methods = []
	noob.declLines = []
	noob.inlineLines = []
	noob.defLines = []
	return noob
end function

//--------------------------------------------------------------------------------
VariableInfo = {}				// used for fields, parameters, and local vars
VariableInfo.name = ""
VariableInfo.type = ""
VariableInfo.scope = ""			// Scope.PUBLIC, etc. (not used for 

VariableInfo.Make = function(name="")
	noob = new VariableInfo
	noob.name = name
	return noob
end function

//--------------------------------------------------------------------------------
MethodInfo = {}
MethodInfo.name = ""
MethodInfo.type = "void"		// (return type)
MethodInfo.scope = ""			// Scope.PUBLIC, etc.
MethodInfo.params = null		// list of VariableInfo
MethodInfo.inline = false

MethodInfo.Make = function(name="")
	noob = new MethodInfo
	noob.name = name
	return noob
end function


//--------------------------------------------------------------------------------
Transpiler = {}
Transpiler.state = ""
Transpiler.gatherMode = false	// if true, just gathering symbols
Transpiler.sourcePath = ""		// file path to .cs file we're processing
Transpiler.outputFolder = ""	// path to folder for output files
Transpiler.namespace = ""		// namespace we're processing
Transpiler.preLines = null		// list of lines (usually comments) before method/field
Transpiler.line = null			// current line we're processing
Transpiler.fileClasses = null	// list of ClassInfo we're processing (in this file)
Transpiler.curClass = null		// ClassInfo that's currently being processed
Transpiler.curMethod = null		// MethodInfo that's currently being processed
Transpiler.headerLines = null	// extra lines for top of header file
Transpiler.cppLines = null		// extra lines for top of .cpp file
Transpiler.endLines = null		// end of namespace, and anything that comes after

Transpiler.sourceName = function; return file.name(self.sourcePath); end function

// Transpiler.match: match the given pattern against our current line; if found,
// copy self.line.m to self.m (for convenience), and also fill in className and
// inline.
Transpiler.match = function(pattern)
	if self.line.match(pattern) then
		self.m = self.line.m
		if self.curClass then
			self.m.className = self.curClass.name
			if self.curClass.nextMethodInline then
				self.m.inline = "inline "
			else
				self.m.inline = ""
			end if
		end if
		return true
	end if
	self.line.m = null
	self.m = null
	return false
end function

// Transpiler.fill: fill the given pattern with values from self.m, and 
// optionally append the comment, if the current line has one.
Transpiler.fill = function(pattern, addComment=true)
	result = pattern.fill(self.m)
	if addComment and self.line.comment then result += " " + self.line.comment
	return result
end function

Transpiler.enterState = function(newState, initAsNew=true)
	if newState == self.state then return
	if self.state then self.exitState newState
	debugPrint "Entering state: " + newState
	self.state = newState
	
	if newState == State.START then
		self.preLines = []
		self.line = ""
		self.fileClasses = []
		self.headerLines = []
		self.cppLines = []
		self.endLines = []
		self.curClass = null
		self.curMethod = null
	
	else if newState == State.NAMESPACE then
		self.curClass = null
		
	else if newState == State.CLASS then
		if initAsNew then
			if self.line and self.line.m and self.line.m.hasIndex("name") then
				name = self.line.m.name
			else
				name = "?"
			end if
			self.curClass = ClassInfo.Make(name)
			self.fileClasses.push self.curClass
			self.curClass.declLines += self.preLines
			self.preLines = []
			self.curClass.defLines.push ""
		end if
		self.curClass.nextMethodInline = false

	else if newState == State.METHOD then
		if self.line and self.line.m and self.line.m.hasIndex("name") then
			name = self.line.m.name
		else
			name = "?"
		end if
		self.curMethod = MethodInfo.Make(name)
		if self.curClass.nextMethodInline then
			self.curClass.methodLines = self.curClass.inlineLines
		else
			self.curClass.methodLines = self.curClass.defLines
		end if

	end if
	
end function

Transpiler.exitState = function(toNewState)
	debugPrint "Exiting state: " + self.state
end function

Transpiler.updateState = function
	
	if self.state == State.START then
		self.updateStateSTART
	else if self.state == State.NAMESPACE then
		self.updateStateNAMESPACE
	else if self.state == State.CLASS then
		self.updateStateCLASS
	else if self.state == State.METHOD then
		self.updateStateMETHOD
	else if self.state == State.DONE then
		self.endLines.push self.line.full	// ToDo: support H: and CPP: comments here?
	end if
	
end function

Transpiler.updateStateSTART = function
	line = self.line
	comment = self.line.comment

	if comment.startsWith("// CPP: ") then
		self.cppLines.push comment[8:]
	else if comment.startsWith("// H: ") then
		self.headerLines.push comment[6:]
	else if line.trimmed == "" then
		self.headerLines.push line.full
	else if self.match("namespace ≤name≥ {") then
		self.headerLines.push line.full
		self.namespace = line.m.name
		self.enterState State.NAMESPACE
	else
		print "Ignored in state START: " + line.full
	end if
end function

Transpiler.updateStateNAMESPACE = function
	line = self.line
	comment = self.line.comment

	if comment.startsWith("// CPP: ") then
		self.cppLines.push comment[8:]
	else if comment.startsWith("// H: ") then
		self.headerLines.push comment[6:]
	else if line.trimmed == "" then
		self.preLines.push line.full
	else if self.match("≤scope:w≥ static class ≤name:w≥ {") then
		self.enterState State.CLASS
		self.curClass.declLines.push line.full
	else if self.match("≤scope:w≥ struct ≤name:w≥ {") then
		self.enterState State.CLASS
		self.curClass.type = ClassType.STRUCT
		self.curClass.declLines.push line.full
	else if self.match("≤scope:w≥ enum ≤name:w≥ : ≤type≥ {") then
		self.enterState State.CLASS
		self.curClass.type = ClassType.ENUM
		self.curClass.declLines.push line.full		
	else if line.full.startsWith("}") then
		if not line.comment then line.comment = "// end of namespace " + self.namespace
		self.endLines.push "} " + line.comment
		self.enterState State.DONE
	end if
end function

Transpiler.updateStateCLASS = function
	line = self.line
	comment = self.line.comment

	if line.full.startsWith("}") then
		self.curClass.declLines.push line.full
		self.enterState State.NAMESPACE
		return
	end if
	
	if line.trimmed.startsWith("[") then
		// decorator -- most are ignored, but check for inline flag
		if line.trimmed.contains("(AggressiveInlining)") then
			self.curClass.nextMethodInline = true
		end if
		return
	end if
	
	if self.curClass.type == ClassType.ENUM then
		// enum value, with/without numeric value and with/without comma
		if self.match("≤name:w≥ = ≤value:w≥,") or self.match("≤name:w≥,") or
		   self.match("≤name:w≥ = ≤value:w≥")  or self.match("≤name:w≥") then
			self.curClass.declLines.push line.full
		end if
	else
		// constant
		if self.match("≤scope:w≥ const ≤type:w≥ ≤name:w≥ = ≤value≥;") then
			self.curClass.declLines.push self.fill("\t≤scope≥: static const ≤type≥ ≤name≥;")
			self.curClass.defLines.push self.fill("const ≤type≥ ≤className≥::≤name≥ = ≤value≥;")
		// ordinary field
		else if self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥;") then
			self.curClass.declLines.push self.fill("\t≤scope≥: ≤type≥ ≤name≥;")
		// constructor method (no return type)
		else if self.match("≤scope:w≥ ≤name:w≥(≤params≥) {") then
			self.enterState State.METHOD
			self.curClass.declLines.push self.fill("\t≤scope≥: ≤name≥(≤params≥);")
			self.curClass.methodLines.push self.fill("≤inline≥≤className≥::≤name≥(≤params≥) {")
		// standard method
		else if self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥(≤params≥) {") then
			self.enterState State.METHOD
			self.curClass.declLines.push self.fill("\t≤scope≥: ≤type≥ ≤name≥(≤params≥);")
			self.curClass.methodLines.push self.fill("≤inline≥≤type≥ ≤className≥::≤name≥(≤params≥) {")
		end if
	end if
end function

Transpiler.updateStateMETHOD = function
	line = self.line
	comment = self.line.comment

	// outdent method lines by 1 tab
	if line.full.startsWith(TAB) then line.full = line.full[1:]
	
	if line.full.startsWith("}") then
		self.curClass.methodLines.push line.full
		self.enterState State.CLASS, false
		return
	end if
	
	// anything else, apply standard transformations, then write out
	cline = line.full
	cline = fixStringInterpolation(cline)
	self.curClass.methodLines.push cline

end function

Transpiler.generateOutput = function
	headerLines = []
	cppLines = []
	
	// Generate the header
	headerLines.push "// AUTO-GENERATED FILE.  DO NOT MODIFY."
	headerLines.push "// Transpiled from: " + self.sourceName
	headerLines.push ""
	headerLines.push "#pragma once"
	headerLines.push "#include ""core_includes.h"""

	headerLines += self.headerLines
	headerLines.push ""
	headerLines.push "// DECLARATIONS"
	headerLines.push ""
	for class in self.fileClasses
		headerLines += class.declLines
		headerLines.push ""
	end for
	headerLines.push ""
	headerLines.push "// INLINE METHODS"
	headerLines.push ""
	for class in self.fileClasses
		headerLines += class.inlineLines
		headerLines.push ""
	end for
	headerLines += self.endLines
	
	// Generate the cpp file
	cppLines.push "// AUTO-GENERATED FILE.  DO NOT MODIFY."
	cppLines.push "// Transpiled from: " + self.sourceName
	cppLines.push ""
	cppLines += self.cppLines
	cppLines.push ""
	cppLines.push "namespace " + self.namespace + " {"
	cppLines.push ""
	for class in self.fileClasses
		cppLines += class.defLines
		cppLines.push ""
	end for
	cppLines.push ""
	cppLines.push "} // end of namespace " + self.namespace

	baseName = self.sourceName - ".cs"
	headerPath = file.child(self.outputFolder, baseName + ".g.h")
	cppPath = file.child(self.outputFolder, baseName + ".g.cpp")
	file.writeLines headerPath, headerLines
	debugPrint "Wrote " + headerLines.len + " lines to " + headerPath
	file.writeLines cppPath, cppLines
	debugPrint "Wrote " + cppLines.len + " lines to " + cppPath
	
end function

Transpiler.processLine = function(line)
	self.line = Line.Make(line)
	
	state = self.state
	line = self.line.trimmed
	comment = self.line.comment
	debugPrint "[≤state≥] ≤line≥ ≤comment≥".fill(locals)
	self.updateState	
end function

Transpiler.process = function(filePath, gatherMode=false)
	debugPrint "Processing: " + filePath
	self.sourcePath = filePath
	self.outputFolder = file.child(file.parent(filePath), "generated")
	self.gatherMode = gatherMode
	self.state = ""
	self.enterState State.START
	for line in file.readLines(filePath)
		self.processLine line
	end for
	if not gatherMode then self.generateOutput
	debugPrint "Done processing: " + filePath
end function

runUnitTests = function
	count = 0
	check = function(actual, expected)
		outer.count += 1
		if actual == expected then return
		print "UNIT TEST FAILED."
		print "EXPECTED: " + expected
		print "  ACTUAL: " + actual
		exit
	end function
	
	check fixStringInterpolation("No interp here"), "No interp here"
	
	check fixStringInterpolation(
	  "x = $""{greeting}, {name}!"";"),
	  "x = Interp(""{}, {}!"", greeting, name);"

	print "All " + count + " unit tests completed successfully."
end function


if locals == globals then
	debugMode = true
	
	if debugMode then runUnitTests
	
	t = new Transpiler
	if shellArgs[1][0] == "/" then
		dir = shellArgs[1]
	else
		dir = file.child(env.MS_SCRIPT_DIR, shellArgs[1])
	end if
	print "Processing directory: " + dir
	for filename in file.children(dir)
		if not filename.endsWith(".cs") or filename == "main.cs" then continue
		t.process file.child(dir, filename)
	end for
end if
