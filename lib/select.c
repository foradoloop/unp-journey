#include "unp_journey.h"

int SELECT(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int fds = 0;
again:
	if ((fds = select(nfds, readfds, writefds, exceptfds, timeout)) == -1) {
		if (errno == EINTR) {
			goto again;
		} else {
			err_sys("select");
		}
	}
	return fds;
}

int SHUTDOWN(int socket, int how)
{
	if (shutdown(socket, how) == -1) {
		err_sys("shutdown");
	}

	return 0;
}


