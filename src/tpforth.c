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

void banner(){
	puts("-----+------------------------------------------------+----");
	puts("---- | TP-FORTH - A Forth Language P-CODE Interpreter |----");
	puts("---- |        © 2024 Jan Atle Ramsli, GPL             |----");
	puts("-----+------------------------------------------------+----");
}

int main(int ac, char*av[]) {
	
	char buf[256];
	dict_init();
	builtin_build_db();
	banner();
	ftask_p t = ftask_create("Main");
	
	char *filename = "startup.fs";
	if (ac > 1){
		filename = av[1];
	}
	char* src = read_file(filename);
	if (src){
		puts(src);
		parse(t, src);
		run_task(t);
		//free(src);
	}
	
	t = ftask_create("Main");
	while(fgets(buf, 255, stdin)){		
		parse(t, buf);
		run_task(t);
		printf("\n%s", "ok ");
	}
	
	return EXIT_SUCCESS;
}
