
" speedQ" 26 QCREATE	\ Create a queue 26 bytes long
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
		" dirQ" Q DUP <Q checkDirection  \ Simulate reading a direction sensor like a compass
		LOOP 
	;
	 
: readspeed
	100 0 DO 
		" speedQ" Q DUP <Q checkSpeed 	\ Simulate reading sensors from wheels or pwm values
		LOOP 
	;
	
' readspeed SPAWN		\ Spawn the two "sensor reading" background tasks 
' readdirection SPAWN

: speed " speedQ" Q SWAP L>S Q> ; \ Simulate a sensor, write the sensor value to the queue
: dir " dirQ" Q SWAP L>S Q> ;

VARIABLE speedq
speedq " speedQ" Q !  \ Storing the speed queue in a variable
speedq @ 210 L>S Q>		\ simulate a speed reading of 210

: drive 130 90 DO \ Simulate acceleration from 90 to 130
	1000000 MS
	I speed
	LOOP
	
;