/*	IPv6 multicast example - ipv6_multicast_recv.c
	2012 - Bjorn Lindgren <nr@c64.org>
	https://github.com/bjornl/ipv6_multicast_example
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
 
int
main(int argc, char *argv[])
{
	struct sockaddr_in6 saddr;
	char buf[1401];
	ssize_t len;
	int sd, fd, rc, on = 1, flag = 0, hops = 255, ifidx = 0;
	struct timeval tv;
	fd_set fds;

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
 
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin6_family = AF_INET6;
	saddr.sin6_port = htons(atoi(argv[2]));
	saddr.sin6_addr = in6addr_any;
 
	if (bind(sd, (struct sockaddr *) &saddr, sizeof(saddr))) {
		perror("bind");
		return 1;
	}
 
	struct group_source_req gsr;
	struct sockaddr_in6 *group;
	struct sockaddr_in6 *source;
	memset(&gsr, 0, sizeof(gsr));

	/* Set up the connection to the group */
	gsr.gsr_interface = if_nametoindex("teredo");
	group=(struct sockaddr_in6*)&gsr.gsr_group;
	source=(struct sockaddr_in6*)&gsr.gsr_source;
	group->sin6_family = AF_INET6;
	inet_pton(AF_INET6, argv[1], &group->sin6_addr);

	group->sin6_port = 0;
	
	source->sin6_family = AF_INET6;
	inet_pton(AF_INET6, argv[3], &source->sin6_addr);

	source->sin6_port = 0;
	
	if (setsockopt(sd, IPPROTO_IPV6, MCAST_JOIN_SOURCE_GROUP, (char *) &gsr, sizeof(gsr))) {
		perror("setsockopt");
		return 1;
	}

	FD_ZERO(&fds);
	FD_SET(sd, &fds);
	tv.tv_sec  = 10;
	tv.tv_usec = 0;

	fd = STDOUT_FILENO;

	while (1) {
		if (flag) {
			rc = select(sd + 1, &fds, NULL, NULL, &tv);
			if (!rc) {
				break;
			}
			tv.tv_sec  = 10;
			tv.tv_usec = 0;
		}
		len = read(sd, buf, 1400);
		buf[len] = '\0';
		/* printf("Read %zd bytes from sd\n", len); */

		if (!len) {
			break;
		} else if (len < 0) {
			perror("read");
			return 1;
		} else {
			len = write(fd, buf, len);
			/* printf("wrote %zd bytes to fd\n", len); */
			flag++;
		}
	}

	close(sd);
	close(fd);

	return 0;
}
