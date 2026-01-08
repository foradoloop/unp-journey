#include "unp_journey.h"

#define PORT 8080
#define LISTENQ 10

static char buf[MAXLINE];

static void erase_cli(int sockfd, fd_set *fdset, int *client, int index);

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

	int client[FD_SETSIZE];
	fd_set allset;
	FD_ZERO(&allset);
	FD_SET(server_sock, &allset);
	int maxfd = server_sock;
	int maxi = -1;

	for (int i = 0; i < FD_SETSIZE; i++) {
		client[i] = -1;
	}

	for (;;) {
		fd_set rset = allset;

		int rfds = SELECT(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(server_sock, &rset)) {
			int client_sock = 0;
			struct sockaddr_in client_sockaddr;
			socklen_t clilen = sizeof(client_sockaddr);
			client_sock = ACCEPT(server_sock, (struct sockaddr *)&client_sockaddr, &clilen);

			if (client_sock > maxfd) {
				maxfd = client_sock;
			}

			int i = 0;
			for (; i < FD_SETSIZE; i++) {
				if (client[i] == -1) {
					client[i] = client_sock;
					break;
				}
			}
			if (i == FD_SETSIZE) {
				err_quit("server can not handle any more clients");
			}

			if (i > maxi) {
				maxi = i;
			}

			FD_SET(client_sock, &allset);

			char client_ipaddr[INET_ADDRSTRLEN] = "UNKNOWN";
			unsigned short client_port = 0;
			
			if (inet_ntop(AF_INET, &client_sockaddr.sin_addr, client_ipaddr, sizeof(client_ipaddr)) != NULL) {
				client_port = ntohs(client_sockaddr.sin_port);
			}

			fprintf(stderr, "[CONNECTED]:\t%s:%u\n", client_ipaddr, client_port);

			if (--rfds <= 0) {
				continue;
			}
		}

		for (int i = 0; i <= maxi; i++) {
			if (client[i] != -1) {
				int client_sock = 0;
				if ((client_sock = client[i]) == -1) {
					continue;
				}

				if (FD_ISSET(client_sock, &rset)) {
					int rb = 0;
					if ((rb = READ(client_sock, buf, MAXLINE)) == 0) {
						erase_cli(client_sock, &allset, client, i);
					} else {
						int wb = 0;
						if ((wb = writen(client_sock, buf, rb)) == -1) {
							erase_cli(client_sock, &allset, client, i);
						}
					}

					if (--rfds <= 0) {
						break;
					}
				}
			}
		}


	}

	return 0;
}

static void erase_cli(int sockfd, fd_set *fdset, int *client, int index)
{
	struct sockaddr_in peer_sa;
	socklen_t peerlen = sizeof(peer_sa);
	char peer_ipaddr[INET_ADDRSTRLEN] = "UNKNOWN";
	unsigned short peer_port = 0;

	if (getpeername(sockfd, (struct sockaddr *)&peer_sa, &peerlen) == 0) {
		if (inet_ntop(AF_INET, &peer_sa.sin_addr, peer_ipaddr, sizeof(peer_ipaddr)) != NULL) {
			peer_port = ntohs(peer_sa.sin_port);
		}
	}
	fprintf(stderr, "[DISCONNECTED]:\t%s:%u\n", peer_ipaddr, peer_port);

	close(sockfd);
	FD_CLR(sockfd, fdset);
	client[index] = -1;
}

