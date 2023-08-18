/**
 * @file
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
 *	@(#)udp_var.h	8.1 (Berkeley) 6/10/93
 */

/**
 * UDP kernel structures and variables.
 */
struct udpiphdr {
	struct ipovly ui_i; /* overlaid ip structure */
	struct udphdr ui_u; /* udp header */
};
#define ui_next ui_i.ih_next
#define ui_prev ui_i.ih_prev
#define ui_x1 ui_i.ih_x1
#define ui_pr ui_i.ih_pr
#define ui_len ui_i.ih_len
#define ui_src ui_i.ih_src
#define ui_dst ui_i.ih_dst
#define ui_sport ui_u.uh_sport
#define ui_dport ui_u.uh_dport
#define ui_ulen ui_u.uh_ulen
#define ui_sum ui_u.uh_sum

struct udpstat {
	/* input statistics: */
	u_long udps_ipackets;     ///< 所有收到的数据报个数 total input packets
	u_long udps_hdrops;       ///< 收到分组小于首部的数据报个数 packet shorter than header
	u_long udps_badsum;       ///< 收到有检验和错误的数据报个数 checksum error
	u_long udps_badlen;       ///< 收到所有数据长度大于分组的数据报个数 data length larger than packet
	u_long udps_noport;       ///< 收到在目的端口没有进程的数据报个数 no socket on port
	u_long udps_noportbcast;  ///< 收到在目的端口没有进程的广播 of above, arrived as broadcast
	u_long udps_fullsock;     ///< 收到由于输入插口已满而没有提交的数据报个数 not delivered, input socket full
	u_long udpps_pcbcachemiss;///< 高速缓存的输入数据报个数 input packets missing pcb cache
	                          /* output statistics: */
	u_long udps_opackets;     ///< 全部输出数据报的个数 total output packets
};

/*
 * Names for UDP sysctl objects
 */
#define UDPCTL_CHECKSUM 1///< checksum UDP packets
#define UDPCTL_MAXID 2

#define UDPCTL_NAMES                       \
	{                                      \
		{0, 0},                            \
		        {"checksum", CTLTYPE_INT}, \
	}

#ifdef KERNEL
struct inpcb udb;      ///< UDP PCB 表的表头
struct udpstat udpstat;///< UDP统计

void udp_ctlinput __P((int, struct sockaddr *, struct ip *) );
void udp_init __P((void) );
void udp_input __P((struct mbuf *, int) );
int udp_output __P((struct inpcb *,
                    struct mbuf *, struct mbuf *, struct mbuf *) );
int udp_sysctl __P((int *, u_int, void *, size_t *, void *, size_t));
int udp_usrreq __P((struct socket *,
                    int, struct mbuf *, struct mbuf *, struct mbuf *) );
#endif
