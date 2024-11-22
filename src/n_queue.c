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
#include <unistd.h>

/**
 name - address/port e.g. 192.168.7.4/4555
*/
nq_p nq_create(char *name, int len) {
  /* extract port from address*/
  char *address_part = strtok(name, "/");
  if (!address_part) {
    printf("Error: Queue name must be on form nnn.nnn.nnn.nnn/nnnn\n");
    return 0;
  }
  char *port_part = strtok(0, "/");
  printf("Creating queue length %d with address (%s) and port %s\n", len,
         address_part, port_part);
  nq_p q = malloc(sizeof(nq_t));

  q->bufsz = len;
  q->buf = malloc(q->bufsz);
  q->port = atoi(port_part);
  bzero(&q->listen_addr, sizeof(q->listen_addr));
  q->listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  q->listen_addr.sin_port = htons(q->port);
  q->listen_addr.sin_family = AF_INET;
  q->listen_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (q->listen_socket < 0) {
    perror("listen_socket");
  }
  if (bind(q->listen_socket, (struct sockaddr *)&q->listen_addr,
           sizeof(q->listen_addr)) == -1) {
    perror("bind");
  }
  q->connect_addr.sin_addr.s_addr = inet_addr(address_part);
  q->connect_addr.sin_port = htons(q->port);
  q->connect_addr.sin_family = AF_INET;
  q->connect_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (q->connect_socket < 0) {
    perror("connect_socket");
  }
  return q;
}

char *nq_read(nq_p q) {
  socklen_t numbytes = sizeof(struct sockaddr_in);
  struct sockaddr_in cliaddr;
  if ((numbytes = recvfrom(q->listen_socket, q->buf, q->bufsz, 0,
                           (struct sockaddr *)&cliaddr, &numbytes)) < 0)
    perror("recvfrom");
  // printf("Received %d bytes: %s\n", numbytes, q->buf);
  return q->buf;
}

int nq_write(nq_p nq, char *buf) {
  // connect to server
  if (connect(nq->connect_socket, (struct sockaddr *)&nq->connect_addr,
              sizeof(nq->connect_addr)) < 0) {
    printf("\n Error : Connect Failed \n");
    return -1;
  }
  if (sendto(nq->connect_socket, buf, nq->bufsz, 0,
             (struct sockaddr *)&nq->connect_addr,
             sizeof(nq->connect_addr)) == -1) {
    perror("sendTo");
    return -1;
  }
  return 0;
}

void nq_destroy(nq_p q) {
  close(q->connect_socket);
  close(q->listen_socket);
  free(q->buf);
  free(q);
}
