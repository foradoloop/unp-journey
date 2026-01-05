#include "unp_journey.h"

void err_sys(const char *err_msg)
{
	perror(err_msg);
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

