: task0 3 0 DO 0 . CR LOOP ;
: task1 CR 3 0 DO 1 . LOOP ;
DICT
TRUE STEP
' task0 SPAWN
' task1 SPAWN