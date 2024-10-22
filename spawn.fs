: task0 30 0 DO 0 . LOOP CR " Task0 done!" s. CR ;
: task1 30 0 DO 1 . LOOP CR " Task1 done!" s. CR ;
: task2 30 0 DO 2 . LOOP CR " Task2 done!" s. CR ;
: task3 30 0 DO 3 . LOOP CR " Task3 done!" s. CR ;
CR " These are the tasks to run:" s. 
DICT
" First, we run them in sequence:" s. CR
' task0 EXEC
' task1 EXEC
' task2 EXEC
' task3 EXEC
CR " Then, we run them concurrently:" s. CR
' task0 SPAWN
' task1 SPAWN
' task2 SPAWN
' task3 SPAWN

10000 0 DO   LOOP
CR " Now we wait for them to finish" s.
