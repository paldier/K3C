#ifndef __PPA_IP_PROTOCOL_TYPES_
#define __PPA_IP_PROTOCOL_TYPES_

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/if_pppox.h>

#include <net/ip.h>

struct ip6_addr {
	uint32_t ip[4];
};

inline void swa_ipv6_addr_copy(struct ip6_addr* dst, struct ip6_addr* src)
{
	swa_memcpy((void*)dst, (void*)src, sizeof(struct ip6_addr) );
}

#endif
