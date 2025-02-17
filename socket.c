/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"

/* Note: these are applicable to receiving sockopts only */
#if defined IPV6_HOPLIMIT && !defined IPV6_RECVHOPLIMIT
#define IPV6_RECVHOPLIMIT IPV6_HOPLIMIT
#endif

#if defined IPV6_PKTINFO && !defined IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif

int open_icmpv6_socket(void)
{
	int sock;
	struct icmp6_filter filter;
	int err;

	sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	if (sock < 0) {
		flog(LOG_ERR, "can't create socket(AF_INET6): %s", strerror(errno));
		return -1;
	}

	err = setsockopt(sock, IPPROTO_IPV6, IPV6_RECVPKTINFO, (int[]){1}, sizeof(int));
	if (err < 0) {
		flog(LOG_ERR, "setsockopt(IPV6_RECVPKTINFO): %s", strerror(errno));
		return -1;
	}

#ifdef __linux__
	err = setsockopt(sock, IPPROTO_RAW, IPV6_CHECKSUM, (int[]){2}, sizeof(int));
#else
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_CHECKSUM, (int[]){2}, sizeof(int));
#endif
	if (err < 0) {
		flog(LOG_ERR, "setsockopt(IPV6_CHECKSUM): %s", strerror(errno));
		return -1;
	}

	err = setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, (int[]){255}, sizeof(int));
	if (err < 0) {
		flog(LOG_ERR, "setsockopt(IPV6_UNICAST_HOPS): %s", strerror(errno));
		return -1;
	}

	err = setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (int[]){255}, sizeof(int));
	if (err < 0) {
		flog(LOG_ERR, "setsockopt(IPV6_MULTICAST_HOPS): %s", strerror(errno));
		return -1;
	}
#ifdef IPV6_RECVHOPLIMIT
	err = setsockopt(sock, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, (int[]){1}, sizeof(int));
	if (err < 0) {
		flog(LOG_ERR, "setsockopt(IPV6_RECVHOPLIMIT): %s", strerror(errno));
		return -1;
	}
#endif

	/*
	 * setup ICMP filter
	 */

	ICMP6_FILTER_SETBLOCKALL(&filter);
	ICMP6_FILTER_SETPASS(ND_ROUTER_SOLICIT, &filter);
	ICMP6_FILTER_SETPASS(ND_ROUTER_ADVERT, &filter);

	err = setsockopt(sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof(filter));
	if (err < 0) {
		flog(LOG_ERR, "setsockopt(ICMPV6_FILTER): %s", strerror(errno));
		return -1;
	}

	return sock;
}

int open_raw_socket(void)
{
	int sock;

	/* proto = 0 for RX-only */
	sock = socket(AF_PACKET, SOCK_RAW, 0);
	if (sock < 0) {
		flog(LOG_ERR, "can't create socket(AF_PACKET): %s", strerror(errno));
		return -1;
	}

	return sock;
}
