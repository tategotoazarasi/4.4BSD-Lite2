/**
 * @file
 * @copyright
 * Copyright (c) 1982, 1986, 1993, 1994, 1995
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
 *	@(#)tcp_var.h	8.4 (Berkeley) 5/24/95
 */

/*
 * Kernel variables for tcp.
 */

/**
 * Tcp control block, one per tcp; fields:
 */
struct tcpcb {
	struct tcpiphdr *seg_next;///< sequencing queue
	struct tcpiphdr *seg_prev;
	short t_state;              ///< state of this connection
	short t_timer[TCPT_NTIMERS];///< tcp timers
	short t_rxtshift;           ///< tcp_backoff[数组索引(指数退避)] log(2) of rexmt exp. backoff
	short t_rxtcur;             ///< 当前重传时限 current retransmit value
	short t_dupacks;            ///< consecutive dup acks recd
	u_short t_maxseg;           ///< TCP将发送的最大报文段长度 maximum segment size
	char t_force;               ///< 1 if forcing out a byte
	u_short t_flags;
#define TF_ACKNOW 0x0001     ///< 立即发送ACK ack peer immediately
#define TF_DELACK 0x0002     ///< 延迟发送ACK ack, but try to delay it
#define TF_NODELAY 0x0004    ///< 立即发送用户数据,不等待形成最大报文段 (禁止Nagle算法) don't delay packets to coalesce
#define TF_NOOPT 0x0008      ///< 不使用TCP选项(永不填充TCP选项字段) don't use tcp options
#define TF_SENTFIN 0x0010    ///< FIN已发送 have sent FIN
#define TF_REQ_SCALE 0x0020  ///< 已经/将要在SYN报文中请求窗口变化选项 have/will request window scaling
#define TF_RCVD_SCALE 0x0040 ///< 对端在SYN报文中发送窗口变化选项时置位 other side has requested scaling
#define TF_REQ_TSTMP 0x0080  ///< 已经/将要在SYN中请求时间戳选项 have/will request timestamps
#define TF_RCVD_TSTMP 0x0100 ///< 对端在SYN报文中发送时间戳选项时置位 a timestamp was received in SYN
#define TF_SACK_PERMIT 0x0200///< other side said I could SACK

	struct tcpiphdr *t_template;///< skeletal packet for transmit */
	struct inpcb *t_inpcb;      ///< back pointer to internet pcb */
	                            /*
 * The following fields are used as in the protocol specification.
 * See RFC783, Dec. 1981, page 21.
 */
	                            /* send sequence variables */
	tcp_seq snd_una;            ///< send unacknowledged
	tcp_seq snd_nxt;            ///< send next
	tcp_seq snd_up;             ///< send urgent pointer
	tcp_seq snd_wl1;            ///< window update seg seq number
	tcp_seq snd_wl2;            ///< window update seg ack number
	tcp_seq iss;                ///< initial send sequence number
	u_long snd_wnd;             ///< send window */
	                            /* receive sequence variables */
	u_long rcv_wnd;             ///< receive window
	tcp_seq rcv_nxt;            ///< receive next
	tcp_seq rcv_up;             ///< receive urgent pointer
	tcp_seq irs;                ///< initial receive sequence number
	                            /*
 * Additional variables for this implementation.
 */
	                            /* receive variables */
	tcp_seq rcv_adv;            ///< advertised window
	                            /* retransmit variables */
	tcp_seq snd_max;            ///< highest sequence number sent; used to recognize retransmits
	                            /* congestion control (for slow start, source quench, retransmit after loss) */
	u_long snd_cwnd;            ///< congestion-controlled window
	u_long snd_ssthresh;        ///< snd_cwnd size threshhold for for slow start exponential to linear switch
	                            /*
 * transmit timing stuff.  See below for scale of srtt and rttvar.
 * "Variance" is actually smoothed difference.
 */
	u_short t_idle;             ///< inactivity time
	short t_rtt;                ///< 已平滑的RTT估计器 round trip time
	tcp_seq t_rtseq;            ///< sequence number being timed
	short t_srtt;               ///< smoothed round-trip time
	short t_rttvar;             ///< 已平滑的RTT平均偏差估计器 variance in round-trip time
	u_short t_rttmin;           ///< 重传时限最小值 minimum rtt allowed
	u_long max_sndwnd;          ///< largest window peer has offered

	/* out-of-band data */
	char t_oobflags;///< have some
	char t_iobc;    ///< input character
#define TCPOOB_HAVEDATA 0x01
#define TCPOOB_HADDATA 0x02
	short t_softerror;///< possible error not yet reported

	/* RFC 1323 variables */
	u_char snd_scale;      ///< window scaling for send window
	u_char rcv_scale;      ///< window scaling for recv window
	u_char request_r_scale;///< pending window scaling
	u_char requested_s_scale;
	u_long ts_recent;    ///< timestamp echo data
	u_long ts_recent_age;///< when last updated
	tcp_seq last_ack_sent;

	/* TUBA stuff */
	caddr_t t_tuba_pcb;///< next level down pcb for TCP over z
};

#define intotcpcb(ip) ((struct tcpcb *) (ip)->inp_ppcb)
#define sototcpcb(so) (intotcpcb(sotoinpcb(so)))

/*
 * The smoothed round-trip time and estimated variance
 * are stored as fixed point numbers scaled by the values below.
 * For convenience, these scales are also used in smoothing the average
 * (smoothed = (1/scale)sample + ((scale-1)/scale)smoothed).
 * With these scales, srtt has 3 bits to the right of the binary point,
 * and thus an "ALPHA" of 0.875.  rttvar has 2 bits to the right of the
 * binary point, and is smoothed with an ALPHA of 0.75.
 */
#define TCP_RTT_SCALE 8   ///< 相乘: t_srtt = srtt × 8 multiplier for srtt; 3 bits frac.
#define TCP_RTT_SHIFT 3   ///< 移位: t_srtt = srtt << 3 shift for srtt; 3 bits frac.
#define TCP_RTTVAR_SCALE 4///< 相乘: t_rttvar = rttvar × 4 multiplier for rttvar; 2 bits
#define TCP_RTTVAR_SHIFT 2///< 移位: t_rttvar = rttvar << 2 multiplier for rttvar; 2 bits

/**
 * The initial retransmission should happen at rtt + 4 * rttvar.
 * Because of the way we do the smoothing, srtt and rttvar
 * will each average +1/2 tick of bias.  When we compute
 * the retransmit timer, we want 1/2 tick of rounding and
 * 1 extra tick because of +-1/2 tick uncertainty in the
 * firing of the timer.  The bias will give us exactly the
 * 1.5 tick we need.  But, because the bias is
 * statistical, we have to test that we don't drop below
 * the minimum feasible timer (which is 2 ticks).
 * This macro assumes that the value of TCP_RTTVAR_SCALE
 * is the same as the multiplier for rttvar.
 */
#define TCP_REXMTVAL(tp) \
	(((tp)->t_srtt >> TCP_RTT_SHIFT) + (tp)->t_rttvar)

/**
 * We want to avoid doing m_pullup on incoming packets but that
 * means avoiding dtom on the tcp reassembly code.  That in turn means
 * keeping an mbuf pointer in the reassembly queue (since we might
 * have a cluster).  As a quick hack, the source & destination
 * port numbers (which are no longer needed once we've located the
 * tcpcb) are overlayed with an mbuf pointer.
 */
#define REASS_MBUF(ti) (*(struct mbuf **) &((ti)->ti_t))

/**
 * TCP statistics.
 * Many of these should be kept per connection,
 * but that's inconvenient at the moment.
 */
struct tcpstat {
	u_long tcps_connattempt; ///< 试图建立连接的次数 (调用connect) connections initiated
	u_long tcps_accepts;     ///< 被动打开的连接数 connections accepted
	u_long tcps_connects;    ///< 主动打开的连接次数 (调用connect成功) connections established
	u_long tcps_drops;       ///< 意外丢失的连接数 (收到SYN之后) connections dropped
	u_long tcps_conndrops;   ///< 在连接建立阶段失败的连接次数 (SYN收到之前) embryonic connections dropped
	u_long tcps_closed;      ///< 关闭的连接数 (包括意外丢失的连接) conn. closed (includes drops)
	u_long tcps_segstimed;   ///< segs where we tried to get rtt
	u_long tcps_rttupdated;  ///< RTT估算值更新次数 times we succeeded
	u_long tcps_delack;      ///< 延迟发送的ACK数 delayed acks sent
	u_long tcps_timeoutdrop; ///< 由于重传超时而丢失的连接数 conn. dropped in rxmt timeout
	u_long tcps_rexmttimeo;  ///< 重传超时次数 retransmit timeouts
	u_long tcps_persisttimeo;///< 持续定时器超时次数 persist timeouts
	u_long tcps_keeptimeo;   ///< 保活定时器或连接建立定时器超时次数 keepalive timeouts
	u_long tcps_keepprobe;   ///< 保活探测指针发送次数 keepalive probes sent
	u_long tcps_keepdrops;   ///< 在保活阶段丢失的连接数 (己建立或正等待SYN) connections dropped in keepalive

	u_long tcps_sndtotal;     ///< 发送的报文总数 total packets sent
	u_long tcps_sndpack;      ///< 发送的数据报文数 (数据长度> 0) data packets sent
	u_long tcps_sndbyte;      ///< 发送的字节数 data bytes sent
	u_long tcps_sndrexmitpack;///< 重传的报文数 data packets retransmitted
	u_long tcps_sndrexmitbyte;///< 重传的数据字节数 data bytes retransmitted
	u_long tcps_sndacks;      ///< 发送的纯ACK报文数(数据长度=0) ack-only packets sent
	u_long tcps_sndprobe;     ///< 发送的窗口探测次数 (等待定时器强行加入 1字节数据) window probes sent
	u_long tcps_sndurg;       ///< 只携带URG标志的报文数 (数据长度= 0) packets sent with URG only
	u_long tcps_sndwinup;     ///< 只携带窗口更新信息的报文数 (数据长度=0) window update-only packets sent
	u_long tcps_sndctrl;      ///< 发送的控制 (SYN、FIN、RST)报文数(数据长度=0) control (SYN|FIN|RST) packets sent

	u_long tcps_rcvtotal;       ///< 收到的报文总数 total packets received
	u_long tcps_rcvpack;        ///< 顺序接收的报文数 packets received in sequence
	u_long tcps_rcvbyte;        ///< 连续收到的字节数 bytes received in sequence
	u_long tcps_rcvbadsum;      ///< 收到的检验和错误的报文数 packets received with ccksum errs
	u_long tcps_rcvbadoff;      ///< 收到的首部长度无效的报文数 packets received with bad offset
	u_long tcps_rcvshort;       ///< 长度过短的报文数 packets received too short
	u_long tcps_rcvduppack;     ///< 内容完全一致的报文数 duplicate-only packets received
	u_long tcps_rcvdupbyte;     ///< 在完全重复报文中收到的字节数 duplicate-only bytes received
	u_long tcps_rcvpartduppack; ///< 部分数据重复的报文数 packets with some duplicate data
	u_long tcps_rcvpartdupbyte; ///< 部分内容重复的报文中的重复字节数 dup. bytes in part-dup. packets
	u_long tcps_rcvoopack;      ///< 收到失序的报文数 out-of-order packets received
	u_long tcps_rcvoobyte;      ///< 收到失序的字节数 out-of-order bytes received
	u_long tcps_rcvpackafterwin;///< 携带数据超出滑动窗口通告值的报文数 packets with data after window
	u_long tcps_rcvbyteafterwin;///< 在滑动窗口己满时收到的字节数 bytes rcvd after window
	u_long tcps_rcvafterclose;  ///< 连接关闭后收到的报文数 packets rcvd after "close"
	u_long tcps_rcvwinprobe;    ///< 收到的窗口探测报文数 rcvd window probe packets
	u_long tcps_rcvdupack;      ///< 收到的重复ACK报文的次数 rcvd duplicate acks
	u_long tcps_rcvacktoomuch;  ///< 收到的对未发送数据进行确认的ACK报文数 rcvd acks for unsent data
	u_long tcps_rcvackpack;     ///< 收到的ACK报文数 rcvd ack packets
	u_long tcps_rcvackbyte;     ///< 由收到的ACK报文确认的发送字节数 bytes acked by rcvd acks
	u_long tcps_rcvwinupd;      ///< 收到的窗口更新报文数 rcvd window update packets
	u_long tcps_pawsdrop;       ///< 由于PSWS而丢失的报文段数 segments dropped due to PAWS
	u_long tcps_predack;        ///< 对ACK报文首部预测的正确次数 times hdr predict ok for acks
	u_long tcps_preddat;        ///< 对数据报文首部预测的正确次数 times hdr predict ok for data pkts
	u_long tcps_pcbcachemiss;   ///< PCB高速缓存匹配失败次数
	u_long tcps_persistdrop;    ///< timeout in persist state
	u_long tcps_badsyn;         ///< bogus SYN, e.g. premature ACK
};

#ifdef KERNEL
struct inpcb tcb;      ///< TCP Internet PC 表的表头 head of queue of active tcpcb's
struct tcpstat tcpstat;///< TCP统计数据 tcp statistics
u_long tcp_now;        ///< 用于RFC 1323时间戳实现的 500 ms计数器 for RFC 1323 timestamps

int tcp_attach __P((struct socket *) );
void tcp_canceltimers __P((struct tcpcb *) );
struct tcpcb *
        tcp_close __P((struct tcpcb *) );
void tcp_ctlinput __P((int, struct sockaddr *, struct ip *) );
int tcp_ctloutput __P((int, struct socket *, int, int, struct mbuf **) );
struct tcpcb *
        tcp_disconnect __P((struct tcpcb *) );
struct tcpcb *
        tcp_drop __P((struct tcpcb *, int) );
void tcp_dooptions __P((struct tcpcb *,
                        u_char *, int, struct tcpiphdr *, int *, u_long *, u_long *) );
void tcp_drain __P((void) );
void tcp_fasttimo __P((void) );
void tcp_init __P((void) );
void tcp_input __P((struct mbuf *, int) );
int tcp_mss __P((struct tcpcb *, u_int));
struct tcpcb *
        tcp_newtcpcb __P((struct inpcb *) );
void tcp_notify __P((struct inpcb *, int) );
int tcp_output __P((struct tcpcb *) );
void tcp_pulloutofband __P((struct socket *,
                            struct tcpiphdr *, struct mbuf *) );
void tcp_quench __P((struct inpcb *, int) );
int tcp_reass __P((struct tcpcb *, struct tcpiphdr *, struct mbuf *) );
void tcp_respond __P((struct tcpcb *,
                      struct tcpiphdr *, struct mbuf *, u_long, u_long, int) );
void tcp_setpersist __P((struct tcpcb *) );
void tcp_slowtimo __P((void) );
struct tcpiphdr *
        tcp_template __P((struct tcpcb *) );
struct tcpcb *
        tcp_timers __P((struct tcpcb *, int) );
void tcp_trace __P((int, int, struct tcpcb *, struct tcpiphdr *, int) );
struct tcpcb *
        tcp_usrclosed __P((struct tcpcb *) );
int tcp_usrreq __P((struct socket *,
                    int, struct mbuf *, struct mbuf *, struct mbuf *) );
void tcp_xmit_timer __P((struct tcpcb *, int) );
#endif
