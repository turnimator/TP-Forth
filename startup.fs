2 VARIABLE test
test .
VARIABLE v

v 10 ALLOT

v 5 6 !!

: p 10 0 DO v I DUP !! LOOP ;
v 5 6 !!

: q 10 0 DO v I @@ . LOOP ;