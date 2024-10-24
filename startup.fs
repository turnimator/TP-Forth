VARIABLE keyready

: keytask
    300 0 DO
    	KEY keyready SWAP !
    	" stored:" s. keyready @ EMIT 
    LOOP
    ;
    
