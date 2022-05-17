/*
   +----------------------------------------------------------------------+
   | Xdebug                                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2002-2022 Derick Rethans                               |
   +----------------------------------------------------------------------+
   | This source file is subject to version 1.01 of the Xdebug license,   |
   | that is bundled with this package in the file LICENSE, and is        |
   | available at through the world-wide-web at                           |
   | https://xdebug.org/license.php                                       |
   | If you did not receive a copy of the Xdebug license and are unable   |
   | to obtain it through the world-wide-web, please send a note to       |
   | derick@xdebug.org so we can mail you a copy immediately.             |
   +----------------------------------------------------------------------+
 */
#ifdef __linux__

#include "php_xdebug.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include "ip_info.h"

#if XDEBUG_GATEWAY_SUPPORT

#define BUFFER_SIZE		4096

static char *convert_to_quad(int domain, void *buf)
{
	char *ip = xdcalloc(1, INET6_ADDRSTRLEN + 1);

	inet_ntop(domain, buf, ip, INET6_ADDRSTRLEN);

	return ip;
}

static int get_ip(int fd, struct sockaddr_nl *sa, int domain)
{
	char              buf[BUFFER_SIZE];
	struct nlmsghdr  *nl;
	struct ifaddrmsg *ifa;
	struct iovec      iov = { 0 };
	struct msghdr     msg = { 0 };
	int               r;

	memset(buf, 0, BUFFER_SIZE);

	/* Assemble the message according to the netlink protocol */
	nl = (struct nlmsghdr*)buf;
	nl->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	nl->nlmsg_type = RTM_GETADDR;
	nl->nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;

	ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);
	ifa->ifa_family = domain; // We only get IPv4 address here

	/* Prepare struct msghdr for sending */
	iov.iov_base = nl;
	iov.iov_len = nl->nlmsg_len;
	msg.msg_name = sa;
	msg.msg_namelen = sizeof(*sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	/* Send netlink message to kernel */
	r = sendmsg(fd, &msg, 0);
	return (r < 0) ? -1 : 0;
}

static int get_msg(int fd, struct sockaddr_nl *sa, void *buf, size_t len)
{
	struct iovec iov;
	struct msghdr msg;

	iov.iov_base = buf;
	iov.iov_len = len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = sa;
	msg.msg_namelen = sizeof(*sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	return recvmsg(fd, &msg, 0);
}

static char *parse_ifa_msg(struct ifaddrmsg *ifa, void *buf, size_t len, const char *wanted_iface)
{
	char           ifname[IF_NAMESIZE];
	struct rtattr *rta = NULL;
	int            fa  = ifa->ifa_family;

	if_indextoname(ifa->ifa_index, ifname);

	if (strcmp(ifname, wanted_iface) != 0) {
		return NULL;
	}

	for (rta = (struct rtattr*)buf; RTA_OK(rta, len); rta = RTA_NEXT(rta, len)) {
		if (rta->rta_type == IFA_ADDRESS) {
			return convert_to_quad(fa, RTA_DATA(rta));
		}
	}

	return NULL;
}

static uint32_t parse_nl_msg(void *buf, size_t len, const char *iface, char **if_address)
{
	struct nlmsghdr *nl = NULL;
	uint32_t nlmsg_type = NLMSG_ERROR;

	for (
		nl = (struct nlmsghdr*)buf;
		NLMSG_OK(nl, (uint32_t)len) && nl->nlmsg_type != NLMSG_DONE;
		nl = NLMSG_NEXT(nl, len)
	) {
		nlmsg_type = nl->nlmsg_type;

		if (nl->nlmsg_type == NLMSG_ERROR) {
			return -1;
		}

		if (nl->nlmsg_type == RTM_NEWADDR) {
			struct ifaddrmsg *ifa;
			ifa = (struct ifaddrmsg*)NLMSG_DATA(nl);

			if (*if_address == NULL) {
				*if_address = parse_ifa_msg(ifa, IFA_RTA(ifa), IFA_PAYLOAD(nl), iface);
				if (*if_address) {
					return NLMSG_DONE;
				}
			}
			continue;
		}
	}
	return nlmsg_type;
}

char *xdebug_get_ip_for_interface(const char *iface)
{
	char      buf[BUFFER_SIZE];
	int       fd = 0, len = 0;
	char     *if_address = NULL;
	uint32_t  nl_msg_type = NLMSG_DONE;
	struct    sockaddr_nl sa;

	/* Create a socket with the AF_NETLINK domain */
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (fd < 0) {
		return NULL;
	}

	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;

	len = get_ip(fd, &sa, AF_INET); // For IPv6: use AF_INET6 instead
	if (len < 0) {
		return NULL;
	}

	do {
		len = get_msg(fd, &sa, buf, BUFFER_SIZE);
		if (len < 0) {
			return NULL;
		}

		nl_msg_type = parse_nl_msg(buf, len, iface, &if_address);
	} while (nl_msg_type != NLMSG_DONE && nl_msg_type != NLMSG_ERROR);

	return if_address;
}

/***/

static int get_gateway_and_iface(in_addr_t *addr, char *interface)
{
    long  destination, gateway;
    char  iface[IF_NAMESIZE];
    char  buf[BUFFER_SIZE];
    FILE *file;

    memset(iface, 0, sizeof(iface));
    memset(buf, 0, sizeof(buf));

    file = fopen("/proc/net/route", "r");
    if (!file) {
        return 0;
	}

    while (fgets(buf, sizeof(buf), file)) {
        if (sscanf(buf, "%s %lx %lx", iface, &destination, &gateway) == 3) {
            if (destination == 0) { /* default */
                *addr = gateway;
                strcpy(interface, iface);
                fclose(file);
                return 1;
            }
        }
    }

    /* default route not found */
    if (file) {
        fclose(file);
	}
    return 0;
}

char *xdebug_get_gateway_ip(void)
{
    in_addr_t addr = 0;
    char      iface[IF_NAMESIZE];

    memset(iface, 0, sizeof(iface));

    if (get_gateway_and_iface(&addr, iface)) {
		return xdstrdup(inet_ntoa(*(struct in_addr *) &addr));
	}

    return NULL;
}
#endif /* XDEBUG_GATEWAY_SUPPORT */

/***/

#if XDEBUG_NAMESERVER_SUPPORT
char *xdebug_get_private_nameserver(void)
{
	res_state res = malloc(sizeof(struct __res_state));
	in_addr_t ns_addr = 0;
	char nameserver_buf[20];
	char *nameserver = NULL;

	res_ninit(res);
	if (res->nscount > 0 && res->nsaddr_list[0].sin_family == AF_INET) {
		ns_addr = res->nsaddr_list[0].sin_addr.s_addr;
		/* Only allow private networks */
		if (
			((ns_addr & 0x000000ff) == 0x0000000a) || // 10.x.x.x/24
			((ns_addr & 0x0000f0ff) == 0x000010ac) || // 172.16.x.x/20
			((ns_addr & 0x0000ffff) == 0x0000a8c0) || // 192.168.x.x/16
			((ns_addr & 0x000000ff) == 0x0000007f)    // 127.x.x.x/8
		)
		{
			snprintf(
				nameserver_buf, 16, "%d.%d.%d.%d",
				(ns_addr & 0xff),
				(ns_addr & 0xff00) >> 8,
				(ns_addr & 0xff0000) >> 16,
				(ns_addr & 0xff000000) >> 24
			);
			nameserver = xdstrdup(nameserver_buf);
		}
	}
	res_nclose(res);
	free(res);

	return nameserver;
}
#endif  /* XDEBUG_NAMESERVER_SUPPORT */


# if 0
int main(int argc, char *argv[])
{
	char *gateway_address = xdebug_get_gateway_ip();
	char *ip_address      = xdebug_get_ip_for_interface(argv[1]);

	printf("Gateway: %s\n", gateway_address);
	printf("IP: %s\n", ip_address);

	free(ip_address);
	free(gateway_address);
}
# endif // int main

#endif // __linux__
