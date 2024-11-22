VARIABLE myq
myq " 127.0.0.1/5000" 128 NQCREATE !
: wr myq @ " Hello" NQ> CR ;
: rd 100 0 DO myq @ <NQ " Received:" S. S. CR LOOP ;
' rd SPAWN
