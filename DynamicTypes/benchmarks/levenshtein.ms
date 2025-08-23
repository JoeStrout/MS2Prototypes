// Calculates the Levenshtein (edit) distance between two strings.
// The core code is the same as the editDistance function added
// by the /sys/lib/stringUtil module.
string.editDistance = function(s2)
	n = self.len
	m = s2.len
	if n == 0 then return m
	if m == 0 then return n
 
	s1chars = self.split("")
	s2chars = s2.split("")
	d = range(0, m)	
	lastCost = 0
 
	for i in range(1, n)
		s1char = s1chars[i-1]
		lastCost = i
		jMinus1 = 0
		for j in range(1, m)
			if s1char == s2chars[jMinus1] then cost = 0 else cost = 1
 
			// set nextCost to the minimum of the following three possibilities:
			a = d[j] + 1
			b = lastCost + 1
			c = cost + d[jMinus1]
 
			if a < b then
				if c < a then nextCost = c else nextCost = a
			else
				if c < b then nextCost = c else nextCost = b
			end if
 
			d[jMinus1] = lastCost
			lastCost = nextCost
			jMinus1 = j
		end for
		d[m] = lastCost
	end for
 
	return nextCost
end function

ga1 = "Four score and seven years ago our fathers brought forth on this continent, a new nation, conceived in Liberty, and dedicated to the proposition that all men are created equal.  Now we are engaged in a great civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are met on a great battle-field of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is altogether fitting and proper that we should do this. " +
"But, in a larger sense, we can not dedicate—we can not consecrate—we can not hallow—this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or detract. The world will little note, nor long remember what we say here, but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. " +
"It is rather for us to be here dedicated to the great task remaining before us—that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion—that we here highly resolve that these dead shall not have died in vain—that this nation, under God, shall have a new birth of freedom—and that government of the people, by the people, for the people, shall not perish from the earth."

ga2 = "Eighty seven years ago our ancestors brought forth in these parts, a new nation, conceived in freedom, and dedicated to the proposition that all people are created equal.  Now we are engaged in a lousy civil war, testing whether that nation, or any nation so conceived and so dedicated, can long endure. We are gathered on a famous battlefield of that war. We have come to dedicate a portion of that field, as a final resting place for those who here gave their lives that that nation might live. It is super groovy and cool that we should do this. " +
"But, in a larger sense, we can not dedicate — we can not consecrate — we can not hallow — this ground. The brave men, living and dead, who struggled here, have consecrated it, far above our poor power to add or subtract. The world will little note, nor long remember what we say here (ha ha as if), but it can never forget what they did here. It is for us the living, rather, to be dedicated here to the unfinished work which they who fought here have thus far so nobly advanced. " +
"It is rather for us to be here dedicated to the great task remaining before us — that from these honored dead we take increased devotion to that cause for which they gave the last full measure of devotion — that we here highly resolve that these dead shall not have died in vain — that this nation, with its constitutionally guaranteed separation of church and state, shall have a new birth of freedom — and that government of the people, by the people, for the people, shall not disappear from Earth."

runTest = function
	x = "kitten".editDistance("sitting")
	s = "this is a test of a slightly longer string"
	x = s.editDistance("that was a test of a slightly longer string")
	x = ga1.editDistance(ga2)
	x = ga1.editDistance("banana")
end function

runBenchmark = function
	t0 = time
	runTest
	t1 = time
	print "levenshtein: " + round(t1 - t0, 3)
	results.levenshtein = round(t1 - t0, 3)
end function

if locals == globals then
	results = {}
	runBenchmark
end if

