/*
 * queue
 *
 *  Created on: Nov 5, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef TP_QUEUE_H_
#define TP_QUEUE_H_

#define MAX_QUEUES 128
#define MAX_Q_NAME_LEN 12

typedef struct tp_queue {
	char name[MAX_Q_NAME_LEN];
	char*msg;
	int msz; // Message size
	int full;
} tpq_t,*tpq_p;

tpq_p q_create(char* name, int size);
char* q_get(tpq_p q);
int q_put(tpq_p q, char* msg);
void q_destroy(tpq_p q);
tpq_p q_find(char* qname);

#endif /* TP_QUEUE_H_ */
