
VARIABLE isprime
VARIABLE current

isprime TRUE !

: testprime
	.s
	dup 
	current !
	current @ .
	.s
	0 DO
		current @ . CR I . 
	LOOP
	;
	  