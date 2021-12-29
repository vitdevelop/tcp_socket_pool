#include "lib/pthread_pool/error_util.h"
#include "lib/pthread_pool/pthread_pool.h"
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 32

struct clientSocket {
  int fd;
  struct sockaddr_in address;
};

int listenServer(char *serverAddress, short port);
void *handleClient(void *arg);

static struct ThreadPool *threadPool;
static int sfd;

int listenServer(char *serverAddress, short port) {
  int sfd, optval;
  struct sockaddr_in echoServAddr;
  char serverAddrStr[INET_ADDRSTRLEN];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1)
    errExit("socket");

  optval = 1;
  if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    errExit("setsockopt");

  memset(&echoServAddr, 0, sizeof(echoServAddr));

  echoServAddr.sin_family = AF_INET;
  if (inet_pton(AF_INET, serverAddress, &echoServAddr.sin_addr) == -1)
    errExit("inet_pton");
  echoServAddr.sin_port = htons(port);

  if (bind(sfd, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) == -1)
    errExit("bind");

  if (listen(sfd, 0) == -1)
    errExit("listen");

  if (inet_ntop(AF_INET, &echoServAddr.sin_addr, serverAddrStr,
                INET_ADDRSTRLEN) == NULL)
    errExit("inet_ntop");

  printf("Server started on address %s:%d\n", serverAddrStr,
         ntohs(echoServAddr.sin_port));

  return sfd;
}

void *handleClient(void *arg) {
  int cfd;
  struct sockaddr_in clientAddr;
  char clientAddress[INET_ADDRSTRLEN];
  char echoBuf[BUF_SIZE];
  int receiveMsgSize;

  struct clientSocket *clientSocket = (struct clientSocket *)arg;
  memcpy(&cfd, &clientSocket->fd, sizeof(int));
  memcpy(&clientAddr, &clientSocket->address, sizeof(struct sockaddr_in));
  free(arg);
  arg = NULL;
  clientSocket = NULL;

  if (inet_ntop(AF_INET, &clientAddr.sin_addr, clientAddress,
                INET_ADDRSTRLEN) == NULL)
    errExit("inet_ntop");

  printf("Connected user %s:%d\n", clientAddress, ntohs(clientAddr.sin_port));

  if ((receiveMsgSize = recv(cfd, echoBuf, BUF_SIZE, 0)) == -1)
    errExit("recv");

  while (receiveMsgSize > 0) {
    if (send(cfd, echoBuf, receiveMsgSize, 0) == -1)
      errExit("send");

    if ((receiveMsgSize = recv(cfd, echoBuf, BUF_SIZE, 0)) == -1)
      errExit("recv");
  }

  close(cfd);
  printf("Disconnected user %s:%d\n", clientAddress,
         ntohs(clientAddr.sin_port));

  return NULL;
}
static void sigHandler(int sig) {
  close(sfd);
  ThreadPool_shutdown(threadPool);
}

int main(int argc, char *argv[]) {
  struct sockaddr_in clientAddr;
  threadPool = ThreadPool_new();

  if (signal(SIGINT, sigHandler) == SIG_ERR)
    errExit("signal");

  sfd = listenServer("127.0.0.1", 8080);

  for (;;) {
    socklen_t len = sizeof(struct sockaddr_in);
    int cfd = accept(sfd, (struct sockaddr *)&clientAddr, &len);
    if (cfd == -1) {
      errExit("accept");
    }

    struct clientSocket *clientData =
        (struct clientSocket *)malloc(sizeof(struct clientSocket));
    clientData->fd = cfd;
    clientData->address = clientAddr;

    ThreadPool_addTask(threadPool, handleClient, clientData);
  }

  exit(EXIT_SUCCESS);
}
