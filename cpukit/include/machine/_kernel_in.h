/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
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
 * 3. Neither the name of the University nor the names of its contributors
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
 *	@(#)in.h	8.3 (Berkeley) 1/3/94
 * $FreeBSD: head/sys/netinet/in.h 326023 2017-11-20 19:43:44Z pfg $
 */

/**
 * @file
 *
 * @ingroup RTEMSImplFreeBSDKernel
 *
 * @brief This header file provides IPv4 definitions for the kernel space
 *   (_KERNEL is defined before including <netinet/in.h>).
 */

#if defined(_NETINET_IN_H_) && defined(_KERNEL)

struct ifnet; struct mbuf;	/* forward declarations for Standard C */
struct in_ifaddr;

int	 in_broadcast(struct in_addr, struct ifnet *);
int	 in_ifaddr_broadcast(struct in_addr, struct in_ifaddr *);
int	 in_canforward(struct in_addr);
int	 in_localaddr(struct in_addr);
#if __FreeBSD_version >= 1400039
bool	 in_localip(struct in_addr);
#else
int	 in_localip(struct in_addr);
#endif
bool	 in_localip_fib(struct in_addr, uint16_t);
int	 in_ifhasaddr(struct ifnet *, struct in_addr);
struct in_ifaddr *in_findlocal(uint32_t, bool);
int	 inet_aton(const char *, struct in_addr *); /* in libkern */
char	*inet_ntoa_r(struct in_addr ina, char *buf); /* in libkern */
char	*inet_ntop(int, const void *, char *, socklen_t); /* in libkern */
int	 inet_pton(int af, const char *, void *); /* in libkern */
void	 in_ifdetach(struct ifnet *);

#define	in_hosteq(s, t)	((s).s_addr == (t).s_addr)
#define	in_nullhost(x)	((x).s_addr == INADDR_ANY)
#define	in_allhosts(x)	((x).s_addr == htonl(INADDR_ALLHOSTS_GROUP))

/* Legacy classful IPv4 macros required by classic libnetworking code. */
#ifndef IN_CLASSA
#define IN_CLASSA(i)		(((uint32_t)(i) & 0x80000000U) == 0)
#endif
#ifndef IN_CLASSA_NET
#define IN_CLASSA_NET		0xff000000U
#endif
#ifndef IN_CLASSA_NSHIFT
#define IN_CLASSA_NSHIFT	24
#endif
#ifndef IN_CLASSA_HOST
#define IN_CLASSA_HOST		0x00ffffffU
#endif

#ifndef IN_CLASSB
#define IN_CLASSB(i)		(((uint32_t)(i) & 0xc0000000U) == 0x80000000U)
#endif
#ifndef IN_CLASSB_NET
#define IN_CLASSB_NET		0xffff0000U
#endif
#ifndef IN_CLASSB_NSHIFT
#define IN_CLASSB_NSHIFT	16
#endif

#ifndef IN_CLASSC
#define IN_CLASSC(i)		(((uint32_t)(i) & 0xe0000000U) == 0xc0000000U)
#endif
#ifndef IN_CLASSC_NET
#define IN_CLASSC_NET		0xffffff00U
#endif
#ifndef IN_CLASSC_NSHIFT
#define IN_CLASSC_NSHIFT	8
#endif

#ifndef IN_LOOPBACKNET
#define IN_LOOPBACKNET		127
#endif

#define	satosin(sa)	((struct sockaddr_in *)(sa))
#define	sintosa(sin)	((struct sockaddr *)(sin))
#define	ifatoia(ifa)	((struct in_ifaddr *)(ifa))

#else /* !_NETINET_IN_H_ || !_KERNEL */
#error "must be included via <netinet/in.h> in kernel space"
#endif /* _NETINET_IN_H_ && _KERNEL */
