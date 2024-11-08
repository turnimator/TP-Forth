
" q1" 26 QCREATE
.s
: sender 5 0 DO " q1" Q " Hello" Q> 10000000 MS LOOP ;
: receiver 10 0 DO " Received:" S. " q1" Q DUP Q. <Q S. CR LOOP ;
' receiver SPAWN
