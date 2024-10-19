VARIABLE NUM

: isPrime 
	NUM SWAP !
	NUM @ 2 DO 
		NUM @ I MOD 0 = IF 0 0 . EXIT THEN
	LOOP
	1 .
	1
	;
	
