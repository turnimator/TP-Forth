/*
 * n_queue.c
 *
 *  Created on: Nov 19, 2024
 *      Author: Jan Atle Ramsli
 *
 */

#include "n_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 name - address/port e.g. 192.168.7.4/4555
*/
nq_p nq_create(char *name, int len) {
  struct addrinfo hints;
  struct addrinfo *result;
  int s;
  socklen_t length;
  struct addrinfo *rptr;

  /* extract port from address*/
  char *p = strtok(name, "/");
  if (!p) {
    printf("Error: Queue name must be on form nnn.nnn.nnn.nnn/nnnn");
    return 0;
  }
  
  nq_p q = malloc(sizeof(nq_t));
  q->bufsz = len;
  q->buf = malloc(q->bufsz);

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  sprintf(q->buf, "%u", q->port);
  if ((s = getaddrinfo(name, q->buf, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }

  /* Scan through the list of address structures returned by
     getaddrinfo. Stop when the the socket and connect calls are successful. */
  for (rptr = result; rptr != NULL; rptr = rptr->ai_next) {
    q->sock = socket(rptr->ai_family, rptr->ai_socktype, rptr->ai_protocol);
    if (q->sock == -1)
      continue;

    break;
  }

  if (rptr == NULL) {
    fprintf(stderr, "Not able to make socket\n");
    return 0;
  }

  freeaddrinfo(result);
  return q;
}

char *nq_read(nq_p q) {
  ssize_t numbytes;
  if ((numbytes = recvfrom(q->sock, q->buf, q->bufsz, 0, NULL, NULL)) == -1)
    perror("recvfrom");
  return q->buf;
}

int nq_write(nq_p nq, char *buf) {}

void nq_destroy(nq_p q) {}
