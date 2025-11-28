import "stringUtil"
import "listUtil"
import "mapUtil"

//--------------------------------------------------------------------------------
// Constants
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
Transpiler.prelines = null		// list of lines (usually comments) before method/field
Transpiler.line = ""			// current line we're processing
Transpiler.fileClasses = null	// list of ClassInfo we're processing (in this file)
Transpiler.headerLines = null	// extra lines for top of header file

Transpiler.sourceName = function; return file.name(self.sourcePath); end function

Transpiler.enterState = function(newState)
	if newState == self.state then return
	if self.state then self.exitState toNewState
	debugPrint "Entering state: " + newState
	self.state = newState
	
	if newState == State.START then
		self.prelines = []
		self.line = ""
		self.fileClasses = []
		self.headerLines = []
	end if
end function

Transpiler.exitState = function(toNewState)
	debugPrint "Exiting state: " + self.state
end function

Transpiler.updateState = function

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
	
	// Generate the cpp file
	cppLines.push "// AUTO-GENERATED FILE.  DO NOT MODIFY."
	cppLines.push "// Transpiled from: " + self.sourceName
	cppLines.push ""
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
	self.line = line
	debugPrint "[{state}] {line}".fill(self)
	
	
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
