: task0 30 0 DO 0 . LOOP CR " Task0 done!" s. CR ;
: task1 30 0 DO 1 . LOOP CR " Task1 done!" s. CR ;
: task2 30 0 DO 2 . LOOP CR " Task2 done!" s. CR ;
: task3 30 0 DO 3 . LOOP CR " Task3 done!" s. CR ;
CR " These are the tasks to run:" s. 
DICT
" Press ENTER to run them in sequence:" s. CR
KEY?
' task0 EXEC
' task1 EXEC
' task2 EXEC
' task3 EXEC
CR " Press ENTER to run them concurrently:" s. CR
KEY?
' task0 SPAWN
' task1 SPAWN
' task2 SPAWN
' task3 SPAWN

10000 0 DO   LOOP
CR " Now we wait for them to finish" s.
