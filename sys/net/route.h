/**
 * @file
 * @copyright
 * Copyright (c) 1980, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)route.h	8.5 (Berkeley) 2/8/95
 */

/*
 * Kernel resident routing tables.
 * 
 * The routing tables are initialized when interface addresses
 * are set by making entries for all directly connected interfaces.
 */

/**
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route {
	struct rtentry *ro_rt;
	struct sockaddr ro_dst;
};

/**
 * These numbers are used by reliable protocols for determining
 * retransmission behavior and are included in the routing structure.
 */
struct rt_metrics {
	u_long rmx_locks;   ///< Kernel must leave these values alone
	u_long rmx_mtu;     ///< MTU for this path
	u_long rmx_hopcount;///< max hops expected
	u_long rmx_expire;  ///< lifetime for route, e.g. redirect
	u_long rmx_recvpipe;///< inbound delay-bandwith product
	u_long rmx_sendpipe;///< outbound delay-bandwith product
	u_long rmx_ssthresh;///< outbound gateway buffer limit
	u_long rmx_rtt;     ///< estimated round trip time
	u_long rmx_rttvar;  ///< estimated rtt variance
	u_long rmx_pksent;  ///< packets sent using this route
};

/*
 * rmx_rtt and rmx_rttvar are stored as microseconds;
 * RTTTOPRHZ(rtt) converts to a value suitable for use
 * by a protocol slowtimo counter.
 */
#define RTM_RTTUNIT 1000000 /* units for rtt, rttvar, as units per sec */
#define RTTTOPRHZ(r) ((r) / (RTM_RTTUNIT / PR_SLOWHZ))

/*
 * We distinguish between routes to hosts and routes to networks,
 * preferring the former if available.  For each route we infer
 * the interface to use from the gateway address supplied when
 * the route was entered.  Routes that forward packets through
 * gateways are marked so that the output routines know to address the
 * gateway rather than the ultimate destination.
 */
#ifndef RNF_NORMAL
#include <net/radix.h>
#endif
struct rtentry {
	struct radix_node rt_nodes[2];///< tree glue, and other values
#define rt_key(r) ((struct sockaddr *) ((r)->rt_nodes->rn_key))
#define rt_mask(r) ((struct sockaddr *) ((r)->rt_nodes->rn_mask))
	struct sockaddr *rt_gateway;///< value
	short rt_flags;             ///< up/down?, host/net
	short rt_refcnt;            ///< # held references
	u_long rt_use;              ///< raw # packets forwarded
	struct ifnet *rt_ifp;       ///< the answer: interface to use
	struct ifaddr *rt_ifa;      ///< the answer: interface to use
	struct sockaddr *rt_genmask;///< for generation of cloned routes
	caddr_t rt_llinfo;          ///< pointer to link level info cache
	struct rt_metrics rt_rmx;   ///< metrics used by rx'ing protocols
	struct rtentry *rt_gwroute; ///< implied entry for gatewayed routes
};

/**
 * Following structure necessary for 4.3 compatibility;
 * We should eventually move it to a compat file.
 */
struct ortentry {
	u_long rt_hash;            ///< to speed lookups
	struct sockaddr rt_dst;    ///< key
	struct sockaddr rt_gateway;///< value
	short rt_flags;            ///< up/down?, host/net
	short rt_refcnt;           ///< # held references
	u_long rt_use;             ///< raw # packets forwarded
	struct ifnet *rt_ifp;      ///< the answer: interface to use
};

#define RTF_UP 0x1          ///< 可用路由 route usable
#define RTF_GATEWAY 0x2     ///< 目的主机是一个网关 (非直接路由) destination is a gateway
#define RTF_HOST 0x4        ///< 主机路由 (否则,为网络路由) host entry (net otherwise)
#define RTF_REJECT 0x8      ///< 有差错的丢弃分组 host or net unreachable
#define RTF_DYNAMIC 0x10    ///< (由重定向)动态创建 created dynamically (by redirect)
#define RTF_MODIFIED 0x20   ///< (由重定向)动态修改 modified dynamically (by redirect)
#define RTF_DONE 0x40       ///< 内核的证实,表示消息处理完毕 message confirmed
#define RTF_MASK 0x80       ///< 子网掩码存在 (未使用) subnet mask present
#define RTF_CLONING 0x100   ///< 使用中产生新的路由 (由ARP使用) generate new routes on use
#define RTF_XRESOLVE 0x200  ///< 由外部守护进程解析名字 (用于X.25) external daemon resolves name
#define RTF_LLINFO 0x400    ///< 当rt_llinfo指针无效时,由ARP设置 generated by ARP or ESIS
#define RTF_STATIC 0x800    ///< 人工添加的路由 (route程序) manually added
#define RTF_BLACKHOLE 0x1000///< 无差错的丢弃分组 just discard pkts (during updates)
#define RTF_PROTO2 0x4000   ///< 协议专用的路由标志 (ARP使用) protocol specific routing flag
#define RTF_PROTO1 0x8000   ///< 协议专用的路由标志 protocol specific routing flag


/**
 * Routing statistics.
 */
struct rtstat {
	short rts_badredirect;///< 无效重定向调用的数目 bogus redirect calls
	short rts_dynamic;    ///< 由重定向创建的路由数目 routes created by redirects
	short rts_newgateway; ///< 由重定向修改的路由数目 routes modified by redirects
	short rts_unreach;    ///< 查找失败的次数 lookups which failed
	short rts_wildcard;   ///< 由通配符匹配的查找次数 (从未使用) lookups satisfied by a wildcard
};
/**
 * Structures for routing messages.
 */
struct rt_msghdr {
	u_short rtm_msglen;       ///< to skip over non-understood messages
	u_char rtm_version;       ///< future binary compatibility
	u_char rtm_type;          ///< message type
	u_short rtm_index;        ///< index for associated ifp
	int rtm_flags;            ///< flags, incl. kern & message, e.g. DONE
	int rtm_addrs;            ///< bitmask identifying sockaddrs in msg
	pid_t rtm_pid;            ///< identify sender
	int rtm_seq;              ///< for sender to identify action
	int rtm_errno;            ///< why failed
	int rtm_use;              ///< from rtentry
	u_long rtm_inits;         ///< which metrics we are initializing
	struct rt_metrics rtm_rmx;///< metrics themselves
};

#define RTM_VERSION 3///< Up the ante and ignore older versions

#define RTM_ADD 0x1     ///< 添加路由 Add Route
#define RTM_DELETE 0x2  ///< 删除路由 Delete Route
#define RTM_CHANGE 0x3  ///< 改变网关、度量或标志 Change Metrics or flags
#define RTM_GET 0x4     ///< 报告度量及其他路由信息 Report Metrics
#define RTM_LOSING 0x5  ///< 内核怀疑某路由无效 Kernel Suspects Partitioning
#define RTM_REDIRECT 0x6///< 内核得知要使用不同的路由 Told to use different route
#define RTM_MISS 0x7    ///< 查找这个地址失败 Lookup failed on this address
#define RTM_LOCK 0x8    ///< 锁定指明的度量 fix specified metrics
#define RTM_OLDADD 0x9  ///< caused by SIOCADDRT
#define RTM_OLDDEL 0xa  ///< caused by SIOCDELRT
#define RTM_RESOLVE 0xb ///< 请求将目的地址解析成链路层地址 req to resolve dst to LL addr
#define RTM_NEWADDR 0xc ///< 接口中添加了地址 address being added to iface
#define RTM_DELADDR 0xd ///< 从接口中删除地址 address being removed from iface
#define RTM_IFINFO 0xe  ///< 接口打开或关闭等 iface going up/down etc.

#define RTV_MTU 0x1      ///< 初始化或者锁住rmx_mtu init or lock _mtu
#define RTV_HOPCOUNT 0x2 ///< 初始化或者锁住rmx_hopcount init or lock _hopcount
#define RTV_EXPIRE 0x4   ///< 初始化或者锁住rmx_expire init or lock _hopcount
#define RTV_RPIPE 0x8    ///< 初始化或者锁住rmx_recvpipe init or lock _recvpipe
#define RTV_SPIPE 0x10   ///< 初始化或者锁住rmx_sendpipe init or lock _sendpipe
#define RTV_SSTHRESH 0x20///< 初始化或者锁住rmx_ssthresh init or lock _ssthresh
#define RTV_RTT 0x40     ///< 初始化或者锁住rmx_rtt init or lock _rtt
#define RTV_RTTVAR 0x80  ///< 初始化或者锁住rmx_rttvar init or lock _rttvar

/*
 * Bitmask values for rtm_addr.
 */
#define RTA_DST 0x1    ///< 目的插口地址结构 destination sockaddr present
#define RTA_GATEWAY 0x2///< 网关插口地址结构 gateway sockaddr present
#define RTA_NETMASK 0x4///< 网络掩码插口地址结构 netmask sockaddr present
#define RTA_GENMASK 0x8///< 克隆掩码插口地址结构 cloning mask sockaddr present
#define RTA_IFP 0x10   ///< 接口名称插口地址结构 interface name sockaddr present
#define RTA_IFA 0x20   ///< 接口地址插口地址结构 interface addr sockaddr present
#define RTA_AUTHOR 0x40///< 重定向产生者的插口地址结构 sockaddr for author of redirect
#define RTA_BRD 0x80   ///< 广播或点到点的目的地址 for NEWADDR, broadcast or p-p dest addr

/*
 * Index offsets for sockaddr array for alternate internal encoding.
 */
#define RTAX_DST 0    ///< 目的插口地址结构 destination sockaddr present
#define RTAX_GATEWAY 1///< 网关插口地址结构 gateway sockaddr present
#define RTAX_NETMASK 2///< 网络掩码插口地址结构 netmask sockaddr present
#define RTAX_GENMASK 3///< 克隆掩码插口地址结构 cloning mask sockaddr present
#define RTAX_IFP 4    ///< 接口名称插口地址结构 interface name sockaddr present
#define RTAX_IFA 5    ///< 接口地址插口地址结构 interface addr sockaddr present
#define RTAX_AUTHOR 6 ///< 重定向产生者的插口地址结构 sockaddr for author of redirect
#define RTAX_BRD 7    ///< 广播或点到点的目的地址 for NEWADDR, broadcast or p-p dest addr
#define RTAX_MAX 8    ///< rti-into[]数组的元素个数 size of array to allocate

struct rt_addrinfo {
	int rti_addrs;
	struct sockaddr *rti_info[RTAX_MAX];
};

struct route_cb {
	int ip_count;
	int ns_count;
	int iso_count;
	int any_count;
};

#ifdef KERNEL
#define RTFREE(rt)           \
	if((rt)->rt_refcnt <= 1) \
		rtfree(rt);          \
	else                     \
		(rt)->rt_refcnt--;

struct route_cb route_cb;///< 选路插口监听器的数目,每个协议的数目及总的数目
struct rtstat rtstat;    ///< 路由选择统计
struct radix_node_head *rt_tables[AF_MAX + 1];

struct socket;

void route_init __P((void) );
int route_output __P((struct mbuf *, struct socket *) );
int route_usrreq __P((struct socket *,
                      int, struct mbuf *, struct mbuf *, struct mbuf *) );
void rt_ifmsg __P((struct ifnet *) );
void rt_maskedcopy __P((struct sockaddr *,
                        struct sockaddr *, struct sockaddr *) );
void rt_missmsg __P((int, struct rt_addrinfo *, int, int) );
void rt_newaddrmsg __P((int, struct ifaddr *, int, struct rtentry *) );
int rt_setgate __P((struct rtentry *,
                    struct sockaddr *, struct sockaddr *) );
void rt_setmetrics __P((u_long, struct rt_metrics *, struct rt_metrics *) );
void rtable_init __P((void **) );
void rtalloc __P((struct route *) );
struct rtentry *
        rtalloc1 __P((struct sockaddr *, int) );
void rtfree __P((struct rtentry *) );
int rtinit __P((struct ifaddr *, int, int) );
int rtioctl __P((u_long, caddr_t, struct proc *) );
void rtredirect __P((struct sockaddr *, struct sockaddr *,
                     struct sockaddr *, int, struct sockaddr *, struct rtentry **) );
int rtrequest __P((int, struct sockaddr *,
                   struct sockaddr *, struct sockaddr *, int, struct rtentry **) );
#endif
