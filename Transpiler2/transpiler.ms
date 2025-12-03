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

//--------------------------------------------------------------------------------
ClassInfo = {}
ClassInfo.name = ""
ClassInfo.type = ""				// ClassType.CLASS, etc.
ClassInfo.fields = null			// list of VariableInfo
ClassInfo.methods = null		// list of MethodInfo
ClassInfo.declLines = null		// class declaration lines (header file)
ClassInfo.inlineLines = null	// inline definition lines (header file)
ClassInfo.defLines = null		// out-of-line definition lines (cpp file)

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
Transpiler.preLines = null		// list of lines (usually comments) before method/field
Transpiler.line = ""			// current line we're processing
Transpiler.fileClasses = null	// list of ClassInfo we're processing (in this file)
Transpiler.curClass = null		// ClassInfo that's currently being processed
Transpiler.headerLines = null	// extra lines for top of header file
Transpiler.cppLines = null		// extra lines for top of .cpp file
Transpiler.endLines = null		// end of namespace, and anything that comes after

Transpiler.sourceName = function; return file.name(self.sourcePath); end function

Transpiler.enterState = function(newState)
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
	end if
	
	if newState == State.NAMESPACE then
		self.curClass = null
	end if
	
end function

Transpiler.exitState = function(toNewState)
	debugPrint "Exiting state: " + self.state
end function

Transpiler.updateState = function
	line = self.line
	comment = self.line.comment
	print "comment: " + comment
	
	if self.state == State.START then
		if comment.startsWith("// CPP: ") then; self.cppLines.push comment[8:]; return; end if
		if comment.startsWith("// H: ") then; self.headerLines.push comment[6:]; return; end if
		if line.trimmed == "" then; self.headerLines.push line.full; return; end if
		if line.trimmed.startsWith("namespace ") then
			self.headerLines.push line.full
			self.enterState State.NAMESPACE
		end if
		return
	end if
	
	if self.state == State.NAMESPACE then
		if comment.startsWith("// CPP: ") then; self.cppLines.push comment[8:]; return; end if
		if comment.startsWith("// H: ") then; self.headerLines.push comment[6:]; return; end if
		if line.trimmed == "" then; self.preLines.push line.full; return; end if
		m = line.trimmed.match("≤scope:w≥ static class ≤name:w≥ {")
		if m then
			self.enterState State.CLASS
			self.curClass = ClassInfo.Make(m.name)
			self.fileClasses.push self.curClass
			self.curClass.declLines += self.preLines
			self.preLines = []
			self.curClass.declLines.push line.full
			self.curClass.defLines.push ""
		end if
		if line.full.startsWith("}") then
			self.endLines.push line.full
			self.enterState State.DONE
		end if
		return
	end if

	if self.state == State.CLASS then
		if line.full.startsWith("}") then
			self.curClass.declLines.push line.full
			self.enterState State.NAMESPACE
		end if
		m = line.trimmed.match("≤scope:w≥ const ≤type:w≥ ≤name:w≥ = ≤value≥;")
		if m then
			m.className = self.curClass.name
			self.curClass.declLines.push "\t≤scope≥: static const ≤type≥ ≤name≥;".fill(m)
			self.curClass.defLines.push "const ≤type≥ ≤className≥::≤name≥ = ≤value≥;".fill(m)
		end if
		return
	end if

	if self.state == State.DONE then
		self.endLines.push line.full	// ToDo: support H: and CPP: comments here?
	end if
	
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
	print "Got " + self.cppLines.len + " cppLines"
	cppLines += self.cppLines
	for class in self.fileClasses
		cppLines += class.defLines
		cppLines.push ""
	end for

	baseName = self.sourceName - ".cs"
	headerPath = file.child(self.outputFolder, baseName + ".g.h")
	cppPath = file.child(self.outputFolder, baseName + ".g.cpp")
	file.writeLines headerPath, headerLines
	debugPrint "Wrote " + headerLines.len + " lines to " + headerPath
	file.writeLines cppPath, cppLines
	debugPrint "Wrote " + cppLines.len + " lines to " + cppPath
	
end function

Transpiler.processLine = function(line)
	self.line = {}
	self.line.full = line
	
	// Split off the comment, if any
	// (ToDo: lex this properly in a string-literal-savvy way)
	parts = line.split("//", 2)
	if parts.len == 2 then
		line = parts[0]
		self.line.comment = "//" + parts[1]
	else
		self.line.comment = ""
	end if
	
	// Also split off leading whitespace (indentation)
	self.line.indentation = ""
	while line and (line[0] == " " or line[0] == TAB)
		self.line.indentation += line[0]
		line = line[1:]
	end while
	
	// And keep the trimmed line
	self.line.trimmed = line.trim
	
	state = self.state
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

if locals == globals then
	debugMode = true
	// Current test case:
	t = new Transpiler
	dir = env.MS_SCRIPT_DIR + "/tests/staticStringConst"
	t.process file.child(dir, "Op.cs")
end if
