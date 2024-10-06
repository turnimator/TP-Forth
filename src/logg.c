/*
 * logg.c
 *
 *  Created on: Oct 2, 2024
 *      Author: Jan Atle Ramsli
 *
 */

int DEBUG = 0;

#include <stdio.h>
void logg(const char* f, char*s)
{
	if (DEBUG) printf("%s %s\n",f, s);
}