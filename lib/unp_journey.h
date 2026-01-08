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
#include <stdarg.h>
#include <sys/select.h>
#include <sys/time.h>

// Wrappers
int SOCKET(int domain, int type, int protocol);
void BIND(int sockfd, struct sockaddr *addr, socklen_t addrlen);
void LISTEN(int sockfd, int backlog);
int ACCEPT(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
void CONNECT(int sockfd, struct sockaddr *addr, socklen_t addrlen);

void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);

void INET_PTON(int af, const char *src, void *dst);

char *FGETS(char *ptr, int n, FILE *stream);
void FPUTS(const char *ptr, FILE *stream);

ssize_t READ(int fd, void *buf, size_t count);
ssize_t WRITE(int fd, const void *buf, size_t count);

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

// Select
int SELECT(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
int SHUTDOWN(int socket, int how);

#endif
