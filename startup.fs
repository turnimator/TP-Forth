
" speedQ" 26 QCREATE
" dirQ" 26 QCREATE

: checkSpeed DUP S. S>L DUP DUP 
	100 < IF "  : Speed up!" S. CR THEN
	200 > IF "  : Slow down!" S. CR THEN
	 ;


: checkDirection DUP S. S>L DUP DUP 
	-1 < IF " Turn right!" S. CR THEN
	1 >  IF " Turn left!" S. CR 	 THEN
	 ;

	 
: readdirection
	10 0 DO 
		" Direction:" S. " dirQ" Q DUP <Q checkDirection 
		LOOP 
	;
	 
: readspeed
	10 0 DO 
		" Speed:" S. " speedQ" Q DUP <Q checkSpeed 
		LOOP 
	;
	
' readspeed SPAWN
' readdirection SPAWN

: speed " speedQ" Q SWAP L>S Q> ;
: dir " dirQ" Q SWAP L>S Q> ;

VARIABLE speedq
speedq " speedQ" Q !
speedq @ 210 L>S Q>


