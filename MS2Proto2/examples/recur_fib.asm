@fib:
	LOADK r1, 2			# r1 = 2
	IFLT r0, r1, done	# if r0 < r1 then return
	
	LOADK r1, 1			# r1 = 1
	SUB r1, r0, r1		# r1 = n - r1
	CALLF r1, 1, 0		# r1 = fib(r1)
	
	LOADK r2, 2			# r2 = 2
	SUB r2, r0, r2		# r2 = n - r2
	CALLF r2, 1, 0		# r2 = fib(r2)
	
	ADD r0, r1, r2		# r0 = r1 + r2

done:
	RETURN				# return r0
	

@main:
	LOADK r0, 10		# r0 = 10
	CALLF r0, 1, 0		# r0 = fib(r0)
	RETURN				# return r0
