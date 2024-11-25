VARIABLE myq
myq " 192.168.7.100/5000" 128 NQCREATE !
: rd 100 0 DO myq @ <NQ " Received:" S. S. CR LOOP ;
