
VARIABLE mynum
VARIABLE prime_flag

: checkprime 
	1 prime_flag !
	mynum @ 2 DO mynum @ I mod 0 = IF 0 prime_flag ! THEN  
	LOOP
 ;
 
 : primes 2 DO
   I mynum !
   checkprime
   prime_flag @ 1 = IF I . CR THEN
   LOOP
   ;
 
 10000 primes
 