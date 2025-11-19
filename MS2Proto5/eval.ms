//------------------------------------------------------------
// General utility functions

isNumber = function(s)
	return "0" <= s[0] <= "9"
end function

isIdentifier = function(s)
	c = s[0]
	return ("A" <= c <= "Z") or c >= "a" or c == "_"
end function

map.get = function(key, default=null)
	if self.hasIndex(key) then return self[key]
	return default
end function 


//------------------------------------------------------------
// A simple lexer, handling numbers, identifiers, and operators.
// Splits a string into a list of tokens, stripping out whitespace.

lex = function(exprStr)
	State = { "START": 0, "NUMBER": 1, "OPERATOR": 2, "IDENTIFIER": 3 }
	state = State.START
	result = []
	token = ""
	for c in exprStr
		if c <= " " then
			if token then result.push token
			state = State.START
			token = ""
		else if "0" <= c <= "9" or c == "." then
			if state == State.NUMBER then
				token += c
			else
				if token then result.push token
				state = State.NUMBER
				token = c
			end if
		else if "+-*/%^()=".indexOf(c) != null then
			if token then result.push token
			state = State.OPERATOR
			token = c
		else
			if state == State.IDENTIFIER then
				token += c
			else
				if token then result.push token
				state = State.IDENTIFIER
				token = c
			end if
		end if
	end for
	if token then result.push token
	return result
end function

//------------------------------------------------------------
// Storage for user-defined variables,
// initialized with some standard constants and functions

vars = {"pi":pi, "rnd":@rnd,
 "round":@round, "floor":@floor, "ceil":@ceil, 
 "log":@log, "sign":@sign, "sqrt":@sqrt,
 "sin":@sin, "cos":@cos, "tan":@tan, 
 "asin":@asin, "acos":@acos, "atan":@atan,
}

lookupVar = function(identifier)
	if vars.hasIndex(identifier) then return vars[identifier]
	print "Undefined identifier: " + identifier
	return null
end function

//------------------------------------------------------------
// Operations: what the various parselets actually _do_.
// As a group, these methods define whether your parser builds an AST,
// or emits bytecode, or immediately evaluates a value, or whatever.
Op = {}
Op.numVal     = function(s);   return val(s); end function
Op.getVar     = function(s)
	if vars.hasIndex(s) then return vars[s]
	print "Undefined identifier: " + s
	return null
end function
Op.call       = function(ident, arg)
	if vars.hasIndex(ident) then
		f = @vars[ident]
		return f(arg)
	end if
	print "Undefined identifier: " + s
	return null
end function
Op.setVar     = function(s,v); vars[s] = v; end function
Op.unaryMinus = function(x);  print "unaryMinus " + x; return -x;  end function
Op.subtract   = function(a,b); return a-b; end function
Op.add        = function(a,b); return a+b; end function
Op.multiply   = function(a,b); return a*b; end function
Op.divide     = function(a,b); return a/b; end function
Op.mod        = function(a,b); return a%b; end function
Op.power      = function(a,b); return a^b; end function
Op.factorial  = function(x)
	result = x
	while x > 1
		x -= 1
		result *= x
	end while
	return result
end function

//------------------------------------------------------------
// Precedence levels (higher precedence binds more strongly).

Precedence = {
    "BELOW_ASSIGNMENT" : 0,
    "ASSIGNMENT" : 1,
    "CONDITIONAL" : 2,
    "SUM" : 3,
    "PRODUCT" : 4,
    "EXPONENT" : 5,
    "FACTORIAL" : 6,
    "PREFIX" : 7,
    "POSTFIX" : 8,
    "CALL" : 9,
}

//------------------------------------------------------------
// Parselets -- mini-parsers, each responsible for parsing one
// certain type of thing.  Some of them are invoked by only 
// one token; others might be invoked by many tokens, and
// parameterized to control what they do.
Parselet = {}
Parselet.precedence = 0
Parselet.operation = null

PrefixParselet = new Parselet
PrefixParselet.parse = function(tokens)
	return self.operation(tokens.pull)
end function

UnaryOpParselet = new PrefixParselet
UnaryOpParselet.parse = function(tokens)
	tokens.pull  // skip operator
	operand = evalTokens(tokens, self.precedence)
	return self.operation(operand)
end function

InfixParselet = new Parselet
InfixParselet.rightAssoc = false
InfixParselet.parse = function(lhs, tokens)
	rhs = evalTokens(tokens, self.precedence - self.rightAssoc)
	return self.operation(lhs, rhs)
end function

PostfixParselet = new InfixParselet
PostfixParselet.parse = function(lhs, tokens)
	// This is a right-side unary operator, not actually a binary operator.
	// So we don't touch the given tokens, but instead just operate on lhs.
	return self.operation(lhs)
end function

//------------------------------------------------------------
// Methods in this Parse collection create Parselets, properly configured
// for each different kind of token.  In many cases, the configuration
// includes the operation (see the Op map above) to actually perform.

Parse = {}

Parse.Number = new PrefixParselet
Parse.Number.operation = @Op.numVal

Parse.Identifier = new PrefixParselet
Parse.Identifier.operation = @Op.getVar
Parse.Identifier.assignmentOp = @Op.setVar
Parse.Identifier.callOp = @Op.call
Parse.Identifier.parse = function(tokens)
	identifier = tokens.pull
	// When we've seen an identifier, there are several possibilities:
	// 1. It could be an assignment (next token is '=')
	// 2. It could be a call (next token is '(')
	// 3. Otherwise, it's a simple variable lookup.
	nextToken = null
	if tokens then nextToken = tokens[0]
	if nextToken == "=" then
		tokens.pull	 // discard "="
		rhs = evalTokens(tokens)
		return self.assignmentOp(identifier, rhs)
	else if nextToken == "(" then
		tokens.pull  // discard "("
		arg = null
		if tokens and tokens[0] != ")" then arg = evalTokens(tokens)
		if not tokens or tokens[0] != ")" then
			print "Unbalanced parentheses"
			return null
		end if
		tokens.pull  // discard ")"
		return self.callOp(identifier, arg)
	else
		return self.operation(identifier)
	end if
end function

Parse.Group = new PrefixParselet
Parse.Group.parse = function(tokens)
	tokens.pull  // discard "("
	result = evalTokens(tokens)
	if not tokens or tokens[0] != ")" then
		print "Unbalanced parentheses"
		return null
	end if
	tokens.pull  // discard ")"
	return result
end function

Parse.UnaryOp = function(operation, precedence)
	if precedence == null then precedence = precedence.PREFIX
	result = new UnaryOpParselet
	result.operation = @operation
	result.precedence = precedence
	return result
end function

Parse.Binop = function(operation, precedence, rightAssoc=false)
	result = new InfixParselet
	result.operation = @operation
	result.precedence = precedence
	if rightAssoc then result.rightAssoc = true
	return result
end function

Parse.PostUnary = function(operation, precedence)
	result = new PostfixParselet
	result.operation = @operation
	result.precedence = precedence
	return result
end function

//------------------------------------------------------------
// And here's where the grammar of our language is defined, as
// a set of token effects -- i.e., the effect of each token.

TokenEffect = {}

effect = function(prefixParser, infixParser)
	result = new TokenEffect
	result.prefixParser = prefixParser
	result.infixParser = @infixParser
	return result
end function

tokenEffects = {}
tokenEffects["("] = effect(Parse.Group, null)
tokenEffects[number] = effect(Parse.Number, null)
tokenEffects["ident"] = effect(Parse.Identifier, null)
tokenEffects["-"] = effect(Parse.UnaryOp(@Op.unaryMinus, Precedence.PREFIX), 
    Parse.Binop(@Op.subtract, Precedence.SUM))
tokenEffects["+"] = effect(null, Parse.Binop(@Op.add, Precedence.SUM))
tokenEffects["*"] = effect(null, Parse.Binop(@Op.multiply, Precedence.PRODUCT))
tokenEffects["/"] = effect(null, Parse.Binop(@Op.divide, Precedence.PRODUCT))
tokenEffects["%"] = effect(null, Parse.Binop(@Op.mod, Precedence.PRODUCT))
tokenEffects["^"] = effect(null, Parse.Binop(@Op.power, Precedence.EXPONENT, true))
tokenEffects["!"] = effect(null, Parse.PostUnary(@Op.factorial, Precedence.FACTORIAL))

//------------------------------------------------------------

// Helper method to get the precedence of the infix parser associated
// with the given token (if any).
infixPrecedence = function(token)
	effect = tokenEffects.get(token)
	if not effect or not effect.infixParser then return -1
	return effect.infixParser.precedence
end function

// Evaluate a set of tokens and return the result.
evalTokens = function(tokens, precedence=0)
	if not tokens then return null
	
	// first, check prefix parsers (for tokens that can start an expression)
	firstTok = tokens[0]
	effect = null
	if isNumber(firstTok) then
		effect = tokenEffects[number]
	else if isIdentifier(firstTok) then
		effect = tokenEffects.ident
	else
		effect = tokenEffects.get(firstTok)
	end if
	if effect and effect.prefixParser then
		value = effect.prefixParser.parse(tokens)
	else
		print "Invalid expression start: " + firstTok
		return null
	end if
	
	// then, continue applying infix parsers, until
	// we hit something of lower (or same?) precedence.
	while tokens and infixPrecedence(tokens[0]) > precedence
		operator = tokens.pull
		effect = tokenEffects.get(operator)
		if not effect or not effect.infixParser then break
		value = effect.infixParser.parse(value, tokens)
	end while
	return value
end function

// Evaluate a string and return the result.
eval = function(s)
	return evalTokens(lex(s))
end function

runUnitTests = function
	print "Unit testing: eval"
	
	errorCount = 0
	check = function(expression, expected)
		actual = eval(expression)
		if actual != expected then
			print "Unit test failure on """ + expression + """: expected " + expected + ", got " + actual
			outer.errorCount = errorCount + 1
		end if
	end function
	
	check "2+2", 4
	check "pi", pi
	check "2+3*4", 14
	check "(2+3)*4", 20
	check "5!", 120
	check "200 - 5!", 80
	check "2^2^3", 256
	check "(2^2)^3", 64
	check "round(cos(45*pi/180)*100)", 71
	
	if errorCount == 0 then
		print "All tests passed."
	else
		print errorCount + " error" + "s" * (errorCount!=1) + " found."
	end if
end function

if locals == globals then
	runUnitTests
	print "Enter expression to evaluate, or `quit` to quit."
	while true
		inp = input("eval> ")
		if inp == "quit" or inp == "exit" then break
		result = eval(inp)
		if result != null then print result
	end while
end if
