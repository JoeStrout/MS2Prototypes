// Recursive Fibonacci sequence.


rfib = function(n)
    if n < 1 then return 0
    if n == 1 then return 1
    return rfib(n-1) + rfib(n-2)
end function

runBenchmark = function(n=30)
	t0 = time
	x = rfib(n)
	t1 = time
	print "rfib(" + n + ") time: " + round(t1 - t0, 3)
	results["rfib(" + n + ")"] = round(t1 - t0, 3)
end function

if locals == globals then
	results = {}
	runBenchmark
end if

