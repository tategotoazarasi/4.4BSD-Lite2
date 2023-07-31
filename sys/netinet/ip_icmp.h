/**
 * @file
 * @copyright
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)ip_icmp.h	8.1 (Berkeley) 6/10/93
 */

/*
 * Interface Control Message Protocol Definitions.
 * Per RFC 792, September 1981.
 */

/**
 * Structure of an icmp header.
 */
struct icmp {
	u_char icmp_type;  ///< type of message, see below
	u_char icmp_code;  ///< type sub code
	u_short icmp_cksum;///< ones complement cksum of struct
	union {
		u_char ih_pptr;          ///< ICMP_PARAMPROB
		struct in_addr ih_gwaddr;///< ICMP_REDIRECT
		struct ih_idseq {
			n_short icd_id;
			n_short icd_seq;
		} ih_idseq;
		int ih_void;

		/* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */
		struct ih_pmtu {
			n_short ipm_void;
			n_short ipm_nextmtu;
		} ih_pmtu;
	} icmp_hun;
#define icmp_pptr icmp_hun.ih_pptr
#define icmp_gwaddr icmp_hun.ih_gwaddr
#define icmp_id icmp_hun.ih_idseq.icd_id
#define icmp_seq icmp_hun.ih_idseq.icd_seq
#define icmp_void icmp_hun.ih_void
#define icmp_pmvoid icmp_hun.ih_pmtu.ipm_void
#define icmp_nextmtu icmp_hun.ih_pmtu.ipm_nextmtu
	union {
		struct id_ts {
			n_time its_otime;
			n_time its_rtime;
			n_time its_ttime;
		} id_ts;
		struct id_ip {
			struct ip idi_ip;
			/* options and then 64 bits of data */
		} id_ip;
		u_long id_mask;
		char id_data[1];
	} icmp_dun;
#define icmp_otime icmp_dun.id_ts.its_otime
#define icmp_rtime icmp_dun.id_ts.its_rtime
#define icmp_ttime icmp_dun.id_ts.its_ttime
#define icmp_ip icmp_dun.id_ip.idi_ip
#define icmp_mask icmp_dun.id_mask
#define icmp_data icmp_dun.id_data
};

/*
 * Lower bounds on packet lengths for various types.
 * For the error advice packets must first insure that the
 * packet is large enought to contain the returned ip header.
 * Only then can we do the check to see if 64 bits of packet
 * data have been returned, since we need to check the returned
 * ip header length.
 */
#define ICMP_MINLEN 8                                     ///< ICMP报文大小的最小值 abs minimum
#define ICMP_TSLEN (8 + 3 * sizeof(n_time))               ///< ICMP时间戳报文大小 timestamp
#define ICMP_MASKLEN 12                                   ///< ICMP地址掩码报文大小 address mask
#define ICMP_ADVLENMIN (8 + sizeof(struct ip) + 8)        ///< ICMP差错(建议)报文大小的最小值 (IP + ICMP + BADIP = 20 + 8 + 8 = 36) min
#define ICMP_ADVLEN(p) (8 + ((p)->icmp_ip.ip_hl << 2) + 8)///< ICMP差错报文的大小,包含无效分组p的IP选项的optsize字节
/* N.B.: must separately check that ip_hl >= 5

/*
 * Definition of type and code field values.
 */
#define ICMP_ECHOREPLY 0           ///< 回显回答 echo reply
#define ICMP_UNREACH 3             ///< 目的主机不可达 dest unreachable, codes:
#define ICMP_UNREACH_NET 0         ///< 网络不可达 bad net
#define ICMP_UNREACH_HOST 1        ///< 主机不可达 bad host
#define ICMP_UNREACH_PROTOCOL 2    ///< 目的主机上协议不能用 bad protocol
#define ICMP_UNREACH_PORT 3        ///< 目的主机上端口没有被激活 bad port
#define ICMP_UNREACH_NEEDFRAG 4    ///< 需要分片并设置DF比特 IP_DF caused drop
#define ICMP_UNREACH_SRCFAIL 5     ///< 源路由失败 src route failed
#define ICMP_UNREACH_NET_UNKNOWN 6 ///< 目的网络未知 unknown net
#define ICMP_UNREACH_HOST_UNKNOWN 7///< 目的主机未知 unknown host
#define ICMP_UNREACH_ISOLATED 8    ///< 源主机被隔离 src host isolated
#define ICMP_UNREACH_NET_PROHIB 9  ///< 从管理上禁止与目的网络通信 prohibited access
#define ICMP_UNREACH_HOST_PROHIB 10///< 从管理上禁止与目的主机通信 ditto
#define ICMP_UNREACH_TOSNET 11     ///< 对服务类型,网络不可达 bad tos for net
#define ICMP_UNREACH_TOSHOST 12    ///< 对服务类型,主机不可达 bad tos for host
#define ICMP_SOURCEQUENCH 4        ///< 要求放慢发送 packet lost, slow down
#define ICMP_REDIRECT 5            ///< 有更好的路由 shorter route, codes:
#define ICMP_REDIRECT_NET 0        ///< 网络有更好的路由 for network
#define ICMP_REDIRECT_HOST 1       ///< 主机有更好的路由 for host
#define ICMP_REDIRECT_TOSNET 2     ///< TOS和网络有更好的路由 for tos and net
#define ICMP_REDIRECT_TOSHOST 3    ///< TOS和主机有更好的路由 for tos and host
#define ICMP_ECHO 8                ///< 回显请求 echo service
#define ICMP_ROUTERADVERT 9        ///< 路由器通告 router advertisement
#define ICMP_ROUTERSOLICIT 10      ///< 路由器请求 router solicitation
#define ICMP_TIMXCEED 11           ///< 超时 time exceeded, code:
#define ICMP_TIMXCEED_INTRANS 0    ///< 传送过程中IP生存期到期 ttl==0 in transit
#define ICMP_TIMXCEED_REASS 1      ///< 重装生存期到期 ttl==0 in reass
#define ICMP_PARAMPROB 12          ///< IP首部的问题 ip header bad
#define ICMP_PARAMPROB_OPTABSENT 1 ///< 丢失需要的选项 req. opt. absent
#define ICMP_TSTAMP 13             ///< 时间戳请求 timestamp request
#define ICMP_TSTAMPREPLY 14        ///< 时间戳回答 timestamp reply
#define ICMP_IREQ 15               ///< 信息请求（过时的） information request
#define ICMP_IREQREPLY 16          ///< 信息回答（过时的） information reply
#define ICMP_MASKREQ 17            ///< 地址掩码请求 address mask request
#define ICMP_MASKREPLY 18          ///< 地址掩码回答 address mask reply

#define ICMP_MAXTYPE 18

#define ICMP_INFOTYPE(type)                                         \
	((type) == ICMP_ECHOREPLY || (type) == ICMP_ECHO ||             \
	 (type) == ICMP_ROUTERADVERT || (type) == ICMP_ROUTERSOLICIT || \
	 (type) == ICMP_TSTAMP || (type) == ICMP_TSTAMPREPLY ||         \
	 (type) == ICMP_IREQ || (type) == ICMP_IREQREPLY ||             \
	 (type) == ICMP_MASKREQ || (type) == ICMP_MASKREPLY)

#ifdef KERNEL
void icmp_error __P((struct mbuf *, int, int, n_long, struct ifnet *) );///< 把作为链路级广播或多播发送的数据报引起的差错报文丢弃。应该丢弃 (但没有)发往 IP广播或多播地址的数据报引起的报文
void icmp_input __P((struct mbuf *, int) );                             ///< 在地址掩码请求中,用接收接口的广播或目的地址代替全0地址
void icmp_reflect __P((struct mbuf *) );                                ///< 为ICMP生成回答
void icmp_send __P((struct mbuf *, struct mbuf *) );                    ///< 处理所有输出的ICMP报文,并在把它们传给IP层之前计算ICMP检验和
int icmp_sysctl __P((int *, u_int, void *, size_t *, void *, size_t));
#endif
