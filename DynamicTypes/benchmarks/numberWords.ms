singles = " one two three four five six seven eight nine ".split
teens = "ten eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen ".split
tys = "  twenty thirty forty fifty sixty seventy eighty ninety".split
ions = "thousand million billion".split

numberToText = function(n)
    if n == 0 then return "zero"
    a = abs(n)
    r = ""  // (result)
    for u in ions
        h = a % 100
        if h > 0 and h < 10 then r = singles[h] + " " + r
        if h > 9 and h < 20 then r = teens[h-10] + " " + r
        if h > 19 and h < 100 then r = tys[h/10] + "-"*(h%10>0) + singles[h%10] + " " + r
        h = floor((a % 1000) / 100)
        if h then r = singles[h] + " hundred " + r
        a = floor(a / 1000)
        if a == 0 then break
        if a % 1000 > 0 then r = u + " " + r
    end for
    if n < 0 then r = "negative " + r
    return r
end function

textToNumber = function(s)
	if s == "zero" then return 0
	words = s.replace("-", " ").split
	result = 0
	ionVal = 0
	negative = false
	maxi = words.len - 1
	i = 0
	while i <= maxi
		word = words[i]
		if word == "negative" then
			negative = true
			i += 1
			continue
		end if
		idx = ions.indexOf(word)
		if idx != null then
			result += ionVal * [1000, 1E6, 1E9][idx]
			ionVal = 0
			i += 1
			continue
		end if
		idx = singles.indexOf(word)
		if idx != null then
			wordVal = idx
		else
			idx = tys.indexOf(word)
			if idx != null then
				wordVal = idx * 10
			else
				idx = teens.indexOf(word)
				if idx != null then
					wordVal = idx + 10
				else
					print "Unexpected word: " + word
					return 0
				end if
			end if
		end if
		if i < maxi and  words[i+1] == "hundred" then
			wordVal *= 100
			i += 1
		end if
		ionVal += wordVal
		i += 1
	end while
	result += ionVal
	if negative then result = -result
	return result
end function

runBenchmark = function(n=10000)
	t0 = time
	i = 0
	while i < n
		s = numberToText(i)
		i2 = textToNumber(s)
		if i2 != i then
			print "Oops!  Failed on " + i + ":"
			print s + " --> " + i2
		end if
		i += 1
	end while
	t1 = time
	print "numberWords(" + n + ") time: " + round(t1 - t0, 3)
	results["numberWords(" + n + ")"] = round(t1 - t0, 3)
end function

if locals == globals then
	print "Correctness checks:"
	for n in [-1234, 0, 7, 42, 4325, 1000004, 214837564]
		words = numberToText(n)
		backToNum = textToNumber(words)
		print n + ": " + words + " -> " + backToNum
		if backToNum != n then
			print "ERROR --^"
			exit
		end if
	end for
	results = {}
	runBenchmark
end if
