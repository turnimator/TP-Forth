
" speedQ" 26 QCREATE
" dirQ" 26 QCREATE

: checkSpeed DUP S. S>L DUP DUP 
	100 < IF "  : Speed up!" S. CR THEN
	120 > IF "  : Slow down!" S. CR THEN
	 ;


: checkDirection DUP S. S>L DUP DUP 
	-1 < IF " Turn right!" S. CR THEN
	1 >  IF " Turn left!" S. CR 	 THEN
	 ;

	 
: readdirection
	100 0 DO 
		" dirQ" Q DUP <Q checkDirection 
		LOOP 
	;
	 
: readspeed
	100 0 DO 
		" speedQ" Q DUP <Q checkSpeed 
		LOOP 
	;
	
' readspeed SPAWN
' readdirection SPAWN

: speed " speedQ" Q SWAP L>S Q> ;
: dir " dirQ" Q SWAP L>S Q> ;

VARIABLE speedq
speedq " speedQ" Q !
speedq @ 210 L>S Q>

: drive 220 90 DO
	1000000 MS
	I speed
	LOOP
;