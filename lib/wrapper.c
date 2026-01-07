#include "unp_journey.h"

void err_sys(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	fprintf(stderr, ": %s\n", strerror(errno));

	exit(1);
}

void err_quit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");

	exit(1);
}

int SOCKET(int domain, int type, int protocol)
{
        int sockfd;
        if ((sockfd = socket(domain, type, protocol)) == -1) {
                err_sys("socket fail");
        }
        return sockfd;
}

void BIND(int sockfd, struct sockaddr *addr, socklen_t addrlen)
{
        if (bind(sockfd, addr, addrlen) == -1) {
                err_sys("bind error");
        }
}

void LISTEN(int sockfd, int backlog)
{
        if (listen(sockfd, backlog) == -1) {
                err_sys("listen fail");
        }
}

int ACCEPT(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int fd = -1;
again:
	if ((fd = accept(sockfd, addr, addrlen)) < 0) {
#if EPROTO
		if (errno == EPROTO || errno == ECONNABORTED || errno == EINTR)
#else
		if (errno == ECONNABORTED || errno == EINTR)
#endif		
		{
			goto again;
		} else {
			err_sys("accept fail");
		}
	}

	return fd;
}

void CONNECT(int sockfd, struct sockaddr *addr, socklen_t addrlen)
{
	if (connect(sockfd, addr, addrlen) == -1) {
		err_sys("connect");
	}
}

void INET_PTON(int af, const char *src, void *dst)
{
	int n;
	
	if ((n = inet_pton(af, src, dst)) <= 0) {
		if (n == 0) {
			err_quit("inet_pton: string is an invalid network address");
		} else if (n == -1 && errno == EAFNOSUPPORT) {
			err_quit("inet_pton: af is an invalid address family");
		} else {
			err_quit("inet_pton fail");
		}
	}
}

char *FGETS(char *ptr, int n, FILE *stream)
{
	char *rptr;
	if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream)) {
		err_sys("fgets");
	}

	return rptr;
}

void FPUTS(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF) {
		err_sys("fputs");
	}
}

