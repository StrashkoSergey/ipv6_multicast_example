/*	IPv6 multicast example - ipv6_multicast_send.c
	2012 - Bjorn Lindgren <nr@c64.org>
	https://github.com/bjornl/ipv6_multicast_example
*/
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
	struct sockaddr_in6 saddr;
	char buf[1400];
	ssize_t len = 1;
	int sd, fd, on = 1, hops = 255, ifidx = if_nametoindex(getenv("IF"));

	if (argc < 3) {
		printf("\nUsage: %s <address> <port>\n\nExample: %s ff02::5:6 12345\n\n", argv[0], argv[0]);
		return 1;
	}

	sd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
	if (sd < 0) {
		perror("socket");
		return 1;
	}

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
		perror("setsockopt");
		return 1;
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifidx, sizeof(ifidx))) {
		perror("setsockopt");
		return 1;
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops))) {
		perror("setsockopt");
		return 1;
	}

	if (setsockopt(sd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &on, sizeof(on))) {
		perror("setsockopt");
		return 1;
	}

	memset(&saddr, 0, sizeof(struct sockaddr_in6));
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(atoi(argv[2]));
	inet_pton(AF_INET6, argv[1], &saddr.sin6_addr);

	fd = STDIN_FILENO;

	while (len) {
		len = read(fd, buf, 1400);
		/* printf("read %zd bytes from fd\n", len); */
		if (!len) {
			break;
		} else if (len < 0) {
			perror("read");
			return 1;
		} else {
			len = sendto(sd, buf, len, 0, (const struct sockaddr *) &saddr, sizeof(saddr));
			/* printf("sent %zd bytes to sd\n", len); */
			usleep(10000); /* rate limit, 10000 = 135 kilobyte/s */
		}
	}

	close(sd);
	close(fd);

	return 0;
}
