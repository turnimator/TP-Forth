/*
 * tp_queue.c
 *
 *  Created on: Nov 5, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "tp_queue.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

tpq_p q_registry[MAX_QUEUES];
int n_queues = 0;

static int q_reg_add(tpq_p q){
	q_registry[n_queues++] = q;
	return n_queues-1;
}

static int q_reg_remove(int qn){
	return 1;
}


static tpq_p q_reg_search(void* criteria, int(f)(char*, tpq_p q)){
	for(int i = 0 ; i < MAX_QUEUES; i++){
		if (f(criteria, q_registry[i])==0){
			return q_registry[i];
		}
	}
	return 0;
}

static int cb_q_find(char*name, tpq_p q){
	return strcmp(name,q->name);
}

 tpq_p q_find(char* qname){
	return q_reg_search(qname,cb_q_find);
}




tpq_p q_create(char* name, int size){
	
	tpq_p q = malloc(sizeof(tpq_t));
	if (size>0){
		q->msg = malloc(size);
		q->msz = size;
	}
	q->full = 0;
	strncpy(q->name, name,MAX_Q_NAME_LEN);
	
	q_reg_add(q);
	return q;
}

char* q_get(tpq_p q){
	while(! q->full){
		usleep(300);
	}
	q->full = 0;
	return q->msg;
}

int q_put(tpq_p q, char* msg){
	while(q->full){
		usleep(300);
	}
	strcpy(q->msg, msg);
	q->full = 1;
	return 1;
}

void q_destroy(tpq_p q){
	if (q->msz>0){
		free(q->msg);
	}
	free(q);
}

