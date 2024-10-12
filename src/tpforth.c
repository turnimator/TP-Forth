/*
 ============================================================================
 Name        : fnf.c
 Author      : Jan Atle Ramsli
 Version     :
 Copyright   : (C) JAR
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

//#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "parser.h"
#include "program.h"
#include "runtime.h"
#include "task.h"

char* read_file(char* filename)
{
	char * rv;
	struct stat stbuf;
	FILE* f = fopen(filename, "r");
	if ( ! f){
		perror("startup.fs");
		return 0;
	}
	
	fstat(fileno(f), &stbuf);
	rv = malloc(stbuf.st_blksize*stbuf.st_blocks);
	fread(rv, stbuf.st_blksize, stbuf.st_blocks, f);
	fclose(f);
	return rv;
}


int main(int ac, char*av[]) {
	puts("TP-FORTH - A Forth Language P-CODE Interpreter)\n");
	
	char buf[256];
	dict_init();
	builtin_build_db();
	
	ftask_p t = ftask_create("Main");
	
	char *filename = "startup.fs";
	if (ac > 1){
		filename = av[1];
	}
	char* src = read_file(filename);
	if (src){
		program_p prog = parse(t, src);
		t->program = prog;
		run_task(t);
		//free(src);
	}
	
	t = ftask_create("Main");
	while(fgets(buf, 255, stdin)){		
  		program_p prog = program_create("MAIN");
  		t->program = prog;
		parse(t, buf);
		run_task(t);
		printf("\n%s", "ok ");
	}
	
	return EXIT_SUCCESS;
}
