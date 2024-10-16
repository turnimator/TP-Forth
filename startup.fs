VARIABLE NUM
: isPrime
	NUM SWAP ! 
	NUM @ 1 - 2 DO 
	NUM @ I MOD 0 = IF 0 EXIT THEN
	LOOP
	1
	;

: tst 20 2 DO I isPrime IF I . THEN LOOP ;
	 