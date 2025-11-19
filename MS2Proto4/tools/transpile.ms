// This program transpiles (a limited subset of) C# code into C++.

import "listUtil"
import "stringUtil"
import "mapUtil"
import "fancyMatch"  // overrides standard string.match and string.fill

TAB = char(9)

debugPrint = function(s)
	//print s
end function

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
Converter.hOnly = false   // when true, output only to hLines
Converter.cppOnly = false // when true, output only to cppLines
Converter.inVmCase = 0    // when > 1, codeBlockDepth of the current VM_CASE block
Converter.classFields = {}  // identifier:type
Converter.localVars = {}    // identifier:type

Converter.Make = function
	noob = new self
	noob.classNames = ["IOHelper", "String", "Math", "List<string>"]
	return noob
end function

Converter.match = function(inp, hOut, cppOut)
	self.m = self.line.match(inp)
	if not self.m then return false
	if not self.m.hasIndex("className") then self.m.className = self.className
	if self.m.hasIndex("params") then
		self.m.paramNamesOnly = self.removeDefaultValues(self.m.params)
	end if
	debugPrint "[" + self.context + "] --> " + inp
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

Converter.getType = function(identifier)
	if self.localVars.hasIndex(identifier) then return self.localVars[identifier]
	return self.classFields.get(identifier, null)
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
	self.comment = comment // (in case helper methods need it)
	
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
		if self.context == Context.METHOD then
			self.cppLines.push indentation + line
			if line and line[-1] == "{" then self.codeBlockDepth += 1
		else
			self.hLines.push indentation + line
		end if
		return
	end if
	
	// Check for other special control comments
	if comment == "//*** BEGIN CS_ONLY ***" then
		self.csOnly = true
	else if comment == "//*** END CS_ONLY ***" then
		self.csOnly = false
		return
	else if line == "/*** BEGIN H_ONLY ***" then
		debugPrint "H ONLY MODE!"
		self.hOnly = true
		return
	else if line == "*** END H_ONLY ***/" then
		debugPrint "H ONLY off"
		self.hOnly = false
		return
	else if line == "/*** BEGIN CPP_ONLY ***" then
		debugPrint "CPP ONLY MODE!"
		self.cppOnly = true
		return
	else if line == "*** END CPP_ONLY ***/" then
		debugPrint "CPP ONLY off"
		self.cppOnly = false
		return
	end if
	if self.csOnly then return
	if self.hOnly then
		self.hLines.push indentation + line + comment
		return
	else if self.cppOnly then
		self.cppLines.push indentation + line + comment
		return
	end if

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
	
	debugPrint line
	
	// And split off an opening curly too
	if line[-1] == "{" then
		self.addCurly = true
		line = line[:-1]
		line = line.trimRight
		if self.context == Context.METHOD then self.codeBlockDepth += 1
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
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
	self.line = self.tweakSyntax(line, self.context)

	if self.context == Context.METHOD then
		self.processMethodLine
	else
		self.processNonmethodLine
	end if
	
	// Closing curly: close the open code block or scope
	if self.line and self.line[0] == "}" then
		debugPrint "CLOSING CURLY!!"		
		fullLine = self.indentation + "}" + self.comment

		if self.context == Context.METHOD and self.codeBlockDepth > 0 then
			self.codeBlockDepth -= 1
			if self.codeBlockDepth < self.inVmCase then self.inVmCase = 0
		else if self.context == Context.METHOD then
			self.context = Context.CLASS
			self.localVars = {}
		else if self.context == Context.CLASS then
			// add ; after class declaration in .h file; omit from .cpp file
			self.hLines.push self.indentation + "}; // end of class " + self.className
			self.context = Context.NAMESPACE
			self.classFields = {}
		else
			self.hLines.push fullLine
			self.cppLines.push fullLine
			self.context = Context.START
		end if		
		debugPrint "Context down: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
	end if

end function

// Process a line of code *not* found within a method.
Converter.processNonmethodLine = function
	
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
	if self.match("namespace ≤name≥", "namespace ≤name≥", "namespace ≤name≥") then
		self.context = Context.NAMESPACE
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	// class declaration, with base class
	if self.match("≤qualifiers≥ class ≤name:w≥ : ≤base≥", "class ≤name≥ : public ≤base≥", "") or
	  self.match("class ≤name≥ : ≤base≥", "class ≤name≥: public ≤base≥", "") then
		self.context = Context.CLASS
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		self.className = self.m.name
		if not self.classNames.contains(self.className) then
			self.classNames.push self.className
		end if
		debugPrint "Got class name: " + self.className + "; all classes: " + self.classNames
		return
	end if

	// class/struct declaration
	if self.match("≤qualifiers≥ class ≤name:w≥", "class ≤name≥", "") or
	  self.match("class ≤name≥", "class ≤name≥", "") then
		self.context = Context.CLASS
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		self.className = self.m.name
		if not self.classNames.contains(self.className) then
			self.classNames.push self.className
		end if
		debugPrint "Got class name: " + self.className + "; all classes: " + self.classNames
		return
	end if

	if self.match("≤qualifiers≥ struct ≤name:w≥", "struct ≤name≥", "") or
	  self.match("struct ≤name≥", "struct ≤name≥", "") then
		self.context = Context.CLASS
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		self.className = self.m.name
		if not self.classNames.contains(self.className) then
			self.classNames.push self.className
		end if
		debugPrint "Got class name: " + self.className + "; all classes: " + self.classNames
		return
	end if
	
	// enum declaration (becomes an enum class in C++)
	if self.match("≤scope≥ enum ≤name≥ : ≤type≥", "enum class ≤name≥ : ≤type≥", "") then
		if not self.checkType then return
		if self.context != Context.CLASS then
			self.context = Context.CLASS
			debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
			self.className = self.m.name
			debugPrint "Got class (enum) name: " + self.className + "; all classes: " + self.classNames
		end if
		if not self.classNames.contains(self.className) then
			self.classNames.push self.className
		end if
		return
	end if
	
	// enum value
	if self.match("≤name:w≥ = ≤value≥,", "≤name≥ = ≤value≥,", "") or
	   self.match("≤name:w≥ = ≤value≥",  "≤name≥ = ≤value≥", "") or
	   self.match("≤name:w≥,", "≤name≥,", "") or
	   self.match("≤name:w≥",  "≤name≥", "") then
		return
	end if
	
	// class field, with and without default value
	// (note: default values, i.e. i-class member initializers, are valid since C++11)
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥;",
	  "≤scope≥: ≤type≥ ≤name≥;", "") then return self.noteClassField
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥ = ≤value≥;",
	  "≤scope≥: ≤type≥ ≤name≥ = ≤value≥;", "") then return self.noteClassField
	// Same but for pointer types (automatically initializing to nullptr if not specified)
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥* ≤name:w≥;",
	  "≤scope≥: ≤type≥* ≤name≥ = nullptr;", "") then return self.noteClassField
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥* ≤name:w≥ = ≤value≥;",
	  "≤scope≥: ≤type≥* ≤name≥ = ≤value≥;", "") then return self.noteClassField

	// class field (pointer/array)
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥[] ≤name:w≥;",
	  "≤scope≥: std::vector<≤type≥> ≤name≥;", "") then return self.noteClassField(self.m.name, self.m.type, true)

	// static class field, with and without default value
	if self.context == Context.CLASS and self.match("≤scope:w≥ static ≤type:w≥ ≤name:w≥;",
	  "≤scope≥: static ≤type≥ ≤name≥;", "") then return self.noteClassField
	if self.context == Context.CLASS and self.match("≤scope:w≥ static ≤type:w≥ ≤name:w≥ = ≤value≥;",
	  "≤scope≥: static ≤type≥ ≤name≥;", "≤type≥ ≤className≥::≤name≥ = ≤value≥;") then return self.noteClassField

	// (static) const class field with default value
	if self.context == Context.CLASS and 
	  self.match("≤scope:w≥ const ≤type:w≥ ≤name:w≥ = ≤value≥;",
	  "≤scope≥: static const ≤type≥ ≤name≥;",
	  "const ≤type≥ ≤className≥::≤name≥ = ≤value≥;") then return self.noteClassField

	// static readonly class field with default (usually non-constant) value
	if self.context == Context.CLASS and 
	  self.match("≤scope:w≥ static readonly ≤type:w≥ ≤name:w≥ = ≤value≥;",
	  "≤scope≥: static const ≤type≥ ≤name≥;",
	  "const ≤type≥ ≤className≥::≤name≥ = ≤value≥;") then return self.noteClassField

	// class property with expression body
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥ => ≤expr≥;",
	  "≤scope≥: ≤type≥ ≤name≥() { return ≤expr≥; }", "") then
		self.noteClassField
		// ToDo: note ≤name≥ in this class as something that will need () where used.
		return
	end if
	
	// public-get, private-set properties: we turn these into ordinary fields,
	// which loses the set protection, but it's fine
	if self.context == Context.CLASS and self.match("≤scope:w≥ ≤type:w≥ ≤name:w≥ { get; private set; }",
	  "≤scope≥: ≤type≥ ≤name≥;", "") then
		self.noteClassField
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
	  "≤className≥::≤name≥(≤paramNamesOnly≥)") then
		self.context = Context.METHOD
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
	// conversion operator declaration
	if self.match("≤scope≥ implicit operator ≤name≥()",
	 "≤scope≥: operator ≤name≥();", "≤className≥::operator ≤name≥()") then
		self.context = Context.METHOD
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	// delegate (function pointer)
	if self.match("public delegate ≤type:w≥ ≤name:w≥(≤params≥);",
		"using ≤name≥ = std::function<≤type≥(≤params≥)>;", "") then
		return
	end if
	
	// method declaration
	if self.match("≤scope:w≥ static ≤rtype:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: static ≤rtype≥ ≤name≥(≤params≥);",
	  "≤rtype≥ ≤className≥::≤name≥(≤paramNamesOnly≥)") then
		self.context = Context.METHOD
		self.noteParams
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if

	if self.match("≤scope:w≥ ≤rtype:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: ≤rtype≥ ≤name≥(≤params≥);",
	  "≤rtype≥ ≤className≥::≤name≥(≤paramNamesOnly≥)") then
	  	if self.m.scope == "static" then return self.fail("scope qualifier required on static method")
		self.context = Context.METHOD
		self.noteParams
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
	// virtual method declaration
	if self.match("≤scope:w≥ virtual ≤rtype:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: ≤rtype≥ ≤name≥(≤params≥);",
	  "≤rtype≥ ≤className≥::≤name≥(≤paramNamesOnly≥)") then
		self.context = Context.METHOD
		self.noteParams
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
	// abstract method declaration
	if self.match("≤scope:w≥ abstract ≤rtype:w≥ ≤name:w≥(≤params≥);",
	  "≤scope≥: virtual ≤rtype≥ ≤name≥(≤params≥) = 0;", "") then
		return
	end if
		
	// override method declaration
	if self.match("≤scope:w≥ override ≤rtype:w≥ ≤name:w≥(≤params≥)",
	  "≤scope≥: ≤rtype≥ ≤name≥(≤params≥);",
	  "≤rtype≥ ≤className≥::≤name≥(≤paramNamesOnly≥)") then
	  	if self.m.scope == "static" then return self.fail("scope qualifier required on static method")
		self.context = Context.METHOD
		self.noteParams
		debugPrint "Context: " + self.context + " (code block depth " + self.codeBlockDepth + ")"
		return
	end if
	
end function

// Process a line of code within a method.
Converter.processMethodLine = function
	// foreach statement
	if self.match("foreach (≤type:w≥ ≤var:w≥ in ≤seq≥)", "",
	  "for (int __≤var≥_idx=0, __≤var≥_qty=≤seq≥.Count(); __≤var≥_idx < __≤var≥_qty; __≤var≥_idx++)") then
		self.cppLines.push self.indentation + TAB + 
		  "≤type≥ ≤var≥ = ≤seq≥[__≤var≥_idx];".fill(self.m)
		return self.checkType
	end if
	
	// (capturing) lambda expression (NOTE: the C# code should include explicit param types)
	if self.comment.contains("[this]") then lambda = "[this]" else lambda = "[]"
	if self.match("≤name≥ = (≤params≥) => ≤expr≥;", "",
	  "≤name≥ = " + lambda + "(≤params≥) { return ≤expr≥; };") then
		return
	else if self.match("≤name≥ = (≤params≥) =>", "",
	  "≤name≥ = " + lambda + "(≤params≥) ") then
		return
	end if
	
	if self.className == "VM" then
		// Special case: VM dispatch case
		if self.match("case Opcode::≤opcode≥:") or self.match("case Opcode.≤opcode≥:") then
			self.cppLines.push self.indentation + "VM_CASE(≤opcode≥)".fill(self.m) + " {" * self.addCurly;
			self.inVmCase = self.codeBlockDepth
			return
		end if
		
		if self.line == "break;" and self.inVmCase then
			self.cppLines.push self.indentation + "VM_NEXT();"
			return
		end if
	end if
	
	// array assignments
	if self.match("≤name:w≥ = new ≤type:w≥[≤qty≥];", "",
	  "≤name≥ = std::vector<≤type≥>(≤qty≥);") then
		return
	end if
	
	// local variable declarations (with/without assignment)
	if self.match("≤type:w≥ ≤name:w≥;", "", self.line) or
	   self.match("≤type:w≥ ≤name:w≥ = ≤expr≥;", "", self.line) then
		self.noteLocalVar
		return
	end if
		
	// Anything else: copy to CPP file
	debugPrint "NO MATCH, copying: " + self.line
	curly = " {" * self.addCurly
	fullLine = self.indentation + self.line + curly + self.comment
	self.cppLines.push fullLine

end function

// Converter.tweakSyntax: do simple syntax transformations that can be done
// within pretty much any line of code.
Converter.tweakSyntax = function(line, context)
	// static class methods/properties, enum values (invoked on the class name itself)
	for cls in self.classNames
		afterPos = -1
		while true
			pos = line.indexOf(cls + ".", afterPos)
			if pos == null then break
			if pos == 0 or not isWordChar(line[pos-1]) then
				line = line[:pos + cls.len] + "::" + line[pos + cls.len + 1:]
				//print "REPLACED TO: " + line
			end if
			afterPos = pos + cls.len + 1
		end while
	end for
	
	// initialized char array declaration
	if self.match("char[] ≤name≥ = new char[≤len≥];") then
		self.m.lenPlus1 = str(self.m.len.val + 1)
		return "char ≤name≥[≤lenPlus1≥]; ≤name≥[≤len≥] = 0;".fill(self.m)
	end if
	
	// other local array declaration
//	m = line.match("≤type:w≥[] ≤name≥ = new ≤type2:w≥[≤count≥];")
//	if m then
//		if not type.endsWith("Ptr") then return "≤type≥ ≤name≥[≤count≥];".fill(m)
//		return line
//	end if
//	// ...other array creation, must always involve a pointer
//	m = line.match("≤name≥ = new ≤type:w≥[≤count≥];")
//	if m then return line

	// array syntax (especially as seen in parameters)
// TODO: commenting this out, as we are probably adopting std::vector everywhere,
// and that will certainly require rethinking this.
//	line = line.replace("[] ", " *")
	
	// special case of a Main program args: add the count
	line = line.replace("string *args", "String args[], int argCount")
	
	// Split to list
	m = line.match("≤pre≥.Split(≤args≥).ToList()≤post≥")
	if m then line = "≤pre≥.SplitToList(≤args≥)≤post≥".fill(m)
	
	if line.contains(" new ") then
		// Object creation & assignment: look up the LHS type to know whether to 
		// keep "new" (because it's a Ptr) or remove it (any other type).
		debugPrint "FOUND new IN: " + line
		m = line.match("≤type≥ ≤name≥ = new ≤expr≥;")
		if not m then m = line.match("≤name≥[≤idx≥] = new ≤expr≥;")
		if not m then m = line.match("≤name≥ = new ≤expr≥[≤qty≥];")
		if m then
			if not m.hasIndex("type") then m.type = self.getType(m.name)
			if m.type == null or (not m.type.endsWith("Ptr") and not m.type.endsWith("[]")) then
				line = line.replace(" new ", " ")
			end if
			print "SAW NEW and found type: " + m.type
		else
			print "SAW NEW, but no match"
			// If not one of the above patterns, we'll assume not a pointer (for now)
			line = line.replace(" new ", " ")
		end if
		debugPrint "...and made it: " + line
	end if
	// Other object creation (assuming a compatible memory-managed type).
	line = line.replace("(new ", "(")
	if line.startsWith("new ") then line = line[4:]
	
	// null --> nullptr
	line = line.replace(" null", " nullptr")
	line = line.replace("(null", "(nullptr")
	
	// Count and Length -- properties in C#, but methods in C++.
	// We may need to handle this more generally.  But for now:
	line = line.replace(".Count", ".Count()")
	line = line.replace(".Length", ".Length()")
		
	// Integer min/max value: just remove the dot, creating constants
	// defined in core_includes.h.
	line = line.replace(".MinValue", "MinValue")
	line = line.replace(".MaxValue", "MaxValue")
	
	// Output parameters on method calls
	line = line.replace("(out ", "(&")
	line = line.replace(", out ", ", &")

	// Pointer dereference (changing `.` to `->`)
	line = line.replace("this.", "this->")
	for kv in self.classFields
		if not kv.value.endsWith("Ptr") then continue
		line = line.replace(kv.key + ".", kv.key + "->")
	end for	
	for kv in self.localVars
		if not kv.value.endsWith("Ptr") then continue
		line = line.replace(kv.key + ".", kv.key + "->")
	end for	
	
	// Reference types, using an alias like FuncDefRef to mean FuncDef&
	if line.contains("Ref ") then
		words = line.split
		for i in words.indexes
			if words[i].endsWith("Ref") then words[i] = words[i][:-3] + "&"
		end for
		line = words.join
	end if
	
	// Reference parameters or arguments
	if context == Context.CLASS then
		// ref Type foo  -->  Type& foo
		lastPos = -1
		while true
			pos = line.indexOf("(ref ", lastPos)
			if pos == null then pos = line.indexOf(" ref ", lastPos);
			if pos == null then break
			//if lastPos < 0 then print "BEFORE: " + line
			spacePos = line.indexOf(" ", pos + 5)
			if spacePos == null then break
			line = line[:pos+1] + line[pos+5:spacePos] + "& " + line[spacePos+1:]
			//print "AFTER: " + line
			lastPos = pos
		end while
	else if context == Context.METHOD then
		// Within a method, just delete " ref "
		line = line.replace("(ref ", "(")
		line = line.replace(" ref ", " ")
	end if

	return line
end function

Converter.removeDefaultValues = function(paramStr)
	params = paramStr.split(",")
	for i in params.indexes
		eqPos = params[i].indexOf("=")
		if eqPos != null then params[i] = params[i][:eqPos]
	end for
	return params.join(",")
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

disallowedTypes = {
	"sbyte": "SByte",
	"byte": "Byte",
	"short": "Int16",
	"ushort": "UInt16",
	"int": "Int32",
	"uint": "UInt32",
	"long": "Int64",
	"ulong": "UInt64" }
// Converter.checkType: ensure that the given type is something we can deal
// with, and throw and error if it is disallowed.  Return true if OK, 
// false if failed.
Converter.checkType = function(type)
	if type == null then type = self.m["type"]
	if not disallowedTypes.hasIndex(type) then return true
	self.fail "Type " + type + " disallowed; use " + disallowedTypes[type] + " instead"
	return false
end function

Converter.noteClassField = function(name, type, array=false)
	if name == null then name = self.m.name
	if type == null then type = self.m.type
	self.classFields[name] = type + "[]" * array
	debugPrint "--> classFields: " + self.classFields
	return self.checkType(type)
end function

Converter.noteLocalVar = function(name, type, array=false)
	if name == null then name = self.m.name
	if type == null then type = self.m.type
	self.localVars[name] = type + "[]" * array
	debugPrint "--> localVars: " + self.localVars
	return self.checkType(type)
end function

Converter.noteParams = function(params)
	if params == null then params = self.m.params
	for param in params.split(",")
		param = param.trim
		typeAndName = param.split
		if typeAndName.len != 2 then continue
		self.localVars[typeAndName[1]] = typeAndName[0]
	end for
	debugPrint "--> localVars: " + self.localVars
end function

WARNING = [
	"// AUTO-GENERATED FILE.  DO NOT MODIFY.",
	"// Transpiled from: ",
	"",
]

Converter.convertClass = function(sourcePath)
	fileNameBase = file.name(sourcePath) - ".cs"
	
	guardSymbol = ("__" + file.name(sourcePath) - ".cs" + "_H").upper
	warning = WARNING[:]
	warning[1] += file.name(sourcePath)
	self.hLines = warning + ["#ifndef " + guardSymbol, "#define " + guardSymbol, ""]
	self.cppLines = warning
	self.didIncludes = false
	self.filename = file.name(sourcePath)
	self.lineNum = 0
	self.failed = false
	self.csOnly = false
	self.hOnly = false
	self.cppOnly = false

	for line in file.readLines(sourcePath)
		self.lineNum += 1
		//debugPrint line
		self.processLine line
		if self.failed then return
	end for

	if self.hLines.len <= 6 and self.cppLines.len <= 3 then
		print fileNameBase + ".cs: skipped (C# only)"
		return
	end if
	totalLines = self.hLines.len + self.cppLines.len
	print "≤fileNameBase≥.cs --> ≤fileNameBase≥.g.h/.cpp (≤totalLines≥ lines)".fill(locals)

	self.hLines.push ""
	self.hLines.push "#endif // " + guardSymbol

	outFolder = "generated/"
	debugPrint "-----------------------------------"
	debugPrint "HEADER:"
	debugPrint self.hLines.join(char(10))
	file.writeLines outFolder + fileNameBase + ".g.h", self.hLines
	debugPrint
	debugPrint "-----------------------------------"
	debugPrint "CPP:"
	debugPrint self.cppLines.join(char(10))
	file.writeLines outFolder + fileNameBase + ".g.cpp", self.cppLines
	debugPrint
end function

Converter.findClassNames = function(sourcePath)
	for line in file.readLines(sourcePath)
		m = line.match("≤scope≥ class ≤class:w≥ {≤≥")
		if not m then m = line.match("≤scope≥ enum ≤class:w≥ : ≤type:w≥ {≤≥")
		if m then
			print "Found class: " + m.class + " in: " + sourcePath
			self.classNames.push m.class
		end if
	end for
end function

if locals == globals then
	cvt = Converter.Make

	for path in shellArgs[1:]
		cvt.findClassNames path
	end for

	for path in shellArgs[1:]
		cvt.convertClass path
		if cvt.failed then exit -1
	end for
end if

