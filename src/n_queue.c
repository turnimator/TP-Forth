/*
 * n_queue.c
 *
 *  Created on: Nov 19, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "n_queue.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

/**
 name - address/port e.g. 192.168.7.4/4555
*/
nq_p nq_create(char *name, int len) {
  /* extract port from address*/
  char *address_part = strtok(name, "/");
  if (!address_part) {
    printf("Error: Queue name must be on form nnn.nnn.nnn.nnn/nnnn");
    return 0;
  }
  char *port_part = strtok(0, "/");
  printf("Creating queue with address %s and port %s\n", address_part,
         port_part);
  nq_p q = malloc(sizeof(nq_t));
  q->bufsz = len;
  q->buf = malloc(q->bufsz);
  q->port = atoi(port_part);
  bzero(&q->servaddr, sizeof(q->servaddr));
  q->servaddr.sin_addr.s_addr = inet_addr(address_part);
  q->servaddr.sin_port = htons(q->port);
  q->servaddr.sin_family = AF_INET;
  q->readsock = socket(AF_INET, SOCK_DGRAM, 0);
  return q;
}

char *nq_read(nq_p q) {
  socklen_t numbytes = sizeof(struct sockaddr_in);
  if (bind(q->readsock, (struct sockaddr *)&q->servaddr, sizeof(q->servaddr)) ==
      -1) {
    perror("bind");
  }

  if ((numbytes = recvfrom(q->readsock, q->buf, q->bufsz, 0,
                           (struct sockaddr *)&q->servaddr, &numbytes)) == -1)
    perror("recvfrom");
  return q->buf;
}

int nq_write(nq_p nq, char *buf) {
  // connect to server
  if (connect(nq->readsock, (struct sockaddr *)&nq->servaddr,
              sizeof(nq->servaddr)) < 0) {
    printf("\n Error : Connect Failed \n");
    return -1;
  }
  if (sendto(nq->readsock, buf, nq->bufsz, 0, (struct sockaddr *)&nq->servaddr,
             sizeof(nq->servaddr)) == -1) {
    perror("sendTo");
    return -1;
  }
  return 0;
}

void nq_destroy(nq_p q) {}
