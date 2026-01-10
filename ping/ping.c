#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>     
#include <stdint.h>    
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/time.h>   
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>  
#include <netinet/ip.h>      
#include <netinet/ip_icmp.h> 

void GETTIMEOFDAY(struct timeval *tv, void *foo)
{
	if (gettimeofday(tv, foo) < 0) {
		perror("gettimeofday");
		exit(1);
	}
}

void SENDTO(int socket, const void *message, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
	if (sendto(socket, message, length, flags, dest_addr, dest_len) != length) {
		perror("sendto");
		exit(1);
	}
}

ssize_t RECVFROM(int socket, void *buffer, size_t length, int flags, struct sockaddr *address, socklen_t *address_len)
{
	ssize_t n = 0;
	if ((n = recvfrom(socket, buffer, length, flags, address, address_len)) < 0) {
		perror("recvfrom");
		exit(1);
	}
	return n;
}

void tv_sub(struct timeval *out, struct timeval *in);
void proc_icmp4(void *buffer, size_t len, struct timeval *tvin);
uint16_t in_cksum(uint16_t *addr, size_t len);
void make_icmp4(void *buffer, size_t len, uint8_t type, uint8_t code, uint16_t id, uint16_t seq, void *data, size_t data_len);

#define BUFSIZE 1024
uint8_t sendbuf[BUFSIZE];
uint8_t recvbuf[BUFSIZE];

uint16_t myid;

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: ./ping <ipaddr>\n");
		exit(1);
	}

	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		perror("socket");
		exit(1);
	}

	struct sockaddr_in sasend;
	memset(&sasend, 0, sizeof(sasend));
	sasend.sin_family = AF_INET;

	if (inet_pton(AF_INET, argv[1], &sasend.sin_addr) <= 0) {
		fprintf(stderr, "inet_pton fail\n");
		exit(1);
	}

	fd_set allset;
	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);

       	myid = getpid() & 0xFFFF;
	uint16_t nsent = 0;
	size_t pktlen = 8 + sizeof(struct timeval);

	for (;;) {
		fd_set rset = allset;

		struct timeval tv_out;
		GETTIMEOFDAY(&tv_out, NULL);

		make_icmp4(sendbuf, pktlen, ICMP_ECHO, 0, myid, nsent++, &tv_out, sizeof(tv_out));
		SENDTO(sockfd, sendbuf, pktlen, 0, (struct sockaddr *)&sasend, sizeof(sasend));

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int nfds = select(sockfd + 1, &rset, NULL, NULL, &timeout);
		if (nfds < 0) {
			perror("select");
			exit(1);
		} else if (nfds == 0) {
			fprintf(stderr, "request timeout for icmp_seq %d\n", nsent - 1);
		} else {
			if (FD_ISSET(sockfd, &rset)) {
				struct timeval tv_in;
				GETTIMEOFDAY(&tv_in, NULL);

				ssize_t rb = RECVFROM(sockfd, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
				if (rb > 0) {
					proc_icmp4(recvbuf, rb, &tv_in);
				}

				if (timeout.tv_sec > 0 || timeout.tv_usec > 0) {
					select(0, NULL, NULL, NULL, &timeout);
				}
			}
		}
	}
}

void tv_sub(struct timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) < 0) {
		--out->tv_sec;
		out->tv_usec += 1000000L;
	}
	out->tv_sec -= in->tv_sec;
}

void proc_icmp4(void *buffer, size_t len, struct timeval *tvin)
{
	struct ip *ip = (struct ip *)buffer;
	int iphdrlen = ip->ip_hl << 2;

	if (ip->ip_p != IPPROTO_ICMP) {
		return;
	}

	struct icmp *icmp = (struct icmp *)((uint8_t *)buffer + iphdrlen);
	int icmplen = len - iphdrlen;
	if (icmplen < 8) {
		return;
	}

	if (icmp->icmp_type == ICMP_ECHOREPLY) {
		if (ntohs(icmp->icmp_id) != myid) {
			return;
		}
		if (icmplen < 8 + sizeof(struct timeval)) {
			return;
		}

		struct timeval *tvout = (struct timeval *)icmp->icmp_data;
		tv_sub(tvin, tvout);
		double rtt = tvin->tv_sec * 1000.0 + tvin->tv_usec / 1000.0;

		fprintf(stderr, "%d bytes from %s: icmp_seq=%u, ttl=%d, time=%.2f ms\n", icmplen, inet_ntoa(ip->ip_src), ntohs(icmp->icmp_seq), ip->ip_ttl, rtt);
	}
}

uint16_t in_cksum(uint16_t *addr, size_t len)
{
	uint32_t sum = 0;
	size_t nleft = len;

	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	if (nleft == 1) {
		uint16_t temp = 0;
		*(unsigned char *)(&temp) = *(unsigned char *)addr;
		sum += temp;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	sum = ~sum;

	return ((uint16_t)sum);
}

void make_icmp4(void *buffer, size_t len, uint8_t type, uint8_t code, uint16_t id, uint16_t seq, void *data, size_t data_len)
{
	if (!buffer) {
		return;
	}

	struct icmp *icmp = (struct icmp *)buffer;
	icmp->icmp_type = type;
	icmp->icmp_code = code;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = htons(id);
	icmp->icmp_seq = htons(seq);

	memcpy(icmp->icmp_data, data, data_len);

	icmp->icmp_cksum = in_cksum(buffer, len);
}

