/**
 * @file
 * Copyright (c) 1982, 1986, 1988, 1991, 1993
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
 *	@(#)uipc_mbuf.c	8.4 (Berkeley) 2/14/95
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/malloc.h>
#include <sys/map.h>
#define MBTYPES
#include <sys/mbuf.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#include <sys/domain.h>
#include <sys/protosw.h>

#include <vm/vm.h>

extern vm_map_t mb_map;
struct mbuf *mbutl;
char *mclrefcnt;

void mbinit() {
	int s;

	s = splimp();
	if(m_clalloc(max(4096 / CLBYTES, 1), M_DONTWAIT) == 0)
		goto bad;
	splx(s);
	return;
bad:
	panic("mbinit");
}

/**
 * Allocate some number of mbuf clusters
 * and place on cluster free list.
 * Must be called at splimp.
 * ARGSUSED
 */
int m_clalloc(ncl, nowait)
register int ncl;
int nowait;
{
	static int logged;
	register caddr_t p;
	int npg;

	npg = ncl * CLSIZE;
	p   = (caddr_t) kmem_malloc(mb_map, ctob(npg), !nowait);
	if(p == NULL) {
		if(logged == 0) {
			logged++;
			log(LOG_ERR, "mb_map full\n");
		}
		return 0;
	}
	ncl = ncl * CLBYTES / MCLBYTES;
	for(register int i = 0; i < ncl; i++) {
		((union mcluster *) p)->mcl_next = mclfree;
		mclfree                          = (union mcluster *) p;
		p += MCLBYTES;
		mbstat.m_clfree++;
	}
	mbstat.m_clusters += ncl;
	return 1;
}

/**
 * When MGET failes, ask protocols to free space when short of memory,
 * then re-attempt to allocate an mbuf.
 */
struct mbuf *
m_retry(i, t)
int i, t;
{
	register struct mbuf *m;

	m_reclaim();
	/*
	 * 因为在调用了m_reclaim后有可能有机会得到更多的存储器，因此再次调用宏MGET，试图获得mbuf。在展开宏MGET(图2-12)之前，m_retry被定义为一个空指针。这可以防止当存储器仍然不可用时的无休止的循环：这个MGET展开会把m设置为空指针而不是调用m_retry函数。在MGET展开以后，这个m_retry的临时定义就被取消了，以防在此之后有对MGET的其他引用。
	 */
#define m_retry(i, t) (struct mbuf *) 0
	MGET(m, i, t)
#undef m_retry
	return m;
}

/**
 * As above; retry an MGETHDR.
 */
struct mbuf *
m_retryhdr(i, t)
int i, t;
{
	register struct mbuf *m;

	m_reclaim();
#define m_retryhdr(i, t) (struct mbuf *) 0
	MGETHDR(m, i, t)
#undef m_retryhdr
	return m;
}

void m_reclaim() {
	register struct domain *dp;
	register struct protosw const *pr;
	int s = splimp();

	for(dp = domains; dp; dp = dp->dom_next)
		for(pr = dp->dom_protosw; pr < dp->dom_protoswNPROTOSW; pr++)
			if(pr->pr_drain)
				(*pr->pr_drain)();
	splx(s);
	mbstat.m_drain++;
}

/**
 * 这个调用表明参数nowait的值为M_WAIT或M_DONTWAIT,它取决于在存储器不可用时是否要求等待。例如，当插口层请求分配一个mbuf来存储sendto系统调用(图1-6)的目标地址时，它指定M_WAIT，因为在此阻塞是没有问题的。但是当以太网设备驱动程序请求分配一个mbuf来存储一个接收的帧时，它指定M_DONTWAIT，因为它是作为一个设备中断处理来执行的，不能进入睡眠状态来等待一个mbuf。在这种情况下，若存储器不可用，设备驱动程序丢弃这个帧比较好。
 * Space allocation routines.
 * These are also available as macros
 * for critical paths.
 */
struct mbuf *
m_get(nowait, type)
int nowait, type;
{
	register struct mbuf *m;

	MGET(m, nowait, type)
	return m;
}

struct mbuf *
m_gethdr(nowait, type)
int nowait, type;
{
	register struct mbuf *m;

	MGETHDR(m, nowait, type)
	return m;
}

__attribute__((unused)) struct mbuf *
m_getclr(nowait, type)
int nowait, type;
{
	register struct mbuf *m;

	MGET(m, nowait, type)
	if(m == 0)
		return 0;
	bzero(mtod(m, caddr_t), MLEN);
	return m;
}

struct mbuf *
m_free(m)
struct mbuf *m;
{
	register struct mbuf *n;

	MFREE(m, n)
	return n;
}

void
        m_freem(m) register struct mbuf *m;
{
	register struct mbuf *n;

	if(m == NULL)
		return;
	do {
		MFREE(m, n)
	} while((m = n) != NULL);
}

/*
 * Mbuffer utility routines.
 */

/**
 * Lesser-used path for M_PREPEND:
 * allocate new mbuf to prepend to chain,
 * copy junk along.
 */
struct mbuf *
m_prepend(m, len, how)
register struct mbuf *m;
int len, how;
{
	struct mbuf *mn;

	MGET(mn, how, m->m_type)
	if(mn == (struct mbuf *) NULL) {
		m_freem(m);
		return ((struct mbuf *) NULL);
	}
	if(m->m_flags & M_PKTHDR) {
		M_COPY_PKTHDR(mn, m)
		m->m_flags &= ~M_PKTHDR;
	}
	mn->m_next = m;
	m          = mn;
	if(len < MHLEN)
		MH_ALIGN(m, len)
	m->m_len = len;
	return m;
}

/**
 * Make a copy of an mbuf chain starting "off0" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * The wait parameter is a choice of M_WAIT/M_DONTWAIT from caller.
 */
int MCFail;

struct mbuf *
m_copym(m, off0, len, wait)
register struct mbuf *m;
int off0, wait;
register int len;
{
	register struct mbuf *n, **np;
	register int off = off0;
	struct mbuf *top;
	int copyhdr = 0;

	if(off < 0 || len < 0)
		panic("m_copym");
	if(off == 0 && m->m_flags & M_PKTHDR)
		copyhdr = 1;
	while(off > 0) {
		if(m == 0)
			panic("m_copym");
		if(off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np  = &top;
	top = 0;
	while(len > 0) {
		if(m == 0) {
			if(len != M_COPYALL)
				panic("m_copym");
			break;
		}
		MGET(n, wait, m->m_type)
		*np = n;
		if(n == 0)
			goto nospace;
		if(copyhdr) {
			M_COPY_PKTHDR(n, m)
			if(len == M_COPYALL)
				n->m_pkthdr.len -= off0;
			else
				n->m_pkthdr.len = len;
			copyhdr = 0;
		}
		n->m_len = min(len, m->m_len - off);
		if(m->m_flags & M_EXT) {
			n->m_data = m->m_data + off;
			mclrefcnt[mtocl(m->m_ext.ext_buf)]++;
			n->m_ext = m->m_ext;
			n->m_flags |= M_EXT;
		} else
			bcopy(mtod(m, caddr_t) + off, mtod(n, caddr_t),
			      (unsigned) n->m_len);
		if(len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m   = m->m_next;
		np  = &n->m_next;
	}
	if(top == 0)
		MCFail++;
	return top;
nospace:
	m_freem(top);
	MCFail++;
	return 0;
}

/**
 * Copy data from an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes, into the indicated buffer.
 */
int m_copydata(m, off, len, cp)
register struct mbuf *m;
register int off;
register int len;
caddr_t cp;
{
	register unsigned count;
	caddr_t old_cp = cp;

	if(off < 0 || len < 0)
		panic("m_copydata");
	while(off > 0) {
		if(m == 0)
			panic("m_copydata");
		if(off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	while(len > 0) {
		if(m == 0)
			break;// panic("m_copydata");
		count = min(m->m_len - off, len);
		bcopy(mtod(m, caddr_t) + off, cp, count);
		len -= count;
		cp += count;
		off = 0;
		m   = m->m_next;
	}
	return cp - old_cp;
}

/**
 * Concatenate mbuf chain n to m.
 * Both chains must be of the same type (e.g. MT_DATA).
 * Any m_pkthdr is not updated.
 */
void
        m_cat(m, n) register struct mbuf *m,
        *n;
{
	while(m->m_next)
		m = m->m_next;
	while(n) {
		if(m->m_flags & M_EXT ||
		   m->m_data + m->m_len + n->m_len >= &m->m_dat[MLEN]) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		      (u_int) n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

void
        m_adj(mp, req_len) struct mbuf *mp;
int req_len;
{
	register int len = req_len;
	register struct mbuf *m;
	register int count;

	if((m = mp) == NULL)
		return;
	if(len >= 0) {
		/*
		 * Trim from head.
		 */
		while(m != NULL && len > 0) {
			if(m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m        = m->m_next;
			} else {
				m->m_len -= len;
				m->m_data += len;
				len = 0;
			}
		}
		m = mp;
		if(mp->m_flags & M_PKTHDR)
			m->m_pkthdr.len -= (req_len - len);
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculating its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len   = -len;
		count = 0;
		for(;;) {
			count += m->m_len;
			if(m->m_next == (struct mbuf *) 0)
				break;
			m = m->m_next;
		}
		if(m->m_len >= len) {
			m->m_len -= len;
			if(mp->m_flags & M_PKTHDR)
				mp->m_pkthdr.len -= len;
			return;
		}
		count -= len;
		if(count < 0)
			count = 0;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		m = mp;
		if(m->m_flags & M_PKTHDR)
			m->m_pkthdr.len = count;
		for(; m; m = m->m_next) {
			if(m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while((m = m->m_next) != NULL)
			m->m_len = 0;
	}
}

/**
 * Rearange an mbuf chain so that len bytes are contiguous
 * and in the data area of an mbuf (so that mtod and dtom
 * will work for a structure of size len).  Returns the resulting
 * mbuf chain on success, frees it and returns null on failure.
 * If there is room, it will add up to max_protohdr-len extra bytes to the
 * contiguous region in an attempt to avoid being called next time.
 */
int MPFail;

/**
 *
 */
struct mbuf *
m_pullup(n, len)
register struct mbuf *n;
int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	/*
	 * If first mbuf has no cluster, and has room for len bytes
	 * without shifting current data, pullup into it,
	 * otherwise allocate a new mbuf to prepend to the chain.
	 */
	if((n->m_flags & M_EXT) == 0 &&
	   n->m_data + len < &n->m_dat[MLEN] && n->m_next) {
		if(n->m_len >= len)
			return n;
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if(len > MHLEN)
			goto bad;
		MGET(m, M_DONTWAIT, n->m_type)
		if(m == 0)
			goto bad;
		m->m_len = 0;
		if(n->m_flags & M_PKTHDR) {
			M_COPY_PKTHDR(m, n)
			n->m_flags &= ~M_PKTHDR;
		}
	}
	space = &m->m_dat[MLEN] - (m->m_data + m->m_len);
	do {
		count = min(min(max(len, max_protohdr), space), n->m_len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		      (unsigned) count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		space -= count;
		if(n->m_len)
			n->m_data += count;
		else
			n = m_free(n);
	} while(len > 0 && n);
	if(len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return m;
bad:
	m_freem(n);
	MPFail++;
	return 0;
}

/**
 * Partition an mbuf chain in two pieces, returning the tail --
 * all but the first len0 bytes.  In case of failure, it returns NULL and
 * attempts to restore the chain to its original state.
 */
struct mbuf *
m_split(m0, len0, wait)
register struct mbuf *m0;
int len0, wait;
{
	register struct mbuf *m, *n;
	unsigned len = len0, remain;

	for(m = m0; m && len > m->m_len; m = m->m_next)
		len -= m->m_len;
	if(m == 0)
		return 0;
	remain = m->m_len - len;
	if(m0->m_flags & M_PKTHDR) {
		MGETHDR(n, wait, m0->m_type)
		if(n == 0)
			return 0;
		n->m_pkthdr.rcvif = m0->m_pkthdr.rcvif;
		n->m_pkthdr.len   = m0->m_pkthdr.len - len0;
		m0->m_pkthdr.len  = len0;
		if(m->m_flags & M_EXT)
			goto extpacket;
		if(remain > MHLEN) {
			/* m can't be the lead packet */
			MH_ALIGN(n, 0)
			n->m_next = m_split(m, len, wait);
			if(n->m_next == 0) {
				(void) m_free(n);
				return 0;
			} else
				return n;
		} else
			MH_ALIGN(n, remain)
	} else if(remain == 0) {
		n         = m->m_next;
		m->m_next = 0;
		return n;
	} else {
		MGET(n, wait, m->m_type)
		if(n == 0)
			return 0;
		M_ALIGN(n, remain)
	}
extpacket:
	if(m->m_flags & M_EXT) {
		n->m_flags |= M_EXT;
		n->m_ext = m->m_ext;
		mclrefcnt[mtocl(m->m_ext.ext_buf)]++;
		m->m_ext.ext_size = 0; /* For Accounting XXXXXX danger */
		n->m_data         = m->m_data + len;
	} else {
		bcopy(mtod(m, caddr_t) + len, mtod(n, caddr_t), remain);
	}
	n->m_len  = remain;
	m->m_len  = len;
	n->m_next = m->m_next;
	m->m_next = 0;
	return n;
}
/**
 * 当接收到一个以太网帧时，设备驱动程序调用函数m_devget来创建一个mbuf链表，并把设备中的帧复制到这个链表中。根据所接收的帧的长度（不包括以太网首部），可能导致4种不同的mbuf链表。
 * Routine to copy from device local memory into mbufs.
 */
struct mbuf *
m_devget(buf, totlen, off0, ifp, copy)
char *buf;
int totlen, off0;
struct ifnet *ifp;
void (*copy)();
{
	register struct mbuf *m;
	struct mbuf *top = 0;
	struct mbuf **mp = &top;
	register int off = off0;
	register int len;
	register char *cp;
	char const *epkt;

	cp   = buf;
	epkt = cp + totlen;
	if(off) {
		/*
		 * If 'off' is non-zero, packet is trailer-encapsulated,
		 * so we have to skip the type and length fields.
		 */
		cp += off + 2 * sizeof(u_int16_t);
		totlen -= 2 * sizeof(u_int16_t);
	}
	MGETHDR(m, M_DONTWAIT, MT_DATA)
	if(m == 0)
		return 0;
	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len   = totlen;
	m->m_len          = MHLEN;

	while(totlen > 0) {
		if(top) {
			MGET(m, M_DONTWAIT, MT_DATA)
			if(m == 0) {
				m_freem(top);
				return 0;
			}
			m->m_len = MLEN;
		}
		len = min(totlen, epkt - cp);
		if(len >= MINCLSIZE) {
			MCLGET(m, M_DONTWAIT)
			if(m->m_flags & M_EXT)
				m->m_len = len = min(len, MCLBYTES);
			else
				len = m->m_len;
		} else {
			/*
			 * Place initial small packet/header at end of mbuf.
			 */
			if(len < m->m_len) {
				if(top == 0 && len + max_linkhdr <= m->m_len)
					m->m_data += max_linkhdr;
				m->m_len = len;
			} else
				len = m->m_len;
		}
		if(copy)
			copy(cp, mtod(m, caddr_t), (unsigned) len);
		else
			bcopy(cp, mtod(m, caddr_t), (unsigned) len);
		cp += len;
		*mp = m;
		mp  = &m->m_next;
		totlen -= len;
		if(cp == epkt)
			cp = buf;
	}
	return top;
}
