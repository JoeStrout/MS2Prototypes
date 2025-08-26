-- Recursive Fibonacci sequence.

function rfib(n)
    if n < 1 then return 0 end
    if n == 1 then return 1 end
    return rfib(n-1) + rfib(n-2)
end

function runBenchmark(n)
    n = n or 40
    local t0 = os.clock()
    local x = rfib(n)
    local t1 = os.clock()
    local elapsed = t1 - t0
    print(string.format("rfib(%d) = %d, time: %.3f seconds", n, x, elapsed))
    results["rfib(" .. n .. ")"] = math.floor(elapsed * 1000 + 0.5) / 1000  -- round to 3 decimal places
end

-- Main execution (equivalent to "if locals == globals")
if not package.loaded[...] then
    results = {}
    runBenchmark()
end