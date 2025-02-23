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
 *	@(#)protosw.h	8.1 (Berkeley) 6/2/93
 */

/**
 * Protocol switch table.
 *
 * Each protocol has a handle initializing one of these structures,
 * which is used for protocol-protocol and system-protocol communication.
 *
 * A protocol is called through the pr_init entry before any other.
 * Thereafter it is called every 200ms through the pr_fasttimo entry and
 * every 500ms through the pr_slowtimo for timer based actions.
 * The system will call the pr_drain entry if it is low on space and
 * this should throw away any non-critical data.
 *
 * Protocols pass data between themselves as chains of mbufs using
 * the pr_input and pr_output hooks.  Pr_input passes data up (towards
 * UNIX) and pr_output passes it down (towards the imps); control
 * information passes up and down on pr_ctlinput and pr_ctloutput.
 * The protocol is responsible for the space occupied by any the
 * arguments to these entries and must dispose it.
 *
 * The userreq routine interfaces protocols to the system and is
 * described below.
 */
struct protosw {
	short pr_type;           ///< 协议的通信语义 socket type used for
	struct domain *pr_domain;///< 指向相关的 domain结构 domain protocol a member of
	short pr_protocol;       ///< 域中协议的编号 protocol number
	short pr_flags;          ///< 标识协议的附加特征 see below
	                         /* protocol-protocol hooks */
	void (*pr_input)();      ///< 处理从一个低层协议输入的数据 input to protocol (from below)
	int (*pr_output)();      ///< 处理从一个高层协议输出的数据 output to protocol (from above)
	void (*pr_ctlinput)();   ///< 处理来自下层的控制信息 向与插口有关的协议报告差错 control input (from below)
	int (*pr_ctloutput)();   ///< 处理来自上层的控制信息 control output (from above)
	                         /* user-protocol hook */
	int (*pr_usrreq)();      ///< 处理来自进程的所有通信请求 user request: see list below
	                         /* utility hooks */
	void (*pr_init)();       ///< 处理初始化 initialization hook
	void (*pr_fasttimo)();   ///< IP不使用 fast timeout (200ms)
	void (*pr_slowtimo)();   ///< 用于IP重装算法的慢超时 slow timeout (500ms)
	void (*pr_drain)();      ///< 如果可能,释放内存 flush any excess space possible
	int (*pr_sysctl)();      ///< 修改系统范围参数 sysctl for protocol
};

#define PR_SLOWHZ 2///< 2 slow timeouts per second
#define PR_FASTHZ 5///< 5 fast timeouts per second

/**
 * Values for pr_flags.
 * PR_ADDR requires PR_ATOMIC;
 * PR_ADDR and PR_CONNREQUIRED are mutually exclusive.
 */
#define PR_ATOMIC 0x01      ///< 每个进程请求映射为一个单个的协议请求 exchange atomic messages only
#define PR_ADDR 0x02        ///< 协议在每个数据报中都传递地址 addresses given with messages
#define PR_CONNREQUIRED 0x04///< 协议是面向连接的 connection required by protocol
#define PR_WANTRCVD 0x08    ///< 当一个进程接收到数据时通知协议 want PRU_RCVD calls
#define PR_RIGHTS 0x10      ///< 协议支持访问权限 passes capabilities

/*
 * The arguments to usrreq are:
 *	(*protosw[].pr_usrreq)(up, req, m, nam, opt);
 * where up is a (struct socket *), req is one of these requests,
 * m is a optional mbuf chain containing a message,
 * nam is an optional mbuf chain containing an address,
 * and opt is a pointer to a socketopt structure or nil.
 * The protocol is responsible for disposal of the mbuf chain m,
 * the caller is responsible for any space held by nam and opt.
 * A non-zero return from usrreq gives an
 * UNIX error number which should be passed to higher level software.
 */
#define PRU_ATTACH 0    ///< 产生了一个新的插口 attach protocol to up
#define PRU_DETACH 1    ///< 插口被关闭 detach protocol from up
#define PRU_BIND 2      ///< 绑定地址到插口 bind socket to address
#define PRU_LISTEN 3    ///< 开始监听连接请求 listen for connection
#define PRU_CONNECT 4   ///< 同地址建立关联或连接 establish connection to peer
#define PRU_ACCEPT 5    ///< 等待并接受连接 accept connection from peer
#define PRU_DISCONNECT 6///< 切断插口和另一地址间的关联 disconnect from peer
#define PRU_SHUTDOWN 7  ///< 结束同另一地址的通信 won't send any more data
#define PRU_RCVD 8      ///< 进程已收到一些数据 have taken data; more room now
#define PRU_SEND 9      ///< 发送正常数据 send this data
#define PRU_ABORT 10    ///< 异常终止每一个存在的连接 abort (fast DISCONNECT, DETATCH)
#define PRU_CONTROL 11  ///< control operations on protocol
#define PRU_SENSE 12    ///< return status into m
#define PRU_RCVOOB 13   ///< 接收OOB数据 retrieve out of band data
#define PRU_SENDOOB 14  ///< 发送OOB数据 send out of band data
#define PRU_SOCKADDR 15 ///< 返回与插口相关联的本地地址 fetch socket's address
#define PRU_PEERADDR 16 ///< 返回与插口关联的对方地址 fetch peer's address
#define PRU_CONNECT2 17 ///< 将两个插口连在一起 connect two sockets
/* begin for protocols internal use */
#define PRU_FASTTIMO 18 ///< 200ms timeout
#define PRU_SLOWTIMO 19 ///< 500ms timeout
#define PRU_PROTORCV 20 ///< receive from below
#define PRU_PROTOSEND 21///< send to below

#define PRU_NREQ 21

#ifdef PRUREQUESTS
char *prurequests[] = {
        "ATTACH",
        "DETACH",
        "BIND",
        "LISTEN",
        "CONNECT",
        "ACCEPT",
        "DISCONNECT",
        "SHUTDOWN",
        "RCVD",
        "SEND",
        "ABORT",
        "CONTROL",
        "SENSE",
        "RCVOOB",
        "SENDOOB",
        "SOCKADDR",
        "PEERADDR",
        "CONNECT2",
        "FASTTIMO",
        "SLOWTIMO",
        "PROTORCV",
        "PROTOSEND",
};
#endif

/*
 * The arguments to the ctlinput routine are
 *	(*protosw[].pr_ctlinput)(cmd, sa, arg);
 * where cmd is one of the commands below, sa is a pointer to a sockaddr,
 * and arg is an optional caddr_t argument used within a protocol family.
 */
#define PRC_IFDOWN 0           ///< 网络接口关闭 interface transition
#define PRC_ROUTEDEAD 1        ///< 如果可能,选择新的路由 select new route if possible ???
#define PRC_QUENCH2 3          ///< 阻塞比特要求放慢 DEC congestion bit says slow down
#define PRC_QUENCH 4           ///< 要求放慢发送 some one said to slow down
#define PRC_MSGSIZE 5          ///< 无效报文大小 message size forced drop
#define PRC_HOSTDEAD 6         ///< 主机似乎已关闭 host appears to be down
#define PRC_HOSTUNREACH 7      ///< deprecated (use PRC_UNREACH_HOST)
#define PRC_UNREACH_NET 8      ///< 网络不可达 no route to network
#define PRC_UNREACH_HOST 9     ///< 主机不可达 no route to host
#define PRC_UNREACH_PROTOCOL 10///< 目的主机上协议不能用 dst says bad protocol
#define PRC_UNREACH_PORT 11    ///< 目的主机上端口没有被激活 bad port #
/* was	PRC_UNREACH_NEEDFRAG	12	   (use PRC_MSGSIZE)*/
#define PRC_UNREACH_SRCFAIL 13 ///< 源路由失败 source route failed
#define PRC_REDIRECT_NET 14    ///< 网络路由选择重定向 net routing redirect
#define PRC_REDIRECT_HOST 15   ///< 主机路由选择重定向 host routing redirect
#define PRC_REDIRECT_TOSNET 16 ///< TOS和网络的重定向 redirect for type of service & net
#define PRC_REDIRECT_TOSHOST 17///< TOS和主机的重定向 redirect for tos & host
#define PRC_TIMXCEED_INTRANS 18///< 传送过程中IP生存期到期 packet lifetime expired in transit
#define PRC_TIMXCEED_REASS 19  ///< 重装生存期到期 lifetime expired on reass q
#define PRC_PARAMPROB 20       ///< 首部不正确 header incorrect

#define PRC_NCMDS 21

#define PRC_IS_REDIRECT(cmd) \
	((cmd) >= PRC_REDIRECT_NET && (cmd) <= PRC_REDIRECT_TOSHOST)

#ifdef PRCREQUESTS
char *prcrequests[] = {
        "IFDOWN", "ROUTEDEAD", "#2", "DEC-BIT-QUENCH2",
        "QUENCH", "MSGSIZE", "HOSTDEAD", "#7",
        "NET-UNREACH", "HOST-UNREACH", "PROTO-UNREACH", "PORT-UNREACH",
        "#12", "SRCFAIL-UNREACH", "NET-REDIRECT", "HOST-REDIRECT",
        "TOSNET-REDIRECT", "TOSHOST-REDIRECT", "TX-INTRANS", "TX-REASS",
        "PARAMPROB"};
#endif

/*
 * The arguments to ctloutput are:
 *	(*protosw[].pr_ctloutput)(req, so, level, optname, optval);
 * req is one of the actions listed below, so is a (struct socket *),
 * level is an indication of which protocol layer the option is intended.
 * optname is a protocol dependent socket option request,
 * optval is a pointer to a mbuf-chain pointer, for value-return results.
 * The protocol is responsible for disposal of the mbuf chain *optval
 * if supplied,
 * the caller is responsible for any space held by *optval, when returned.
 * A non-zero return from usrreq gives an
 * UNIX error number which should be passed to higher level software.
 */
#define PRCO_GETOPT 0
#define PRCO_SETOPT 1

#define PRCO_NCMDS 2

#ifdef PRCOREQUESTS
char *prcorequests[] = {
        "GETOPT",
        "SETOPT",
};
#endif

#ifdef KERNEL
extern struct protosw *pffindproto(), *pffindtype();
#endif
