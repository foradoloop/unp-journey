#include "unp_journey.h"

void str_cli(FILE *, int);

int main(int argc, char **argv)
{
	if (argc != 3) {
		err_quit("usage: ./echo_client <ipaddr> <port>");
	}

	int sockfd = SOCKET(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_sa;
	bzero(&server_sa, sizeof(server_sa));
	server_sa.sin_family = AF_INET;
	server_sa.sin_port = htons(atoi(argv[2]));
	
	INET_PTON(AF_INET, argv[1], &server_sa.sin_addr);

	CONNECT(sockfd, (struct sockaddr *)&server_sa, sizeof(server_sa));

	str_cli(stdin, sockfd);

	exit(0);
}

#define max(a, b) ((a > b) ? a : b)

void str_cli(FILE *fp, int sockfd)
{
	char buf[MAXLINE];
	fd_set rset;
	FD_ZERO(&rset);
	int stdineof = 0;
	for (;;) {
		if (stdineof == 0) {
			FD_SET(fileno(fp), &rset);
		}

		FD_SET(sockfd, &rset);
		int maxfdp1 = max(fileno(fp), sockfd) + 1;
		SELECT(maxfdp1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(sockfd, &rset)) {
			int n = 0;
			if ((n = READ(sockfd, buf, MAXLINE)) == 0) {
				if (stdineof == 1) {
					return;
				} else {
					err_quit("str_cli: server terminated prematurely");
				}
			}

			WRITE(fileno(stdout), buf, n);
		}

		if (FD_ISSET(fileno(fp), &rset)) {
			int n = 0;
			if ((n = READ(fileno(fp), buf, MAXLINE)) == 0) {
				stdineof = 1;
				SHUTDOWN(sockfd, SHUT_WR);
				FD_CLR(fileno(fp), &rset);
				continue;
			}

			writen(sockfd, buf, n);
		}
	}
}

