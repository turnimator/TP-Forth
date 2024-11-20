/*
 * n_queue.h
 *
 *  Created on: Nov 18, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#ifndef N_QUEUE_H_
#define N_QUEUE_H_

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef struct n_queue {
  struct sockaddr_in servaddr;
  int readsock;
  int writesock;
  int port;
  char *buf;
  int bufsz;
} nq_t, *nq_p;

nq_p nq_create(char *name, int length);
char *nq_read(nq_p q);
int nq_write(nq_p nq, char *buf);
void nq_destroy(nq_p q);

#endif /* N_QUEUE_H_ */
