VARIABLE keyready

: keytask
    30 0 DO
    	KEY keyready SWAP !
    	" took-> " s. keyready @ EMIT CR
    LOOP
    " Keytask finished!" s.
    ;
    
: task1 1000 0 DO
	KEY 48 = IF " SPACE" s. ELSE keyready @ EMIT THEN
	LOOP
	;
	


