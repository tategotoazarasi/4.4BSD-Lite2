/**
 * @file
 * @copyright
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)socketvar.h	8.3 (Berkeley) 2/19/95
 */

#include <sys/select.h> /* for struct selinfo */

/**
 * Kernel structure per socket.
 * Contains send and receive buffer queues,
 * handle on protocol and pointer to protocol
 * private data and error information.
 */
struct socket {
	short so_type;           ///< generic type, see socket.h
	short so_options;        ///< from socket call, see socket.h
	short so_linger;         ///< time to linger while closing
	short so_state;          ///< internal state flags SS_*, below
	caddr_t so_pcb;          ///< protocol control block
	struct protosw *so_proto;///< protocol handle
	                         /**
 * Variables for connection queueing.
 * Socket where accepts occur is so_head in all subsidiary sockets.
 * If so_head is 0, socket is not related to an accept.
 * For head socket so_q0 queues partially completed connections,
 * while so_q is a queue of connections ready to be accepted.
 * If a connection is aborted and it has so_head set, then
 * it has to be pulled out of either so_q0 or so_q.
 * We allow connections to queue up based on current queue lengths
 * and limit on number of queued connections for this socket.
 */
	struct socket *so_head;  ///< back pointer to accept socket
	struct socket *so_q0;    ///< queue of partial connections
	struct socket *so_q;     ///< queue of incoming connections
	short so_q0len;          ///< partials on so_q0
	short so_qlen;           ///< number of connections on so_q
	short so_qlimit;         ///< max number queued connections
	short so_timeo;          ///< connection timeout
	u_short so_error;        ///< error affecting connection
	pid_t so_pgid;           ///< pgid for signals
	u_long so_oobmark;       ///< chars to oob mark
	/**
	 * Variables for socket buffering.
	 */
	struct sockbuf {
		u_long sb_cc;         ///< actual chars in buffer
		u_long sb_hiwat;      ///< max actual char count
		u_long sb_mbcnt;      ///< chars of mbufs used
		u_long sb_mbmax;      ///< max chars of mbufs to use
		long sb_lowat;        ///< low water mark
		struct mbuf *sb_mb;   ///< the mbuf chain
		struct selinfo sb_sel;///< process selecting read/write
		short sb_flags;       ///< flags, see below
		short sb_timeo;       ///< timeout for read/write
	} so_rcv, so_snd;
#define SB_MAX (256 * 1024)///< default for max chars in sockbuf
#define SB_LOCK 0x01       ///< lock on data queue
#define SB_WANT 0x02       ///< someone is waiting to lock
#define SB_WAIT 0x04       ///< someone is waiting for data/space
#define SB_SEL 0x08        ///< someone is selecting
#define SB_ASYNC 0x10      ///< ASYNC I/O, need signals
#define SB_NOTIFY (SB_WAIT | SB_SEL | SB_ASYNC)
#define SB_NOINTR 0x40///< operations not interruptible

	caddr_t so_tpcb;///< Wisc. protocol control block XXX
	void(*so_upcall) __P((struct socket * so, caddr_t arg, int waitf));
	caddr_t so_upcallarg;///< Arg for above
};

/*
 * Socket state bits.
 */
#define SS_NOFDREF 0x001        ///< 插口没有同任何描述符相连 no file table ref any more
#define SS_ISCONNECTED 0x002    ///< 插口被连接到外部插口 socket connected to a peer
#define SS_ISCONNECTING 0x004   ///< 插口正在连接一个外部插口 in process of connecting to peer
#define SS_ISDISCONNECTING 0x008///< 插口正在同对方断连 in process of disconnecting
#define SS_CANTSENDMORE 0x010   ///< 插口不能再发送数据给对方 can't send more data to peer
#define SS_CANTRCVMORE 0x020    ///< 插口不能再从对方接收数据 can't receive more data from peer
#define SS_RCVATMARK 0x040      ///< 在最近的带外数据到达之前,插口已处理完所有收到的数据 at mark on input

#define SS_PRIV 0x080        ///< 插口由拥有超级用户权限的进程所产生 privileged for broadcast, raw...
#define SS_NBIO 0x100        ///< 插口操作不能阻塞进程 non-blocking ops
#define SS_ASYNC 0x200       ///< 插口应该I/O事件的异步通知 async i/o notify
#define SS_ISCONFIRMING 0x400///< 插口正在协商一个连接请求 deciding to accept connection req


/*
 * Macros for sockets and socket buffering.
 */

/**
 * How much space is there in a socket buffer (so->so_snd or so->so_rcv)?
 * This is problematical if the fields are unsigned, as the space might
 * still be negative (cc > hiwat or mbcnt > mbmax).  Should detect
 * overflow and return 0.  Should use "lmin" but it doesn't exist now.
 */
#define sbspace(sb)                                    \
	((long) imin((int) ((sb)->sb_hiwat - (sb)->sb_cc), \
	             (int) ((sb)->sb_mbmax - (sb)->sb_mbcnt)))

/**
 * so中指定的协议要求每一个发送系统调用产生一个协议请求吗?
 * do we have to send all at once on a socket?
 */
#define sosendallatonce(so) \
	((so)->so_proto->pr_flags & PR_ATOMIC)

/**
 * 插口so上的读调用不阻塞就返回信息吗?
 * can we read something from so?
 */
#define soreadable(so)                              \
	((so)->so_rcv.sb_cc >= (so)->so_rcv.sb_lowat || \
	 ((so)->so_state & SS_CANTRCVMORE) ||           \
	 (so)->so_qlen || (so)->so_error)

/**
 * 插口so上的写调用不阻塞就返回吗?
 * can we write something to so?
 */
#define sowriteable(so)                                             \
	(sbspace(&(so)->so_snd) >= (so)->so_snd.sb_lowat &&             \
	         (((so)->so_state & SS_ISCONNECTED) ||                  \
	          ((so)->so_proto->pr_flags & PR_CONNREQUIRED) == 0) || \
	 ((so)->so_state & SS_CANTSENDMORE) ||                          \
	 (so)->so_error)

/**
 * adjust counters in sb reflecting allocation of m
 */
#define sballoc(sb, m)                             \
	{                                              \
		(sb)->sb_cc += (m)->m_len;                 \
		(sb)->sb_mbcnt += MSIZE;                   \
		if((m)->m_flags & M_EXT)                   \
			(sb)->sb_mbcnt += (m)->m_ext.ext_size; \
	}

/**
 * adjust counters in sb reflecting freeing of m
 */
#define sbfree(sb, m)                              \
	{                                              \
		(sb)->sb_cc -= (m)->m_len;                 \
		(sb)->sb_mbcnt -= MSIZE;                   \
		if((m)->m_flags & M_EXT)                   \
			(sb)->sb_mbcnt -= (m)->m_ext.ext_size; \
	}

/**
 * Set lock on sockbuf sb; sleep if lock is already held.
 * Unless SB_NOINTR is set on sockbuf, sleep is interruptible.
 * Returns error without lock if sleep is interrupted.
 */
#define sblock(sb, wf) ((sb)->sb_flags & SB_LOCK ? (((wf) == M_WAITOK) ? sb_lock(sb) : EWOULDBLOCK) : ((sb)->sb_flags |= SB_LOCK), 0)

/**
 * release lock on sockbuf sb
 */
#define sbunlock(sb)                            \
	{                                           \
		(sb)->sb_flags &= ~SB_LOCK;             \
		if((sb)->sb_flags & SB_WANT) {          \
			(sb)->sb_flags &= ~SB_WANT;         \
			wakeup((caddr_t) & (sb)->sb_flags); \
		}                                       \
	}

#define sorwakeup(so)                                                   \
	{                                                                   \
		sowakeup((so), &(so)->so_rcv);                                  \
		if((so)->so_upcall)                                             \
			(*((so)->so_upcall))((so), (so)->so_upcallarg, M_DONTWAIT); \
	}

#define sowwakeup(so) sowakeup((so), &(so)->so_snd)

#ifdef KERNEL
u_long sb_max;
/**
 * to catch callers missing new second argument to sonewconn:
 */
#define sonewconn(head, connstatus) sonewconn1((head), (connstatus))
struct socket *sonewconn1 __P((struct socket * head, int connstatus));

/* strings for sleep message: */
extern char netio[], netcon[], netcls[];

/*
 * File operations on sockets.
 */
int soo_read __P((struct file * fp, struct uio *uio, struct ucred *cred));
int soo_write __P((struct file * fp, struct uio *uio, struct ucred *cred));
int soo_ioctl __P((struct file * fp, u_long cmd, caddr_t data,
                   struct proc *p));
int soo_select __P((struct file * fp, int which, struct proc *p));
int soo_close __P((struct file * fp, struct proc *p));

struct mbuf;
struct sockaddr;

void sbappend __P((struct sockbuf * sb, struct mbuf *m));
int sbappendaddr __P((struct sockbuf * sb, struct sockaddr *asa,
                      struct mbuf *m0, struct mbuf *control));
int sbappendcontrol __P((struct sockbuf * sb, struct mbuf *m0,
                         struct mbuf *control));
void sbappendrecord __P((struct sockbuf * sb, struct mbuf *m0));
void sbcheck __P((struct sockbuf * sb));
void sbcompress __P((struct sockbuf * sb, struct mbuf *m, struct mbuf *n));
void sbdrop __P((struct sockbuf * sb, int len));
void sbdroprecord __P((struct sockbuf * sb));
void sbflush __P((struct sockbuf * sb));
void sbinsertoob __P((struct sockbuf * sb, struct mbuf *m0));
void sbrelease __P((struct sockbuf * sb));
int sbreserve __P((struct sockbuf * sb, u_long cc));
int sbwait __P((struct sockbuf * sb));
int sb_lock __P((struct sockbuf * sb));
int soabort __P((struct socket * so));
int soaccept __P((struct socket * so, struct mbuf *nam));
int sobind __P((struct socket * so, struct mbuf *nam));
void socantrcvmore __P((struct socket * so)); ///< 设置插口标志 SO_CANTRCVMORE。唤醒所有等待在接收缓存上的进程
void socantsendmore __P((struct socket * so));///< 设置插口标志 SO_CANTSENDMORE。唤醒所有等待在发送缓存上的进程
int soclose __P((struct socket * so));
int soconnect __P((struct socket * so, struct mbuf *nam));
int soconnect2 __P((struct socket * so1, struct socket *so2));
int socreate __P((int dom, struct socket **aso, int type, int proto));
int sodisconnect __P((struct socket * so));
void sofree __P((struct socket * so));
int sogetopt __P((struct socket * so, int level, int optname,
                  struct mbuf **mp));
void sohasoutofband __P((struct socket * so));
void soisconnected __P((struct socket * so));
void soisconnecting __P((struct socket * so));   ///< 将插口状态设置为 SO_ISCONNECTING
void soisdisconnected __P((struct socket * so)); ///< 清除SS_ISCONNECTING、SS_ISCONNECTED和SS_ISDISCONNECTING标志。设置SS_CANTRCVMORE和SS_CANTSENDMORE标志。唤醒所有等待在插口上的进程或等待close完成的进程
void soisdisconnecting __P((struct socket * so));///< 清除SS_ISCONNECTING标志。设置SS_ISDISCONG、SS_CANTRCVMORE 和SS_CANTSENDMORE标志。唤醒所有等待在插口上的进程
int solisten __P((struct socket * so, int backlog));
struct socket *
        sonewconn1 __P((struct socket * head, int connstatus));
void soqinsque __P((struct socket * head, struct socket *so, int q));///< 将so插入head指向的队列中。如果q等于0,插口被插到存放未完成的连接的so_q0队列的后面。否则,插口被插到存放准备接受的连接的队列so_q的后面。Net/1错误地将插口插到队列的前面
int soqremque __P((struct socket * so, int q));                      ///< 从队列q中删除so。通过so->so_head来定位插口队列
int soreceive __P((struct socket * so, struct mbuf **paddr, struct uio *uio,
                   struct mbuf **mp0, struct mbuf **controlp, int *flagsp));
int soreserve __P((struct socket * so, u_long sndcc, u_long rcvcc));
void sorflush __P((struct socket * so));
int sosend __P((struct socket * so, struct mbuf *addr, struct uio *uio,
                struct mbuf *top, struct mbuf *control, int flags));
int sosetopt __P((struct socket * so, int level, int optname,
                  struct mbuf *m0));
int soshutdown __P((struct socket * so, int how));
void sowakeup __P((struct socket * so, struct sockbuf *sb));
#endif /* KERNEL */
