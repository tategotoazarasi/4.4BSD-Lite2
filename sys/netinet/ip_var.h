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
 *	@(#)ip_var.h	8.2 (Berkeley) 1/9/95
 */

/**
 * Overlay for ip header used by other protocols (tcp, udp).
 */
struct ipovly {
	caddr_t ih_next, ih_prev;///< for protocol sequence q's
	u_char ih_x1;            ///< (unused)
	u_char ih_pr;            ///< protocol
	short ih_len;            ///< protocol length
	struct in_addr ih_src;   ///< source internet address
	struct in_addr ih_dst;   ///< destination internet address
};

/**
 * Ip reassembly queue structure.  Each fragment
 * being reassembled is attached to one of these structures.
 * They are timed out after ipq_ttl drops to 0, and may also
 * be reclaimed if memory becomes tight.
 */
struct ipq {
	struct ipq *next, *prev;///< to other reass headers
	u_char ipq_ttl;         ///< time for reass q to live
	u_char ipq_p;           ///< protocol of this fragment
	u_short ipq_id;         ///< sequence id for reassembly
	struct ipasfrag *ipq_next, *ipq_prev;
	///< to ip headers of fragments
	struct in_addr ipq_src, ipq_dst;
};

/**
 * Ip header, when holding a fragment.
 *
 * Note: ipf_next must be at same offset as ipq_next above
 */
struct ipasfrag {
#if BYTE_ORDER == LITTLE_ENDIAN
	u_char ip_hl : 4,
	        ip_v : 4;
#endif
#if BYTE_ORDER == BIG_ENDIAN
	u_char ip_v : 4,
	        ip_hl : 4;
#endif
	u_char ipf_mff;///< XXX overlays ip_tos: use low bit to avoid destroying tos; copied from (ip_off&IP_MF)
	short ip_len;
	u_short ip_id;
	short ip_off;
	u_char ip_ttl;
	u_char ip_p;
	u_short ip_sum;
	struct ipasfrag *ipf_next;///< next fragment
	struct ipasfrag *ipf_prev;///< previous fragment
};

/**
 * Structure stored in mbuf in inpcb.ip_options
 * and passed to ip_output when ip options are in use.
 * The actual length of the options (including ipopt_dst)
 * is in m_len.
 */
#define MAX_IPOPTLEN 40

struct ipoption {
	struct in_addr ipopt_dst;     ///< first-hop dst if source routed */
	char ipopt_list[MAX_IPOPTLEN];///< options proper */
};

/**
 * Structure attached to inpcb.ip_moptions and
 * passed to ip_output when IP multicast options are in use.
 */
struct ip_moptions {
	struct ifnet *imo_multicast_ifp;///< ifp for outgoing multicasts
	u_char imo_multicast_ttl;       ///< TTL for outgoing multicasts
	u_char imo_multicast_loop;      ///< 1 => hear sends if a member
	u_short imo_num_memberships;    ///< no. memberships this socket
	struct in_multi *imo_membership[IP_MAX_MEMBERSHIPS];
};

struct ipstat {
	u_long ips_total;       ///< 全部接收到的分组数 total packets received
	u_long ips_badsum;      ///< 检验和差错的分组数 checksum bad
	u_long ips_tooshort;    ///< 具有无效数据长度的分组数 packet too short
	u_long ips_toosmall;    ///< 无法包含IP分组的太小的分组数 not enough data
	u_long ips_badhlen;     ///< IP首部长度无效的分组数 ip header length < data size
	u_long ips_badlen;      ///< IP首部和IP数据长度不一致的分组数 ip length < ip header length
	u_long ips_fragments;   ///< 收到分片数 fragments received
	u_long ips_fragdropped; ///< 分片丢失数 (副本或空间不足) frags dropped (dups, out of space)
	u_long ips_fragtimeout; ///< 超时的分片数 fragments timed out
	u_long ips_forward;     ///< 转发的分组数 packets forwarded
	u_long ips_cantforward; ///< 目的站不可到达的分组数 packets rcvd for unreachable dest
	u_long ips_redirectsent;///< 已发送的重定向报文数 packets forwarded on same net
	u_long ips_noproto;     ///< 具有未知或不支持的协议的分组数 unknown or unsupported protocol
	u_long ips_delivered;   ///< 向高层交付的数据报数 datagrams delivered to upper level*/
	u_long ips_localout;    ///< 系统生成的数据报数 (即没有转发的) total ip packets generated here
	u_long ips_odropped;    ///< 由于资源不足丢掉的分组数 lost packets due to nobufs, etc.
	u_long ips_reassembled; ///< 重装的数据报数 total packets reassembled ok
	u_long ips_fragmented;  ///< 成功分片的数据报数 datagrams sucessfully fragmented
	u_long ips_ofragments;  ///< 为输出产生的分片数 output fragments created
	u_long ips_cantfrag;    ///< 由于不分片比特而丢弃的分组数 don't fragment flag was set, etc.
	u_long ips_badoptions;  ///< 在选项处理中发现差错的分组数 error in option processing
	u_long ips_noroute;     ///< 丢弃的分组数 —到目的地没有路由 packets discarded due to no route
	u_long ips_badvers;     ///< IP版本不是4的分组数 ip version != 4
	u_long ips_rawout;      ///< 全部生成的原始ip分组数 total raw ip packets generated
};

#ifdef KERNEL
/* flags passed to ip_output as last parameter */
#define IP_FORWARDING 0x1             ///< most of ip header exists
#define IP_RAWOUTPUT 0x2              ///< raw ip header exists
#define IP_ROUTETOIF SO_DONTROUTE     ///< bypass routing tables
#define IP_ALLOWBROADCAST SO_BROADCAST///< can send broadcast packets

struct ipstat ipstat;///< IP统计
struct ipq ipq;      ///< ip reass. queue
u_short ip_id;       ///< ip packet ctr, for ids
int ip_defttl;       ///< 数据报的默认 TTL (64跳) default IP ttl

int in_control __P((struct socket *, u_long, caddr_t, struct ifnet *) );
int ip_ctloutput __P((int, struct socket *, int, int, struct mbuf **) );
void ip_deq __P((struct ipasfrag *) );
int ip_dooptions __P((struct mbuf *) );
void ip_drain __P((void) );
void ip_enq __P((struct ipasfrag *, struct ipasfrag *) );
void ip_forward __P((struct mbuf *, int) );
void ip_freef __P((struct ipq *) );
void ip_freemoptions __P((struct ip_moptions *) );
int ip_getmoptions __P((int, struct ip_moptions *, struct mbuf **) );
void ip_init __P((void) );
int ip_mforward __P((struct mbuf *, struct ifnet *) );
int ip_optcopy __P((struct ip *, struct ip *) );
int ip_output __P((struct mbuf *,
                   struct mbuf *, struct route *, int, struct ip_moptions *) );
int ip_pcbopts __P((struct mbuf **, struct mbuf *) );
struct ip *
        ip_reass __P((struct ipasfrag *, struct ipq *) );
struct in_ifaddr *
        ip_rtaddr __P((struct in_addr));///< 查询路由缓存,必要时查询完整的路由表,来找到到给定IP地址的路由。
int ip_setmoptions __P((int, struct ip_moptions **, struct mbuf *) );
void ip_slowtimo __P((void) );
struct mbuf *
        ip_srcroute __P((void) );
void ip_stripoptions __P((struct mbuf *, struct mbuf *) );
int ip_sysctl __P((int *, u_int, void *, size_t *, void *, size_t));
void ipintr __P((void) );
int rip_ctloutput __P((int, struct socket *, int, int, struct mbuf **) );
void rip_init __P((void) );
void rip_input __P((struct mbuf *) );
int rip_output __P((struct mbuf *, struct socket *, u_long));
int rip_usrreq __P((struct socket *,
                    int, struct mbuf *, struct mbuf *, struct mbuf *) );
#endif
