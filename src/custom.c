/*
 * custom.c
 *
 *  Created on: Oct 4, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "builtins.h"
#include "task.h"
static void my_custom(ftask_p task) {
  printf("\nThis is my custom word. There are two numbers on the stack now\n");
  d_push(task, 1);
  d_push(task, 2);
}

void add_custom_builtins() 
{ 
	builtin_add("my_custom", my_custom); 
}
