#ifndef UNP_JOURNEY_H
#define UNP_JOURNEY_H

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>

// Wrappers
int SOCKET(int domain, int type, int protocol);
void BIND(int sockfd, struct sockaddr *addr, socklen_t addrlen);
void LISTEN(int sockfd, int backlog);
int ACCEPT(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void err_sys(const char *err_msg);

#define MAXLINE 100

// I/O Functions
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readline(int fd, void *vptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);

// Signal Handler
typedef void Sigfunc(int);
Sigfunc *SIGNAL(int signo, Sigfunc *func);
void sig_chld(int signo);

#endif
