#include "unp_journey.h"

#define PORT 8080
#define LISTENQ 10

static char buf[MAXLINE];

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

	for (;;) {
		struct sockaddr_in client_sockaddr;
		socklen_t clilen = sizeof(client_sockaddr);
		int client_sock = ACCEPT(server_sock, (struct sockaddr *)&client_sockaddr, &clilen);

		char client_ipaddr[INET_ADDRSTRLEN] = "UNKNOWN";
		unsigned short client_port = 0;
		if (inet_ntop(AF_INET, &client_sockaddr.sin_addr, client_ipaddr, sizeof(client_ipaddr)) != NULL) {
			client_port = ntohs(client_sockaddr.sin_port);
		}
		fprintf(stderr, "[CONNECTED]:\t%s:%u\n", client_ipaddr, client_port);

		int close_conn = 0;
		while (!close_conn) {
			ssize_t rc = readline(client_sock, buf, MAXLINE);
			if (rc <= 0) {
				close_conn = 1;
			} else {
				ssize_t wc = writen(client_sock, buf, rc);
				if (wc == -1) {
					close_conn = 1;
				}
			}
		}

		fprintf(stderr, "[DISCONNECTED]:\t%s:%u\n", client_ipaddr, client_port);

		close(client_sock);
	}

	return 0;
}

