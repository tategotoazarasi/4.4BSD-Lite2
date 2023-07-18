/**
 * @file
 * Copyright (c) 1982, 1986, 1988, 1993
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
 *	@(#)mbuf.h	8.5 (Berkeley) 2/19/95
 */

#ifndef M_WAITOK
#include <sys/malloc.h>
#endif

/**
 * Mbufs are of a single size, MSIZE (machine/machparam.h), which
 * includes overhead.  An mbuf may add a single "mbuf cluster" of size
 * MCLBYTES (also in machine/machparam.h), which has no additional overhead
 * and is used instead of the internal data area; this is done when
 * at least MINCLSIZE of data must be stored.
 */

#define MLEN (MSIZE - sizeof(struct m_hdr)) /// 在正常mbuf中的最大数据量 normal data len
#define MHLEN (MLEN - sizeof(struct pkthdr))/// 带分组首部的mbuf的最大数据量 data len w/pkthdr

#define MINCLSIZE (MHLEN + MLEN) /// 存储到簇中的最小数据量 smallest amount to put in cluster
#define M_MAXCOMPRESS (MHLEN / 2)/// max amount to copy for compression

/**
* @defgroup macros4typconv Macros for type conversion
* @{
*/
#define mtod(m, t) ((t) ((m)->m_data))                                    ///< 将m指向的mbuf的数据区指针的类型转换成type类型。返回一个指向mbuf数据的指针，并把指针声明为指定类型。 convert mbuf pointer to data pointer of correct type
#define dtom(x) ((struct mbuf *) ((long) (x) & ~(MSIZE - 1)))             ///< 将指向一个mbuf数据区中某个位置的指针x转换成一个指向这个mbuf的起始的指针。取得一个存放在一个mbuf中任意位置的数据的指针，并返回这个mbuf结构本身的一个指针。 convert data pointer within mbuf to mbuf pointer (XXX)
#define mtocl(x) (((u_long) (x) - (u_long) mbutl) >> MCLSHIFT)            ///< convert pointer within cluster to cluster index #
#define cltom(x) ((caddr_t) ((u_long) mbutl + ((u_long) (x) << MCLSHIFT)))///< convert cluster # to ptr to beginning of cluster
/** @} */

/**
 * header at beginning of each mbuf:
 */
struct m_hdr {
	struct mbuf *mh_next;   ///< next buffer in chain
	struct mbuf *mh_nextpkt;///< next chain in queue/record
	caddr_t mh_data;        ///< location of data
	int mh_len;             ///< amount of data in this mbuf
	short mh_type;          ///< type of data in this mbuf
	short mh_flags;         ///< flags; see below
};

/// record/packet header in first mbuf of chain; valid if M_PKTHDR set
struct pkthdr {
	struct ifnet *rcvif;///< rcv interface
	int len;            ///< total packet length
};

/// description of external storage mapped into mbuf, valid if M_EXT set
struct m_ext {
	caddr_t ext_buf;   ///< start of buffer
	void (*ext_free)();///< free routine if not the usual
	u_int ext_size;    ///< size of buffer, for ext_free
};

struct mbuf {
	struct m_hdr m_hdr;
	union {
		struct {
			struct pkthdr MH_pkthdr;///< M_PKTHDR set
			union {
				struct m_ext MH_ext;///< M_EXT set
				char MH_databuf[MHLEN];
			} MH_dat;
		} MH;
		char M_databuf[MLEN];///< !M_PKTHDR, !M_EXT
	} M_dat;
};
#define m_next m_hdr.mh_next
#define m_len m_hdr.mh_len
#define m_data m_hdr.mh_data
#define m_type m_hdr.mh_type
#define m_flags m_hdr.mh_flags
#define m_nextpkt m_hdr.mh_nextpkt
#define m_act m_nextpkt
#define m_pkthdr M_dat.MH.MH_pkthdr
#define m_ext M_dat.MH.MH_dat.MH_ext
#define m_pktdat M_dat.MH.MH_dat.MH_databuf
#define m_dat M_dat.M_databuf

/**
 * @defgroup mbuf_flags mbuf flags
 * @{
 */
#define M_EXT 0x0001   ///< 此mbuf带有簇（外部缓存） has associated external storage
#define M_PKTHDR 0x0002///< 形成一个分组（记录）的第一个mbuf start of record
#define M_EOR 0x0004   ///< 记录结束 end of record
/** @} */

/**
 * @defgroup m_flags mbuf pkthdr flags, also in m_flags
 * @{
 */
#define M_BCAST 0x0100///< 作为链路层广播发送/接收 send/received as link-level broadcast
#define M_MCAST 0x0200///< 作为链路层多播发送/接收 send/received as link-level multicast
/** @} */

/// flags copied when copying m_pkthdr
#define M_COPYFLAGS (M_PKTHDR | M_EOR | M_BCAST | M_MCAST)

/**
* @defgroup mbuf_types mbuf types
* @{
*/
#define MT_FREE 0    ///< 应在自由列表中 should be on free list
#define MT_DATA 1    ///< 动态数据分配 dynamic (data) allocation
#define MT_HEADER 2  ///< 分组首部 packet header
#define MT_SOCKET 3  ///< 插口结构 socket structure
#define MT_PCB 4     ///< 协议控制块 protocol control block
#define MT_RTABLE 5  ///< 路由表 routing tables
#define MT_HTABLE 6  ///< IMP主机表 IMP host tables
#define MT_ATABLE 7  ///< address resolution tables
#define MT_SONAME 8  ///< 插口名称 socket name
#define MT_SOOPTS 10 ///< 插口选项 socket options
#define MT_FTABLE 11 ///< 分片重组首部 fragment reassembly header
#define MT_RIGHTS 12 ///< 访问权限 access rights
#define MT_IFADDR 13 ///< 接口地址 interface address
#define MT_CONTROL 14///< 外部数据协议报文 extra-data protocol message
#define MT_OOBDATA 15///< 加速（带外）数据 expedited data
/** @} */

/**
* @defgroup MGET_flags flags to m_get/MGET
* @{
*/
#define M_DONTWAIT M_NOWAIT
#define M_WAIT M_WAITOK
/** @} */

/**
 * mbuf utility macros:
 *
 *	MBUFLOCK(code)
 * prevents a section of code from from being interrupted by network
 * drivers.
 */
#define MBUFLOCK(code)     \
	{                      \
		int ms = splimp(); \
		{code} splx(ms);   \
	}

/**
 * mbuf allocation/deallocation macros:
 *
 *	MGET(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain internal data.
 *
 *	MGETHDR(struct mbuf *m, int how, int type)
 * allocates an mbuf and initializes it to contain a packet header
 * and internal data.
 */
#define MGET(m, how, type)                                       \
	{                                                            \
		MALLOC((m), struct mbuf *, MSIZE, mbtypes[type], (how)); \
		if(m) {                                                  \
			(m)->m_type = (type);                                \
			MBUFLOCK(mbstat.m_mtypes[type]++;)                   \
			(m)->m_next    = (struct mbuf *) NULL;               \
			(m)->m_nextpkt = (struct mbuf *) NULL;               \
			(m)->m_data    = (m)->m_dat;                         \
			(m)->m_flags   = 0;                                  \
		} else                                                   \
			(m) = m_retry((how), (type));                        \
	}

/**
 * 分配一个mbuf,并把它初始化为一个分组首部。这个宏与MGET(图2-12)相似,但设置了标志M_PKTHDR,并且数据指针(m_data)指向紧接分组首部后的100字节的缓存。
 */
#define MGETHDR(m, how, type)                                    \
	{                                                            \
		MALLOC((m), struct mbuf *, MSIZE, mbtypes[type], (how)); \
		if(m) {                                                  \
			(m)->m_type = (type);                                \
			MBUFLOCK(mbstat.m_mtypes[type]++;)                   \
			(m)->m_next    = (struct mbuf *) NULL;               \
			(m)->m_nextpkt = (struct mbuf *) NULL;               \
			(m)->m_data    = (m)->m_pktdat;                      \
			(m)->m_flags   = M_PKTHDR;                           \
		} else                                                   \
			(m) = m_retryhdr((how), (type));                     \
	}

/**
 * Mbuf cluster macros.
 * MCLALLOC(caddr_t p, int how) allocates an mbuf cluster.
 * MCLGET adds such clusters to a normal mbuf;
 * the flag M_EXT is set upon success.
 * MCLFREE releases a reference to a cluster allocated by MCLALLOC,
 * freeing the cluster if the reference count has reached 0.
 *
 * Normal mbuf clusters are normally treated as character arrays
 * after allocation, but use the first word of the buffer as a free list
 * pointer while on the free list.
 */
union mcluster {
	union mcluster *mcl_next;
	char mcl_buf[MCLBYTES];
};

#define MCLALLOC(p, how)                                      \
	MBUFLOCK(                                                 \
	        if(mclfree == 0)(void) m_clalloc(1, (how));       \
	        if(((p) = (caddr_t) mclfree) != 0) {              \
		        ++mclrefcnt[mtocl(p)];                        \
		        mbstat.m_clfree--;                            \
		        mclfree = ((union mcluster *) (p))->mcl_next; \
	        })

/**
 * 获得一个簇 (一个外部缓存 )并将m指向的mbuf中的数据指针 (m_data)设置为指向这个簇。如果存储器不可用,返回时不设置mbuf中的M_EXT标志。
 */
#define MCLGET(m, how)                        \
	{                                         \
		MCLALLOC((m)->m_ext.ext_buf, (how));  \
		if((m)->m_ext.ext_buf != NULL) {      \
			(m)->m_data = (m)->m_ext.ext_buf; \
			(m)->m_flags |= M_EXT;            \
			(m)->m_ext.ext_size = MCLBYTES;   \
		}                                     \
	}

#define MCLFREE(p)                                                           \
	MBUFLOCK(                                                                \
	        if(--mclrefcnt[mtocl(p)] == 0) {                                 \
		        ((union mcluster *) (p))->mcl_next = mclfree;                \
		        mclfree                            = (union mcluster *) (p); \
		        mbstat.m_clfree++;                                           \
	        })

/**
 * MFREE(struct mbuf *m, struct mbuf *n)
 * Free a single mbuf and associated external storage.
 * Place the successor, if any, in n.
 * 释放一个m指向的mbuf。若m指向一个簇 (设置了M_EXT),这个簇的引用计数器减1,但这个簇并不被释放,直到它的引用计数器降为0。返回m的后继(由m->m_next指向,可以为空)存放在n中。
 */
#ifdef notyet
#define MFREE(m, n)                                            \
	{                                                          \
		MBUFLOCK(mbstat.m_mtypes[(m)->m_type]--;)              \
		if((m)->m_flags & M_EXT) {                             \
			if((m)->m_ext.ext_free)                            \
				(*((m)->m_ext.ext_free))((m)->m_ext.ext_buf,   \
				                         (m)->m_ext.ext_size); \
			else                                               \
				MCLFREE((m)->m_ext.ext_buf);                   \
		}                                                      \
		(n) = (m)->m_next;                                     \
		FREE((m), mbtypes[(m)->m_type]);                       \
	}
#else /* notyet */
#define MFREE(m, nn)                              \
	{                                             \
		MBUFLOCK(mbstat.m_mtypes[(m)->m_type]--;) \
		if((m)->m_flags & M_EXT) {                \
			MCLFREE((m)->m_ext.ext_buf);          \
		}                                         \
		(nn) = (m)->m_next;                       \
		FREE((m), mbtypes[(m)->m_type]);          \
	}
#endif

/**
 * Copy mbuf pkthdr from from to to.
 * from must have M_PKTHDR set, and to must be empty.
 */
#define M_COPY_PKTHDR(to, from)                         \
	{                                                   \
		(to)->m_pkthdr = (from)->m_pkthdr;              \
		(to)->m_flags  = (from)->m_flags & M_COPYFLAGS; \
		(to)->m_data   = (to)->m_pktdat;                \
	}

/**
 * Set the m_data pointer of a newly-allocated mbuf (m_get/MGET) to place
 * an object of the specified size at the end of the mbuf, longword aligned.
 */
#define M_ALIGN(m, len) \
	{ (m)->m_data += (MLEN - (len)) & ~(sizeof(long) - 1); }
/**
 * As above, for mbufs allocated with m_gethdr/MGETHDR
 * or initialized by M_COPY_PKTHDR.
 * 设置包含一个分组首部的mbuf的m_data,在这个mbuf数据区的尾部为一个长度为len字节的对象提供空间。这个数据指针也是长字对准方式的。
 */
#define MH_ALIGN(m, len) \
	{ (m)->m_data += (MHLEN - (len)) & ~(sizeof(long) - 1); }

/**
 * Compute the amount of space available
 * before the current start of data in an mbuf.
 */
#define M_LEADINGSPACE(m)                                                                                                    \
	((m)->m_flags & M_EXT ? /* (m)->m_data - (m)->m_ext.ext_buf */ 0 : (m)->m_flags & M_PKTHDR ? (m)->m_data - (m)->m_pktdat \
	                                                                                           : (m)->m_data - (m)->m_dat)

/**
 * Compute the amount of space available
 * after the end of data in an mbuf.
 */
#define M_TRAILINGSPACE(m)                                             \
	((m)->m_flags & M_EXT ? (m)->m_ext.ext_buf + (m)->m_ext.ext_size - \
	                                ((m)->m_data + (m)->m_len)         \
	                      : &(m)->m_dat[MLEN] - ((m)->m_data + (m)->m_len))

/**
 * Arrange to prepend space of size plen to mbuf m.
 * If a new mbuf must be allocated, how specifies whether to wait.
 * If how is M_DONTWAIT and allocation fails, the original mbuf chain
 * is freed and m is set to NULL.
 * 在m指向的mbuf中的数据的前面添加len字节的数据。如果mbuf有空间,则仅把指针(m_data)减len字节,并将长度(m_len)增加len字节。如果没有足够的空间,就分配一个新的mbuf,它的m_next指针被设置为m。一个新mbuf的指针存放在m中。并且新mbuf的数据指针被设置,这样len字节的数据放置到这个mbuf的尾部(例如,调用MH_ALIGN)。如果一个新mbuf被分配,并且原来的mbuf的分组首部标志被设置,则分组首部从老mbuf中移到新mbuf中。
 */
#define M_PREPEND(m, plen, how)                  \
	{                                            \
		if(M_LEADINGSPACE(m) >= (plen)) {        \
			(m)->m_data -= (plen);               \
			(m)->m_len += (plen);                \
		} else                                   \
			(m) = m_prepend((m), (plen), (how)); \
		if((m) && (m)->m_flags & M_PKTHDR)       \
			(m)->m_pkthdr.len += (plen);         \
	}

/// change mbuf to new type
#define MCHTYPE(m, t)                                                   \
	{                                                                   \
		MBUFLOCK(mbstat.m_mtypes[(m)->m_type]--; mbstat.m_mtypes[t]++;) \
		(m)->m_type = t;                                                \
	}

/// length to m_copy to copy all */
#define M_COPYALL 1000000000

/**
 * 这是m_copym的三参数版本,它隐含的第4个参数的值为M_DONTWAIT。
 * compatiblity with 4.3
 */
#define m_copy(m, o, l) m_copym((m), (o), (l), M_DONTWAIT)

/**
 * mbuf统计 Mbuf statistics.
 */
struct mbstat {
	__attribute__((unused)) u_long m_mbufs;///< 从页池（未用）中获得的mbuf数 mbufs obtained from page pool
	u_long m_clusters;                     ///< 从页池中获得的簇 clusters obtained from page pool
	__attribute__((unused)) u_long m_spare;///< 剩余空间（未用） spare field
	u_long m_clfree;                       ///< 自由簇 free clusters
	__attribute__((unused)) u_long m_drops;///< 寻找空间（未用）失败的次数 times failed to find space
	__attribute__((unused)) u_long m_wait; ///< 等待空间（未用）的次数 times waited for space
	u_long m_drain;                        ///< 调用协议的drain函数来回收空间的次数 times drained protocols for space
	u_short m_mtypes[256];                 ///< 当前mbuf的分配数量：MT_xxx索引 type specific mbuf allocations
};

#ifdef KERNEL
extern struct mbuf *mbutl;///< virtual address of mclusters
extern char *mclrefcnt;   ///< cluster reference counts
struct mbstat mbstat;     ///< mbuf的统计信息
__attribute__((unused)) extern int nmbclusters;
union mcluster *mclfree;
int max_linkhdr;                        ///< largest link-level header
int max_protohdr;                       ///< largest protocol header
int max_hdr;                            ///< largest link+protocol header
__attribute__((unused)) int max_datalen;///< MHLEN - max_hdr
extern int mbtypes[];                   ///< XXX

struct mbuf *m_copym __P((struct mbuf *, int, int, int) );                ///< 创建一个新的mbuf链表,并从m指向的mbuf链表的开始offset处复制len字节的数据。一个新mbuf链表的指针作为此函数的返回值。如果len等于常量M_COPYALL,则从这个mbuf链表的offset开始的所有数据都将被复制。
struct mbuf *m_free __P((struct mbuf *) );                                ///< 宏MFREE的函数版本。
struct mbuf *m_devget __P((char *, int, int, struct ifnet *, void (*)()));///< 创建一个带分组首部的mbuf链表,并返回指向这个链表的指针。这个分组首部的len和rcvif字段被设置为len和ifp。调用函数copy从设备接口(由buf指向)将数据复制到mbuf中。如果copy是一个空指针,调用函数bcopy。由于尾部协议不再被支持,off为0。
struct mbuf *m_get __P((int, int) );                                      ///< 宏MGET的函数版本。
__attribute__((unused)) struct mbuf *m_getclr __P((int, int) );           ///< 此函数调用宏MGET来得到一个mbuf,并把108字节的缓存清零。
struct mbuf *m_gethdr __P((int, int) );                                   ///< 宏MGETHDR的函数版本。
struct mbuf *m_prepend __P((struct mbuf *, int, int) );
struct mbuf *m_pullup __P((struct mbuf *, int) );///< 重新排列由m指向的mbuf中的数据,使得前len字节的数据连续地存储在链表中的第一个mbuf中。如果这个函数成功,则宏mtod能返回一个正好指向这个大小为len的结构。
struct mbuf *m_retry __P((int, int) );
struct mbuf *m_retryhdr __P((int, int) );
void m_adj __P((struct mbuf *, int) );          ///< 从m指向的mbuf中移走len字节的数据。如果len是正数,则所操作的是紧排在这个mbuf的开始的len字节数据;否则是紧排在这个mbuf的尾部的len绝对值字节数据。
void m_cat __P((struct mbuf *, struct mbuf *) );///< 把由n指向的mbuf链表链接到由m指向的mbuf链表的尾部。当我们讨论IP重组时(第10章)会遇到这个函数。
int m_clalloc __P((int, int) );
int m_copydata __P((struct mbuf *, int, int, caddr_t));///< 从m指向的mbuf链表中复制len字节数据到由cp指向的缓存。从mbuf链表数据区起始的offset字节开始复制。
void m_freem __P((struct mbuf *) );                    ///< 释放m指向的链表中的所有mbuf。
void m_reclaim __P((void) );

#ifdef MBTYPES
int mbtypes[] = {
        /* XXX */
        M_FREE,   /* MT_FREE	0	   should be on free list */
        M_MBUF,   /* MT_DATA	1	   dynamic (data) allocation */
        M_MBUF,   /* MT_HEADER	2	   packet header */
        M_SOCKET, /* MT_SOCKET	3	   socket structure */
        M_PCB,    /* MT_PCB	4	   protocol control block */
        M_RTABLE, /* MT_RTABLE	5	   routing tables */
        M_HTABLE, /* MT_HTABLE	6	   IMP host tables */
        0,        /* MT_ATABLE	7	   address resolution tables */
        M_MBUF,   /* MT_SONAME	8	   socket name */
        0,        /* 		9 */
        M_SOOPTS, /* MT_SOOPTS	10	   socket options */
        M_FTABLE, /* MT_FTABLE	11	   fragment reassembly header */
        M_MBUF,   /* MT_RIGHTS	12	   access rights */
        M_IFADDR, /* MT_IFADDR	13	   interface address */
        M_MBUF,   /* MT_CONTROL	14	   extra-data protocol message */
        M_MBUF,   /* MT_OOBDATA	15	   expedited data  */
#ifdef DATAKIT
        25, 26, 27, 28, 29, 30, 31, 32 /* datakit ugliness */
#endif
};
#endif
#endif
