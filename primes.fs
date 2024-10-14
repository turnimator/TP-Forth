VARIABLE mynum
VARIABLE prime_flag
: checkprime ( n -- )
1 prime_flag ! \ Optimistic bet!
mynum @ 2 ?DO
mynum @ I MOD 0= IF
0 prime_flag ! EXIT
THEN
LOOP ;
: primes 2 ?DO
I mynum ! checkprime
prime_flag @ IF
CR I .
THEN
LOOP ;

