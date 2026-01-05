#include "unp_journey.h"

static int read_cnt = 0;
static char *read_ptr = NULL;
static char read_buf[MAXLINE];

ssize_t readn(int fd, void *vptr, size_t n)
{
	size_t nleft = n;
	char *ptr = vptr;

	while (nleft > 0) {
		ssize_t nread = 0;
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR) {
				nread = 0;
			} else {
				return -1;
			}
		} else if (nread == 0) {
			break;
		}

		nleft -= nread;
		ptr += nread;
	}

	return n - nleft;
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft = n;
	const char *ptr = vptr;

	while (nleft > 0) {
		ssize_t nwritten = 0;
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR) {
				nwritten = 0;
			} else {
				return -1;
			}
		}

		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}

static ssize_t my_read(int fd, char *ptr)
{
	if(read_cnt <= 0) {
again:
		read_cnt = read(fd, read_buf, sizeof(read_buf));
		if (read_cnt == -1) {
			if (errno == EINTR) {
				goto again;
			}
			return -1;
		} else if (read_cnt == 0) {
			return 0;
		} else {
			read_ptr = read_buf;
		}
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
	char *ptr = vptr;

	ssize_t n = 1;
	for (; n < maxlen; n++) {
		ssize_t rc = 0;
		char c = 0;
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n') {
				break;
			}
		} else if (rc == 0) {
			*ptr = '\0';
			return n - 1;
		} else {
			return -1;
		}
	}

	*ptr = '\0';
	return n;
}

ssize_t readlinebuf(void **vptrptr)
{
	if (read_cnt) {
		*vptrptr = read_ptr;
	}
	return read_cnt;
}

