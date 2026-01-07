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

void str_cli(FILE *fp, int sockfd)
{
	char sendline[MAXLINE], recvline[MAXLINE];

	while (FGETS(sendline, MAXLINE, fp) != NULL) {
		writen(sockfd, sendline, strlen(sendline));
		
		if (readline(sockfd, recvline, MAXLINE) == 0) {
			err_quit("str_cli: server terminated prematurely");
		}

		FPUTS(recvline, stdout);
	}
}

