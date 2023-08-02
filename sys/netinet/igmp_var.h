/**
 * @file
 * @copyright
 * Copyright (c) 1988 Stephen Deering.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Stephen Deering of Stanford University.
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
 *	@(#)igmp_var.h	8.1 (Berkeley) 7/19/93
 */

/*
 * Internet Group Management Protocol (IGMP),
 * implementation-specific definitions.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * MULTICAST 1.1
 */

struct igmpstat {
	u_long igps_rcv_total;     ///< 接收的全部报文数 total IGMP messages received
	u_long igps_rcv_tooshort;  ///< 字节数太少的报文数 received with too few bytes
	u_long igps_rcv_badsum;    ///< 接收的报文检验和错误数 received with bad checksum
	u_long igps_rcv_queries;   ///< 作为成员关系查询接收的报文数 received membership queries
	u_long igps_rcv_badqueries;///< 作为无效查询接收的报文数 received invalid queries
	u_long igps_rcv_reports;   ///< 作为成员关系报告接收的报文数 received membership reports
	u_long igps_rcv_badreports;///< 作为无效报告接收的报文数 received invalid reports
	u_long igps_rcv_ourreports;///< 作为逻辑组的报告接收的报文数 received reports for our groups
	u_long igps_snd_reports;   ///< 作为成员关系报告发送的报文数 sent membership reports
};

#ifdef KERNEL
struct igmpstat igmpstat;///< IGMP统计

/**
 * Macro to compute a random timer value between 1 and (IGMP_MAX_REPORTING_
 * DELAY * countdown frequency).  We generate a "random" number by adding
 * the total number of IP packets received, our primary IP address, and the
 * multicast address being timed-out.  The 4.3 random() routine really
 * ought to be available in the kernel!
 */
#define IGMP_RANDOM_DELAY(multiaddr)                    \
	/* struct in_addr multiaddr; */                     \
	((ipstat.ips_total +                                \
	  ntohl(IA_SIN(in_ifaddr)->sin_addr.s_addr) +       \
	  ntohl((multiaddr).s_addr)) %                      \
	         (IGMP_MAX_HOST_REPORT_DELAY * PR_FASTHZ) + \
	 1)

void igmp_init __P(());
void igmp_input __P((struct mbuf *, int) );
void igmp_joingroup __P((struct in_multi *) );
void igmp_leavegroup __P((struct in_multi *) );
void igmp_fasttimo __P(());
#endif
