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
  bzero(&q->listen_addr, sizeof(q->listen_addr));
  q->listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  q->listen_addr.sin_port = htons(q->port);
  q->listen_addr.sin_family = AF_INET;
  q->listen_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (bind(q->listen_socket, (struct sockaddr *)&q->listen_addr,
           sizeof(q->listen_addr)) == -1) {
    perror("bind");
  }
  q->connect_addr.sin_addr.s_addr = inet_addr(address_part);
  q->connect_addr.sin_port = htons(q->port);
  q->connect_addr.sin_family = AF_INET;
  q->client_socket = socket(AF_INET, SOCK_DGRAM, 0);
  return q;
}

char *nq_read(nq_p q) {
  socklen_t numbytes = sizeof(struct sockaddr_in);
  struct sockaddr_in cliaddr;
  if ((numbytes = recvfrom(q->listen_socket, q->buf, q->bufsz, 0,
                           (struct sockaddr *)&cliaddr, &numbytes)) == -1)
    perror("recvfrom");
  return q->buf;
}

int nq_write(nq_p nq, char *buf) {
  // connect to server
  if (connect(nq->listen_socket, (struct sockaddr *)&nq->listen_addr,
              sizeof(nq->listen_addr)) < 0) {
    printf("\n Error : Connect Failed \n");
    return -1;
  }
  if (sendto(nq->listen_socket, buf, nq->bufsz, 0,
             (struct sockaddr *)&nq->listen_addr,
             sizeof(nq->listen_addr)) == -1) {
    perror("sendTo");
    return -1;
  }
  return 0;
}

void nq_destroy(nq_p q) {}
