/**
 * @copyright
 * Copyright (c) 1982, 1986, 1989, 1993
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
 *	@(#)if.h	8.3 (Berkeley) 2/9/95
 */

/**
 * @file
 * 接口结构定义
 *
 * Structures defining a network interface, providing a packet
 * transport mechanism (ala level 0 of the PUP protocols).
 *
 * Each interface accepts output datagrams of a specified maximum
 * length, and provides higher level routines with input datagrams
 * received from its medium.
 *
 * Output occurs when the routine if_output is called, with three parameters:
 *	(*ifp->if_output)(ifp, m, dst, rt)
 * Here m is the mbuf chain to be sent and dst is the destination address.
 * The output routine encapsulates the supplied datagram if necessary,
 * and then transmits it on its medium.
 *
 * On input, each interface unwraps the data received by it, and either
 * places it on the input queue of a internetwork datagram routine
 * and posts the associated software interrupt, or passes the datagram to a raw
 * packet input routine.
 *
 * Routines exist for locating interfaces by their addresses
 * or for locating a interface on a certain network, as well as more general
 * routing and gateway routines maintaining information used to locate
 * interfaces.  These routines live in the files if.c and route.c
 */
#ifndef _TIME_//  XXX fast fix for SNMP, going away soon
#include <sys/time.h>
#endif

#ifdef __STDC__
/*
 * Forward structure declarations for function prototypes [sic].
 */
struct mbuf;
struct proc;
struct rtentry;
struct socket;
struct ether_header;
#endif
/*
 * Structure describing information about an interface
 * which may be of interest to management entities.
 */
/*
 * Structure defining a queue for a network interface.
 *
 * (Would like to call this struct ``if'', but C isn't PL/1.)
 */

struct ifnet {
	char *if_name;             ///< 接口的文本名称 是一个短字符串，用于标识接口的类型，而if_unit标识多个相同类型的实例。例如，一个系统有两个SLIP接口，每个都有一个if_name，包含两字节的“s1”和一个if_unit。对第一个接口，if_unit为0；对第二个接口为1。if_index在内核中唯一地标识这个接口，这在sysctl系统调用以及路由域中要用到。 name, e.g. ``en'' or ``lo''
	struct ifnet *if_next;     ///< 把所有接口的ifnet结构链接成一个链表。函数if_attach在系统初始化期间构造这个链表。if_addrlist指向这个接口的ifaddr结构列表。每个ifaddr结构存储一个要用这个接口通信的协议的地址信息。 all struct ifnets are chained
	struct ifaddr *if_addrlist;///< linked list of addresses per if
	int if_pcount;             ///< number of promiscuous listeners
	caddr_t if_bpf;            ///< packet filter structure
	u_short if_index;          ///< 唯一地标识接口 numeric abbreviation for this if
	short if_unit;             ///< sub-unit for lower level driver
	short if_timer;            ///< 以秒为单位记录时间，直到内核为此接口调用函数if_watchdog为止。这个函数用于设备驱动程序定时收集接口统计，或用于复位运行不正确的硬件 time 'til if_watchdog called
	short if_flags;            ///< 表明接口的操作状态和属性。一个进程能检查所有的标志，但不能改变在图3-7中“内核专用”列中作了记号的标志。这些标志用4.4节讨论的命令SIOCGIFFLAGS和SIOCSIFFLAGS来访问。 up/down, broadcast, etc.
	struct if_data {
		/* generic interface information */
		u_char ifi_type;              ///< 接口的类型 指明接口支持的硬件地址类型 ethernet, tokenring, etc
		u_char ifi_addrlen;           ///< 数据链路地址的长度 media address length
		u_char ifi_hdrlen;            ///< 由硬件附加给任何分组的首部的长度 media header length
		u_long ifi_mtu;               ///< 接口的MTU(字节) 接口传输单元的最大值：接口在一次输出操作中能传输的最大数据单元的字节数。这是控制网络和传输协议创建分组大小的重要参数。对于以太网来说，这个值是1500。maximum transmission unit
		u_long ifi_metric;            ///< 通常是0；其他更大的值不利于路由通过此接口。 routing metric (external only)
		u_long ifi_baudrate;          ///< 指定接口的传输速率，只有SLIP接口才设置它。 linespeed
		                              /* volatile statistics */
		u_long ifi_ipackets;          ///< 在接口接收到的分组数 packets received on interface
		u_long ifi_ierrors;           ///< 接收到的有输入差错分组数 input errors on interface
		u_long ifi_opackets;          ///< 接口上发送的分组数 packets sent on interface
		u_long ifi_oerrors;           ///< 接口上输出的差错数 output errors on interface
		u_long ifi_collisions;        ///< 在CSMA接口的冲突数 if_collisionscollisions on csma interfaces
		u_long ifi_ibytes;            ///< 接收到的字节总数 total number of octets received
		u_long ifi_obytes;            ///< 发送的字节总数 total number of octets sent
		u_long ifi_imcasts;           ///< 接收到的多播分组数 packets received via multicast
		u_long ifi_omcasts;           ///< 发送的多播分组数 packets sent via multicast
		u_long ifi_iqdrops;           ///< 被此接口丢失的输入分组数 dropped on input, this interface
		u_long ifi_noproto;           ///< 指定为不支持协议的分组数 destined for unsupported protocol
		struct timeval ifi_lastchange;///< 上一次改变统计的时间 记录任何统计改变的最近时间 last updated
	} if_data;                        ///< 用来描述接口的硬件特性
	/* procedure handles */
	int(*if_init)/// 初始化接口 init routine
	        __P((int) );
	int(*if_output)/// 对要传输的输出分组进行排队 output routine (enqueue)
	        __P((struct ifnet *, struct mbuf *, struct sockaddr *,
	             struct rtentry *) );
	int(*if_start)/// 启动分组的传输 initiate output routine
	        __P((struct ifnet *) );
	int(*if_done)                  /// 传输完成后的清除 (未用) output complete routine
	        __P((struct ifnet *) );/// (XXX not used; fake prototype)
	int(*if_ioctl)                 /// 处理I/O控制命令 ioctl routine
	        __P((struct ifnet *, u_long, caddr_t));
	int(*if_reset)
	        __P((int) );/// 复位接口设备 new autoconfig will permit removal
	int(*if_watchdog)   /// 周期性接口例程 timer routine
	        __P((int) );
	struct ifqueue {
		struct mbuf *ifq_head;///< 指向队列的第一个分组 (下一个要输出的分组)
		struct mbuf *ifq_tail;///< 指向队列最后一个分组
		int ifq_len;          ///< 是当前队列中分组的数目输出队列中的分组数
		int ifq_maxlen;       ///< 队列中允许的缓存最大个数
		int ifq_drops;        ///< 统计因为队列满而丢弃的分组数 在输出期间丢失的分组数
	} if_snd;                 ///< 接口输出分组队列 output queue
};
#define if_mtu if_data.ifi_mtu  ///< 接口的MTU(字节)
#define if_type if_data.ifi_type///< 接口的类型
#define if_addrlen if_data.ifi_addrlen
#define if_hdrlen if_data.ifi_hdrlen
#define if_metric if_data.ifi_metric
#define if_baudrate if_data.ifi_baudrate
#define if_ipackets if_data.ifi_ipackets    ///< 在接口接收到的分组数
#define if_ierrors if_data.ifi_ierrors      ///< 接收到的有输入差错分组数
#define if_opackets if_data.ifi_opackets    ///< 接口上发送的分组数
#define if_oerrors if_data.ifi_oerrors      ///< 接口上输出的差错数
#define if_collisions if_data.ifi_collisions///< 在CSMA接口的冲突数
#define if_ibytes if_data.ifi_ibytes        ///< 接收到的字节总数
#define if_obytes if_data.ifi_obytes        ///< 发送的字节总数
#define if_imcasts if_data.ifi_imcasts      ///< 接收到的多播分组数
#define if_omcasts if_data.ifi_omcasts      ///< 发送的多播分组数
#define if_iqdrops if_data.ifi_iqdrops      ///< 被此接口丢失的输入分组数
#define if_noproto if_data.ifi_noproto      ///< 指定为不支持协议的分组数
#define if_lastchange if_data.ifi_lastchange///< 上一次改变统计的时间

#define IFF_UP 0x1          ///< 接口正在工作 interface is up
#define IFF_BROADCAST 0x2   ///< 接口用于广播网 broadcast address valid
#define IFF_DEBUG 0x4       ///< 这个接口允许调试 turn on debugging
#define IFF_LOOPBACK 0x8    ///< 接口用于环回网络 is a loopback net
#define IFF_POINTOPOINT 0x10///< 接口用于点对点网络 interface is point-to-point link
#define IFF_NOTRAILERS 0x20 ///< 避免使用尾部封装 avoid use of trailers
#define IFF_RUNNING 0x40    ///< 资源已分配给这个接口 resources allocated
#define IFF_NOARP 0x80      ///< 在这个接口上不使用ARP协议 no address resolution protocol
#define IFF_PROMISC 0x100   ///< 接口接收所有网络分组 receive all packets
#define IFF_ALLMULTI 0x200  ///< 接口正接收所有多播分组 receive all multicast packets
#define IFF_OACTIVE 0x400   ///< 正在传输数据 transmission in progress
#define IFF_SIMPLEX 0x800   ///< 接口不能接收它自己发送的数据 can't hear own transmissions
#define IFF_LINK0 0x1000    ///< 由设备驱动程序定义 per link layer defined bit
#define IFF_LINK1 0x2000    ///< 由设备驱动程序定义 per link layer defined bit
#define IFF_LINK2 0x4000    ///< 由设备驱动程序定义 per link layer defined bit
#define IFF_MULTICAST 0x8000///< 接口支持多播 supports multicast

/// 对所有在“内核专用”列中作了记号的标志进行按位“或”操作 flags set internally only:
#define IFF_CANTCHANGE                                             \
	(IFF_BROADCAST | IFF_POINTOPOINT | IFF_RUNNING | IFF_OACTIVE | \
	 IFF_SIMPLEX | IFF_MULTICAST | IFF_ALLMULTI)

/*
 * Output queues (ifp->if_snd) and internetwork datagram level (pup level 1)
 * input routines have queues of messages stored on ifqueue structures
 * (defined above).  Entries are added to and deleted from these structures
 * by these macros, which should be called with ipl raised to splimp().
 */
#define IF_QFULL(ifq) ((ifq)->ifq_len >= (ifq)->ifq_maxlen)///< ifq是否满
#define IF_DROP(ifq) ((ifq)->ifq_drops++)                  ///< IF_DROP仅将与ifq关联的ifq_drops加1。这个名字会引起误导:调用者丢弃这个分组
/**
 * 把分组m追加到ifq队列的后面。分组通过mbuf首部中的m_nextpkt链接在一起
 */
#define IF_ENQUEUE(ifq, m)                  \
	{                                       \
		(m)->m_nextpkt = 0;                 \
		if((ifq)->ifq_tail == 0)            \
			(ifq)->ifq_head = m;            \
		else                                \
			(ifq)->ifq_tail->m_nextpkt = m; \
		(ifq)->ifq_tail = m;                \
		(ifq)->ifq_len++;                   \
	}
/**
 * 把分组m插入到ifq队列的前面
 */
#define IF_PREPEND(ifq, m)                \
	{                                     \
		(m)->m_nextpkt = (ifq)->ifq_head; \
		if((ifq)->ifq_tail == 0)          \
			(ifq)->ifq_tail = (m);        \
		(ifq)->ifq_head = (m);            \
		(ifq)->ifq_len++;                 \
	}
/**
 * 从ifq队列中取走第一个分组。m指向取走的分组,若队列为空,则m为空值
 */
#define IF_DEQUEUE(ifq, m)                              \
	{                                                   \
		(m) = (ifq)->ifq_head;                          \
		if(m) {                                         \
			if(((ifq)->ifq_head = (m)->m_nextpkt) == 0) \
				(ifq)->ifq_tail = 0;                    \
			(m)->m_nextpkt = 0;                         \
			(ifq)->ifq_len--;                           \
		}                                               \
	}

#define IFQ_MAXLEN 50
#define IFNET_SLOWHZ 1///< granularity is 1 second

/**
 * 通过ifa_next把分配给一个接口的所有地址链接起来，它还包括一个指回接口的ifnet结构的指针ifa_ifp
 * The ifaddr structure contains information about one address
 * of an interface.  They are maintained by the different address families,
 * are allocated and attached when an address is set, and are linked
 * together so all addresses for an interface can be located.
 */
struct ifaddr {
	struct sockaddr *ifa_addr;   ///< ifa_addr指向接口的一个协议地址，而ifa_netmask指向一个位掩码，它用于选择ifa_addr中的网络部分。地址中表示网络部分的比特在掩码中被设置为1，地址中表示主机的部分被设置为0 address of interface
	struct sockaddr *ifa_dstaddr;///< other end of p-to-p link
#define ifa_broadaddr ifa_dstaddr///< broadcast address interface
	struct sockaddr *ifa_netmask;///< used to determine subnet
	struct ifnet *ifa_ifp;       ///< back-pointer to interface
	struct ifaddr *ifa_next;     ///< next address for interface
	void (*ifa_rtrequest)();     ///< check or clean routes (+ or -)'d
	u_short ifa_flags;           ///< mostly rt_flags for cloning
	short ifa_refcnt;            ///< extra to malloc for link info
	int ifa_metric;              ///< cost of going out this interface
#ifdef notdef
	struct rtentry *ifa_rt; /* XXXX for ROUTETOIF ????? */
#endif
};
#define IFA_ROUTE RTF_UP///< route installed

/**
 * Message format for use in obtaining information about interfaces
 * from getkerninfo and the routing socket
 */
struct if_msghdr {
	u_short ifm_msglen;     ///< to skip over non-understood messages
	u_char ifm_version;     ///< future binary compatability
	u_char ifm_type;        ///< message type
	int ifm_addrs;          ///< like rtm_addrs
	int ifm_flags;          ///< value of if_flags
	u_short ifm_index;      ///< index for associated ifp
	struct if_data ifm_data;///< statistics and other data about if
};

/**
 * Message format for use in obtaining information about interface addresses
 * from getkerninfo and the routing socket
 */
struct ifa_msghdr {
	u_short ifam_msglen;///< to skip over non-understood messages
	u_char ifam_version;///< future binary compatability
	u_char ifam_type;   ///< message type
	int ifam_addrs;     ///< like rtm_addrs
	int ifam_flags;     ///< value of ifa_flags
	u_short ifam_index; ///< index for associated ifp
	int ifam_metric;    ///< value of ifa_metric
};

/**
 * Interface request structure used for socket
 * ioctl's.  All interface ioctl's must have parameter
 * definitions which begin with ifr_name.  The
 * remainder may be interface specific.
 */
struct ifreq {
#define IFNAMSIZ 16
	char ifr_name[IFNAMSIZ]; /* if name, e.g. "en0" */
	union {
		struct sockaddr ifru_addr;
		struct sockaddr ifru_dstaddr;
		struct sockaddr ifru_broadaddr;
		short ifru_flags;
		int ifru_metric;
		caddr_t ifru_data;
	} ifr_ifru;
#define ifr_addr ifr_ifru.ifru_addr          ///< address
#define ifr_dstaddr ifr_ifru.ifru_dstaddr    ///< other end of p-to-p link
#define ifr_broadaddr ifr_ifru.ifru_broadaddr///< broadcast address
#define ifr_flags ifr_ifru.ifru_flags        ///< flags
#define ifr_metric ifr_ifru.ifru_metric      ///< metric
#define ifr_data ifr_ifru.ifru_data          ///< for use by interface
};

struct ifaliasreq {
	char ifra_name[IFNAMSIZ];///< if name, e.g. "en0"
	struct sockaddr ifra_addr;
	struct sockaddr ifra_broadaddr;
	struct sockaddr ifra_mask;
};

/**
 * Structure used in SIOCGIFCONF request.
 * Used to retrieve interface configuration
 * for machine (useful for programs which
 * must know all networks accessible).
 */
struct ifconf {
	int ifc_len;///< size of associated buffer
	union {
		caddr_t ifcu_buf;
		struct ifreq *ifcu_req;
	} ifc_ifcu;
#define ifc_buf ifc_ifcu.ifcu_buf///< buffer address
#define ifc_req ifc_ifcu.ifcu_req///< array of structures returned
};

#include <net/if_arp.h>

#ifdef KERNEL
#define IFAFREE(ifa)           \
	if((ifa)->ifa_refcnt <= 0) \
		ifafree(ifa);          \
	else                       \
		(ifa)->ifa_refcnt--;

struct ifnet *ifnet;

void ether_ifattach __P((struct ifnet *) );
void ether_input __P((struct ifnet *, struct ether_header *, struct mbuf *) );
int ether_output __P((struct ifnet *,
                      struct mbuf *, struct sockaddr *, struct rtentry *) );
char *ether_sprintf __P((u_char *) );

void if_attach __P((struct ifnet *) );
void if_down __P((struct ifnet *) );
void if_qflush __P((struct ifqueue *) );///< 丢弃队列ifq中的所有分组,例如,当一个接口被关闭了
void if_slowtimo __P((void *) );
void if_up __P((struct ifnet *) );
#ifdef vax
void ifubareset __P((int) );
#endif
int ifconf __P((int, caddr_t));
void ifinit __P((void) );
int ifioctl __P((struct socket *, u_long, caddr_t, struct proc *) );
int ifpromisc __P((struct ifnet *, int) );
struct ifnet *ifunit __P((char *) );

struct ifaddr *ifa_ifwithaddr __P((struct sockaddr *) );
struct ifaddr *ifa_ifwithaf __P((int) );
struct ifaddr *ifa_ifwithdstaddr __P((struct sockaddr *) );
struct ifaddr *ifa_ifwithnet __P((struct sockaddr *) );
struct ifaddr *ifa_ifwithroute __P((int, struct sockaddr *,
                                    struct sockaddr *) );
struct ifaddr *ifaof_ifpforaddr __P((struct sockaddr *, struct ifnet *) );
void ifafree __P((struct ifaddr *) );
void link_rtrequest __P((int, struct rtentry *, struct sockaddr *) );

int loioctl __P((struct ifnet *, u_long, caddr_t));
void loopattach __P((int) );
int looutput __P((struct ifnet *,
                  struct mbuf *, struct sockaddr *, struct rtentry *) );
void lortrequest __P((int, struct rtentry *, struct sockaddr *) );
#endif
