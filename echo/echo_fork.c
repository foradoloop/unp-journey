#include "unp_journey.h"

#define PORT 8080
#define LISTENQ 10

static char buf[MAXLINE];

void str_echo(int);

int main(void)
{
	struct sockaddr_in server_sockaddr;
	bzero(&server_sockaddr, sizeof(server_sockaddr));

	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int server_sock = SOCKET(AF_INET, SOCK_STREAM, 0);
	BIND(server_sock, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr));
	LISTEN(server_sock, LISTENQ);

	SIGNAL(SIGCHLD, sig_chld);

	for (;;) {
		struct sockaddr_in client_sockaddr;
		socklen_t clilen = sizeof(client_sockaddr);
		int client_sock;
		if ((client_sock = accept(server_sock, (struct sockaddr *)&client_sockaddr, &clilen)) < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				err_sys("accept error");
			}
		}

		char client_ipaddr[INET_ADDRSTRLEN] = "UNKNOWN";
		unsigned short client_port = 0;
		if (inet_ntop(AF_INET, &client_sockaddr.sin_addr, client_ipaddr, sizeof(client_ipaddr)) != NULL) {
			client_port = ntohs(client_sockaddr.sin_port);
		}

		pid_t chldpd = fork();
		if (chldpd == 0) {
			fprintf(stderr, "[CONNECTED]:\t%s:%u\n", client_ipaddr, client_port);
			close(server_sock);
			str_echo(client_sock);
			close(client_sock);
			fprintf(stderr, "[DISCONNECTED]:\t%s:%u\n", client_ipaddr, client_port);

			exit(0);
		} 

		close(client_sock);
	}

	return 0;
}

void str_echo(int sockfd)
{
	int close_conn = 0;
	while (!close_conn) {
		ssize_t rc = readline(sockfd, buf, MAXLINE);
		if (rc <= 0) {
			close_conn = 1;
		} else {
			ssize_t wc = writen(sockfd, buf, rc);
			if (wc == -1) {
				close_conn = 1;
			}
		}
	}
}
