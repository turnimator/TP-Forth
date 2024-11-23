VARIABLE myq
myq " 192.168.7.102/5000" 128 NQCREATE !
: wr myq @ " Hello" NQ> CR ;
: rd 100 0 DO myq @ <NQ " Received:" S. S. CR LOOP ;

