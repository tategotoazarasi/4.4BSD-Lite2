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
 *	@(#)tcp_fsm.h	8.1 (Berkeley) 6/10/93
 */

/*
 * TCP FSM state definitions.
 * Per RFC793, September, 1981.
 */

#define TCP_NSTATES 11

#define TCPS_CLOSED 0      ///< 关闭 closed
#define TCPS_LISTEN 1      ///< 监听连接请求 (被动打开) listening for connection
#define TCPS_SYN_SENT 2    ///< 已发送SYN (主动打开) active, have sent syn
#define TCPS_SYN_RECEIVED 3///< 已发送并接收 SYN;等待ACK have send and received syn
/* states < TCPS_ESTABLISHED are those where connections not established */
#define TCPS_ESTABLISHED 4///< 连接建立 (数据传输) established
#define TCPS_CLOSE_WAIT 5 ///< 已收到FIN,等待应用程序关闭 rcvd fin, waiting for close
/* states > TCPS_CLOSE_WAIT are those where user has closed */
#define TCPS_FIN_WAIT_1 6///< 已关闭,发送 FIN;等待ACK和FIN have closed, sent fin
#define TCPS_CLOSING 7   ///< 同时关闭;等待 ACK closed xchd FIN; await FIN ACK
#define TCPS_LAST_ACK 8  ///< 收到的FIN已关闭;等待 ACK had fin and close; await FIN ACK
/* states > TCPS_CLOSE_WAIT && < TCPS_FIN_WAIT_2 await ACK of FIN */
#define TCPS_FIN_WAIT_2 9///< 已关闭,等待 FIN have closed, fin is acked
#define TCPS_TIME_WAIT 10///< 主动关闭后 2MSL等待状态 in 2*msl quiet wait after close

#define TCPS_HAVERCVDSYN(s) ((s) >= TCPS_SYN_RECEIVED)
#define TCPS_HAVERCVDFIN(s) ((s) >= TCPS_TIME_WAIT)

#ifdef TCPOUTFLAGS
/*
 * Flags used when sending segments in tcp_output.
 * Basic flags (TH_RST,TH_ACK,TH_SYN,TH_FIN) are totally
 * determined by state, with the proviso that TH_FIN is sent only
 * if all data queued for output is included in the segment.
 */
u_char tcp_outflags[TCP_NSTATES] = {
        TH_RST | TH_ACK,
        0,
        TH_SYN,
        TH_SYN | TH_ACK,
        TH_ACK,
        TH_ACK,
        TH_FIN | TH_ACK,
        TH_FIN | TH_ACK,
        TH_FIN | TH_ACK,
        TH_ACK,
        TH_ACK,
};
#endif

#ifdef KPROF
int tcp_acounts[TCP_NSTATES][PRU_NREQ];
#endif

#ifdef TCPSTATES
char *tcpstates[] = {
        "CLOSED",
        "LISTEN",
        "SYN_SENT",
        "SYN_RCVD",
        "ESTABLISHED",
        "CLOSE_WAIT",
        "FIN_WAIT_1",
        "CLOSING",
        "LAST_ACK",
        "FIN_WAIT_2",
        "TIME_WAIT",
};
#endif
