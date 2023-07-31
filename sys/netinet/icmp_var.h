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
 *	@(#)icmp_var.h	8.1 (Berkeley) 6/10/93
 */

/**
 * Variables related to this implementation
 * of the internet control message protocol.
 */
struct icmpstat {
	/* statistics related to icmp packets generated */
	u_long icps_error;                    ///< icmp_error的调用(重定向除外)数 # of calls to icmp_error
	u_long icps_oldshort;                 ///< 因为IP数据报太短而丢弃的差错数 no error 'cuz old ip too short
	u_long icps_oldicmp;                  ///< 因为数据报是一个ICMP报文而丢弃的差错数 no error 'cuz old was icmp
	u_long icps_outhist[ICMP_MAXTYPE + 1];///< 输出计数器数组;每种ICMP类型对应一个
	/* statistics related to input messages processed */
	u_long icps_badcode;                 ///< 由于无效码而丢弃的ICMP报文数 icmp_code out of range
	u_long icps_tooshort;                ///< 由于ICMP首部太短而丢弃的报文数 packet < ICMP_MINLEN
	u_long icps_checksum;                ///< 由于坏的ICMP检验和而丢弃的ICMP报文数 bad checksum
	u_long icps_badlen;                  ///< 由于无效的ICMP体而丢弃的ICMP报文数 calculated bound mismatch
	u_long icps_reflect;                 ///< 内核反映的ICMP报文数 number of responses
	u_long icps_inhist[ICMP_MAXTYPE + 1];///< 输入计数器数组;每种ICMP类型对应一个
};

/*
 * Names for ICMP sysctl objects
 */
#define ICMPCTL_MASKREPL 1///< allow replies to netmask requests
#define ICMPCTL_MAXID 2

#define ICMPCTL_NAMES                      \
	{                                      \
		{0, 0},                            \
		        {"maskrepl", CTLTYPE_INT}, \
	}

#ifdef KERNEL
struct icmpstat icmpstat;///< ICMP统计量
#endif
