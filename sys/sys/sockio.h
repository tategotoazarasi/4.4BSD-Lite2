/**
 * @copyright
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
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
 *	@(#)sockio.h	8.1 (Berkeley) 3/28/94
 */

#ifndef _SYS_SOCKIO_H_
#define _SYS_SOCKIO_H_

#include <sys/ioccom.h>

/* Socket ioctl's. */
#define SIOCSHIWAT _IOW('s', 0, int)///< set high watermark
#define SIOCGHIWAT _IOR('s', 1, int)///< get high watermark
#define SIOCSLOWAT _IOW('s', 2, int)///< set low watermark
#define SIOCGLOWAT _IOR('s', 3, int)///< get low watermark
#define SIOCATMARK _IOR('s', 7, int)///< at oob mark?
#define SIOCSPGRP _IOW('s', 8, int) ///< set process group
#define SIOCGPGRP _IOR('s', 9, int) ///< get process group

#define SIOCADDRT _IOW('r', 10, struct ortentry)///< add route
#define SIOCDELRT _IOW('r', 11, struct ortentry)///< delete route

#define SIOCSIFADDR _IOW('i', 12, struct ifreq)     ///< 设置接口地址 set ifnet address
#define OSIOCGIFADDR _IOWR('i', 13, struct ifreq)   ///< get ifnet address
#define SIOCGIFADDR _IOWR('i', 33, struct ifreq)    ///< 获得接口地址 get ifnet address
#define SIOCSIFDSTADDR _IOW('i', 14, struct ifreq)  ///< 设置接口目标地址 set p-p address
#define OSIOCGIFDSTADDR _IOWR('i', 15, struct ifreq)///< get p-p address
#define SIOCGIFDSTADDR _IOWR('i', 34, struct ifreq) ///< 获得接口目标地址 get p-p address
#define SIOCSIFFLAGS _IOW('i', 16, struct ifreq)    ///< 设置接口标志 set ifnet flags
#define SIOCGIFFLAGS _IOWR('i', 17, struct ifreq)   ///< 获得接口标志 get ifnet flags
#define OSIOCGIFBRDADDR _IOWR('i', 18, struct ifreq)///< get broadcast addr
#define SIOCGIFBRDADDR _IOWR('i', 35, struct ifreq) ///< 获得接口广播地址 get broadcast addr
#define SIOCSIFBRDADDR _IOW('i', 19, struct ifreq)  ///< 设置接口广播地址 set broadcast addr
#define OSIOCGIFCONF _IOWR('i', 20, struct ifconf)  ///< 获取接口配置清单 get ifnet list
#define SIOCGIFCONF _IOWR('i', 36, struct ifconf)   ///< get ifnet list
#define OSIOCGIFNETMASK _IOWR('i', 21, struct ifreq)///< get net addr mask
#define SIOCGIFNETMASK _IOWR('i', 37, struct ifreq) ///< 获得接口网络掩码 get net addr mask
#define SIOCSIFNETMASK _IOW('i', 22, struct ifreq)  ///< 设置接口网络掩码 set net addr mask
#define SIOCGIFMETRIC _IOWR('i', 23, struct ifreq)  ///< 获得接口度量 get IF metric
#define SIOCSIFMETRIC _IOW('i', 24, struct ifreq)   ///< 设置接口度量 set IF metric
#define SIOCDIFADDR _IOW('i', 25, struct ifreq)     ///< 删除接口地址 delete IF addr
#define SIOCAIFADDR _IOW('i', 26, struct ifaliasreq)///< 添加接口地址 add/chg IF alias

#define SIOCADDMULTI _IOW('i', 49, struct ifreq)///< 在接收表里加上多播地址 add m'cast addr
#define SIOCDELMULTI _IOW('i', 50, struct ifreq)///< 在接收表里删去多播地址 del m'cast addr

#endif /* !_SYS_SOCKIO_H_ */
