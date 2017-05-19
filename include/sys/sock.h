#ifndef _platform_socket_h_
#define _platform_socket_h_

#if defined(OS_WINDOWS)
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <ws2ipdef.h>

#ifndef OS_SOCKET_TYPE
typedef SOCKET	socket_t;
typedef WSABUF	socket_bufvec_t;
#define OS_SOCKET_TYPE
#endif /* OS_SOCKET_TYPE */

#define socket_invalid	INVALID_SOCKET
#define socket_error	SOCKET_ERROR

#if defined(_MSC_VER)
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(push)
#pragma warning(disable: 6031) // warning C6031: Return value ignored: 'snprintf'
#endif

#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH
#else
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>

#ifndef OS_SOCKET_TYPE
typedef int socket_t;
typedef struct iovec socket_bufvec_t;
#define OS_SOCKET_TYPE
#endif /* OS_SOCKET_TYPE */

#define socket_invalid	-1
#define socket_error	-1

#endif

#include <assert.h>
#include <stdio.h>

#ifndef IN
#define IN 
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#define SOCKET_ADDRLEN INET6_ADDRSTRLEN

static int socket_init(void);
static int socket_cleanup(void);
static int socket_geterror(void);

static socket_t socket_tcp(void);
static socket_t socket_udp(void);
static socket_t socket_raw(void);
static socket_t socket_rdm(void);
static socket_t socket_tcp_ipv6(void);
static socket_t socket_udp_ipv6(void);
static socket_t socket_raw_ipv6(void);
// @return 0-ok, <0-socket_error(by socket_geterror())
static int socket_shutdown(socket_t sock, int flag); // SHUT_RD/SHUT_WR/SHUT_RDWR
static int socket_close(socket_t sock);

// @return 0-ok, <0-socket_error(by socket_geterror())
static int socket_connect(IN socket_t sock, IN const struct sockaddr* addr, IN socklen_t addrlen);
// MSDN: When using bind with the SO_EXCLUSIVEADDR or SO_REUSEADDR socket option, the socket option must be set prior to executing bind to have any affect
static int socket_bind(IN socket_t sock, IN const struct sockaddr* addr, IN socklen_t addrlen);
static int socket_listen(IN socket_t sock, IN int backlog);
// @return >=0-accepted socket, <0-socket_invalid(by socket_geterror())
static socket_t socket_accept(IN socket_t sock, OUT struct sockaddr_storage* ss, OUT socklen_t* addrlen);

// socket read/write
// @return >0-sent/received bytes, <0-socket_error(by socket_geterror()), 0-peer shutdown(recv only)
static int socket_send(IN socket_t sock, IN const void* buf, IN size_t len, IN int flags);
static int socket_recv(IN socket_t sock, OUT void* buf, IN size_t len, IN int flags);
static int socket_sendto(IN socket_t sock, IN const void* buf, IN size_t len, IN int flags, IN const struct sockaddr* to, IN socklen_t tolen);
static int socket_recvfrom(IN socket_t sock, OUT void* buf, IN size_t len, IN int flags, OUT struct sockaddr* from, OUT socklen_t* fromlen);

// @return >0-sent/received bytes, <0-socket_error(by socket_geterror()), 0-peer shutdown(recv only)
static int socket_send_v(IN socket_t sock, IN const socket_bufvec_t* vec, IN size_t n, IN int flags);
static int socket_recv_v(IN socket_t sock, IN socket_bufvec_t* vec, IN size_t n, IN int flags);
static int socket_sendto_v(IN socket_t sock, IN const socket_bufvec_t* vec, IN size_t n, IN int flags, IN const struct sockaddr* to, IN socklen_t tolen);
static int socket_recvfrom_v(IN socket_t sock, IN socket_bufvec_t* vec, IN size_t n, IN int flags, IN struct sockaddr* from, IN socklen_t* fromlen);

// Linux: 1. select may update the timeout argument to indicate how much time was left
//        2. This interval will be rounded up to the system clock granularity, and kernel scheduling delays mean that the blocking interval may overrun by a small amount.
// @return 0-timeout, >0-available fds, <0-socket_error(by socket_geterror())
static int socket_select(IN int n, IN fd_set* rfds, IN fd_set* wfds, IN fd_set* efds, IN struct timeval* timeout); // timeout: NULL-forever, 0-immediately
static int socket_select_readfds(IN int n, IN fd_set* fds, IN struct timeval* timeout); // timeout: NULL-forever, 0-immediately
static int socket_select_writefds(IN int n, IN fd_set* fds, IN struct timeval* timeout); // timeout: NULL-forever, 0-immediately
static int socket_select_read(IN socket_t sock, IN int timeout); // timeout: >0-milliseconds, 0-immediately, <0-forever
static int socket_select_write(IN socket_t sock, IN int timeout); // timeout: >0-milliseconds, 0-immediately, <0-forever
static int socket_readable(IN socket_t sock);
static int socket_writeable(IN socket_t sock);

// socket options
// @return 0-ok, <0-socket_error(by socket_geterror())
static int socket_setkeepalive(IN socket_t sock, IN int enable); // keep alive
static int socket_getkeepalive(IN socket_t sock, OUT int* enable);
static int socket_setlinger(IN socket_t sock, IN int onoff, IN int seconds); // linger
static int socket_getlinger(IN socket_t sock, OUT int* onoff, OUT int* seconds);
static int socket_setsendbuf(IN socket_t sock, IN size_t size); // send buf
static int socket_getsendbuf(IN socket_t sock, OUT size_t* size);
static int socket_setrecvbuf(IN socket_t sock, IN size_t size); // recv buf
static int socket_getrecvbuf(IN socket_t sock, OUT size_t* size);
static int socket_setsendtimeout(IN socket_t sock, IN size_t seconds); // send timeout
static int socket_getsendtimeout(IN socket_t sock, OUT size_t* seconds);
static int socket_setrecvtimeout(IN socket_t sock, IN size_t seconds); // recv timeout
static int socket_getrecvtimeout(IN socket_t sock, OUT size_t* seconds);
static int socket_setreuseaddr(IN socket_t sock, IN int enable); // reuse addr. 
static int socket_getreuseaddr(IN socket_t sock, OUT int* enable);
static int socket_setipv6only(IN socket_t sock, IN int ipv6_only); // 1-ipv6 only, 0-both ipv4 and ipv6
static int socket_getdomain(IN socket_t sock, OUT int* domain); // get socket protocol address family(sock don't need bind)

// socket status
// @return 0-ok, <0-socket_error(by socket_geterror())
static int socket_setnonblock(IN socket_t sock, IN int noblock); // non-block io, 0-block, 1-nonblock
static int socket_setnondelay(IN socket_t sock, IN int nodelay); // non-delay io(Nagle Algorithm), 0-delay, 1-nodelay
static int socket_getunread(IN socket_t sock, OUT size_t* size); // MSDN: Use to determine the amount of data pending in the network's input buffer that can be read from socket s

static int socket_getname(IN socket_t sock, OUT char ip[SOCKET_ADDRLEN], OUT u_short* port); // must be bound/connected
static int socket_getpeername(IN socket_t sock, OUT char ip[SOCKET_ADDRLEN], OUT u_short* port);

// socket utility
static int socket_isip(IN const char* ip); // socket_isip("192.168.1.2") -> 0, socket_isip("www.google.com") -> -1
static int socket_ipv4(IN const char* ipv4_or_dns, OUT char ip[SOCKET_ADDRLEN]);
static int socket_ipv6(IN const char* ipv6_or_dns, OUT char ip[SOCKET_ADDRLEN]);

static int socket_addr_from_ipv4(OUT struct sockaddr_in* addr4, IN const char* ip_or_dns, IN u_short port);
static int socket_addr_from_ipv6(OUT struct sockaddr_in6* addr6, IN const char* ip_or_dns, IN u_short port);
static int socket_addr_from(OUT struct sockaddr_storage* ss, OUT socklen_t* len, IN const char* ipv4_or_ipv6_or_dns, IN u_short port);
static int socket_addr_to(IN const struct sockaddr* sa, IN socklen_t salen, OUT char ip[SOCKET_ADDRLEN], OUT u_short* port);
static int socket_addr_name(IN const struct sockaddr* sa, IN socklen_t salen, OUT char* host, IN size_t hostlen);
static int socket_addr_setport(IN struct sockaddr* sa, IN socklen_t salen, u_short port);
static int socket_addr_is_multicast(IN const struct sockaddr* sa, IN socklen_t salen);

static void socket_setbufvec(INOUT socket_bufvec_t* vec, IN int idx, IN void* ptr, IN size_t len);
static void socket_getbufvec(IN const socket_bufvec_t* vec, IN int idx, OUT void** ptr, OUT size_t* len);

// multicast
static int socket_multicast_join(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local);
static int socket_multicast_leave(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local);
static int socket_multicast_join_source(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local);
static int socket_multicast_leave_source(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local);

//////////////////////////////////////////////////////////////////////////
///
/// socket create/close 
/// 
//////////////////////////////////////////////////////////////////////////
static int socket_init(void)
{
#if defined(OS_WINDOWS)
	WORD wVersionRequested;
	WSADATA wsaData;
	
	wVersionRequested = MAKEWORD(2, 2);
	return WSAStartup(wVersionRequested, &wsaData);
#else
	return 0;
#endif
}

static int socket_cleanup(void)
{
#if defined(OS_WINDOWS)
	return WSACleanup();
#else
	return 0;
#endif
}

static int socket_geterror(void)
{
#if defined(OS_WINDOWS)
	return WSAGetLastError();
#else
	return errno;
#endif
}

static socket_t socket_tcp(void)
{
	return socket(PF_INET, SOCK_STREAM, 0);
}

static socket_t socket_udp(void)
{
	return socket(PF_INET, SOCK_DGRAM, 0);
}

static socket_t socket_raw(void)
{
	return socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
}

static socket_t socket_rdm(void)
{
	return socket(PF_INET, SOCK_RDM, 0);
}

static socket_t socket_tcp_ipv6(void)
{
	return socket(PF_INET6, SOCK_STREAM, 0);
}

static socket_t socket_udp_ipv6(void)
{
	return socket(PF_INET6, SOCK_DGRAM, 0);
}

static socket_t socket_raw_ipv6(void)
{
	return socket(PF_INET6, SOCK_RAW, IPPROTO_RAW);
}

static int socket_shutdown(socket_t sock, int flag)
{
	return shutdown(sock, flag);
}

static int socket_close(socket_t sock)
{
#if defined(OS_WINDOWS)
	// MSDN:
	// If closesocket fails with WSAEWOULDBLOCK the socket handle is still valid, 
	// and a disconnect is not initiated. The application must call closesocket again to close the socket. 
	return closesocket(sock);
#else
	return close(sock);
#endif
}


//////////////////////////////////////////////////////////////////////////
///
/// socket operation
/// 
//////////////////////////////////////////////////////////////////////////
static int socket_connect(IN socket_t sock, IN const struct sockaddr* addr, IN socklen_t addrlen)
{
	return connect(sock, addr, addrlen);
}

static int socket_bind(IN socket_t sock, IN const struct sockaddr* addr, IN socklen_t addrlen)
{
	return bind(sock, addr, addrlen);
}

static int socket_listen(IN socket_t sock, IN int backlog)
{
	return listen(sock, backlog);
}

static socket_t socket_accept(IN socket_t sock, OUT struct sockaddr_storage* addr, OUT socklen_t* addrlen)
{
	*addrlen = sizeof(struct sockaddr_storage);
	return accept(sock, (struct sockaddr*)addr, addrlen);
}

//////////////////////////////////////////////////////////////////////////
///
/// socket read/write
/// 
//////////////////////////////////////////////////////////////////////////
static int socket_send(IN socket_t sock, IN const void* buf, IN size_t len, IN int flags)
{
#if defined(OS_WINDOWS)
	return send(sock, (const char*)buf, (int)len, flags);
#else
	return send(sock, buf, len, flags);
#endif
}

static int socket_recv(IN socket_t sock, OUT void* buf, IN size_t len, IN int flags)
{
#if defined(OS_WINDOWS)
	return recv(sock, (char*)buf, (int)len, flags);
#else
	return recv(sock, buf, len, flags);
#endif
}

static int socket_sendto(IN socket_t sock, IN const void* buf, IN size_t len, IN int flags, IN const struct sockaddr* to, IN socklen_t tolen)
{
#if defined(OS_WINDOWS)
	return sendto(sock, (const char*)buf, (int)len, flags, to, tolen);
#else
	return sendto(sock, buf, len, flags, to, tolen);
#endif
}

static int socket_recvfrom(IN socket_t sock, OUT void* buf, IN size_t len, IN int flags, OUT struct sockaddr* from, OUT socklen_t* fromlen)
{
#if defined(OS_WINDOWS)
	return recvfrom(sock, (char*)buf, (int)len, flags, from, fromlen);
#else
	return recvfrom(sock, buf, len, flags, from, fromlen);
#endif
}

static int socket_send_v(IN socket_t sock, IN const socket_bufvec_t* vec, IN size_t n, IN int flags)
{
#if defined(OS_WINDOWS)
	DWORD count = 0;
	int r = WSASend(sock, (socket_bufvec_t*)vec, (DWORD)n, &count, flags, NULL, NULL);
	return 0 == r ? (int)count : r;
#else
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = (struct iovec*)vec;
	msg.msg_iovlen = n;
	return sendmsg(sock, &msg, flags);
#endif
}

static int socket_recv_v(IN socket_t sock, IN socket_bufvec_t* vec, IN size_t n, IN int flags)
{
#if defined(OS_WINDOWS)
	DWORD count = 0;
	int r = WSARecv(sock, vec, (DWORD)n, &count, (LPDWORD)&flags, NULL, NULL);
	return 0 == r ? (int)count : r;
#else
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = vec;
	msg.msg_iovlen = n;
	return recvmsg(sock, &msg, flags);
#endif
}

static int socket_sendto_v(IN socket_t sock, IN const socket_bufvec_t* vec, IN size_t n, IN int flags, IN const struct sockaddr* to, IN socklen_t tolen)
{
#if defined(OS_WINDOWS)
	DWORD count = 0;
	int r = WSASendTo(sock, (socket_bufvec_t*)vec, (DWORD)n, &count, flags, to, tolen, NULL, NULL);
	return 0 == r ? (int)count : r;
#else
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (struct sockaddr*)to;
	msg.msg_namelen = tolen;
	msg.msg_iov = (struct iovec*)vec;
	msg.msg_iovlen = n;
	return sendmsg(sock, &msg, flags);
#endif
}

static int socket_recvfrom_v(IN socket_t sock, IN socket_bufvec_t* vec, IN size_t n, IN int flags, IN struct sockaddr* from, IN socklen_t* fromlen)
{
#if defined(OS_WINDOWS)
	DWORD count = 0;
	int r = WSARecvFrom(sock, vec, (DWORD)n, &count, (LPDWORD)&flags, from, fromlen, NULL, NULL);
	return 0 == r ? (int)count : r;
#else
	struct msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = from;
	msg.msg_namelen = *fromlen;
	msg.msg_iov = vec;
	msg.msg_iovlen = n;
	return recvmsg(sock, &msg, flags);
#endif
}

static int socket_select(IN int n, IN fd_set* rfds, IN fd_set* wfds, IN fd_set* efds, IN struct timeval* timeout)
{
#if defined(OS_WINDOWS)
	return select(n, rfds, wfds, efds, timeout);
#else
	int r = select(n, rfds, wfds, efds, timeout);
	while(-1 == r && EINTR == errno)
		r = select(n, rfds, wfds, efds, timeout);
	return r;
#endif
}

static int socket_select_readfds(IN int n, IN fd_set* fds, IN struct timeval* timeout)
{
	return socket_select(n, fds, NULL, NULL, timeout);
}

static int socket_select_writefds(IN int n, IN fd_set* fds, IN struct timeval* timeout)
{
	return socket_select(n, NULL, fds, NULL, timeout);
}

static int socket_select_read(IN socket_t sock, IN int timeout)
{
#if defined(OS_WINDOWS)
	fd_set fds;
	struct timeval tv;
	assert(socket_invalid != sock); // linux: FD_SET error
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;
	return socket_select_readfds(sock+1, &fds, timeout<0?NULL:&tv);
#else
	int r;
	struct pollfd fds;

	fds.fd = sock;
	fds.events = POLLIN;
	fds.revents = 0;

	r = poll(&fds, 1, timeout);
	while(-1 == r && EINTR == errno)
		r = poll(&fds, 1, timeout);
	return r;
#endif
}

static int socket_select_write(IN socket_t sock, IN int timeout)
{
#if defined(OS_WINDOWS)
	fd_set fds;
	struct timeval tv;

	assert(socket_invalid != sock); // linux: FD_SET error

	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	tv.tv_sec = timeout/1000;
	tv.tv_usec = (timeout%1000) * 1000;
	return socket_select_writefds(sock+1, &fds, timeout<0?NULL:&tv);
#else
	int r;
	struct pollfd fds;

	fds.fd = sock;
	fds.events = POLLOUT;
	fds.revents = 0;

	r = poll(&fds, 1, timeout);
	while(-1 == r && EINTR == errno)
		r = poll(&fds, 1, timeout);
	return r;
#endif
}

static int socket_readable(IN socket_t sock)
{
	return socket_select_read(sock, 0);
}

static int socket_writeable(IN socket_t sock)
{
	return socket_select_write(sock, 0);
}

//////////////////////////////////////////////////////////////////////////
///
/// socket options
/// 
//////////////////////////////////////////////////////////////////////////
static int socket_setopt_bool(IN socket_t sock, IN int optname, IN int enable)
{
#if defined(OS_WINDOWS)
	BOOL v = enable ? TRUE : FALSE;
	return setsockopt(sock, SOL_SOCKET, optname, (const char*)&v, sizeof(v));
#else
	return setsockopt(sock, SOL_SOCKET, optname, &enable, sizeof(enable));
#endif
}

static int socket_getopt_bool(IN socket_t sock, IN int optname, OUT int* enable)
{
	socklen_t len;
#if defined(OS_WINDOWS)
	int r;
	BOOL v;
	len = sizeof(v);
	r = getsockopt(sock, SOL_SOCKET, optname, (char*)&v, &len);
	if(0 == r)
		*enable = (TRUE==v)?1:0;
	return r;
#else
	len = sizeof(*enable);
	return getsockopt(sock, SOL_SOCKET, optname, enable, &len);
#endif
}

static int socket_setkeepalive(IN socket_t sock, IN int enable)
{
	return socket_setopt_bool(sock, SO_KEEPALIVE, enable);
}

static int socket_getkeepalive(IN socket_t sock, OUT int* enable)
{
	return socket_getopt_bool(sock, SO_KEEPALIVE, enable);
}

static int socket_setlinger(IN socket_t sock, IN int onoff, IN int seconds)
{
	struct linger l;
	l.l_onoff = (u_short)onoff;
	l.l_linger = (u_short)seconds;
	return setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l));
}

static int socket_getlinger(IN socket_t sock, OUT int* onoff, OUT int* seconds)
{
	int r;
	socklen_t len;
	struct linger l;
	
	len = sizeof(l);
	r = getsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&l, &len);
	if(0 == r)
	{
		*onoff = l.l_onoff;
		*seconds = l.l_linger;
	}
	return r;
}

static int socket_setsendbuf(IN socket_t sock, IN size_t size)
{
	return setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size));
}

static int socket_getsendbuf(IN socket_t sock, OUT size_t* size)
{
	socklen_t len;
	len = sizeof(*size);
	return getsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)size, &len);
}

static int socket_setrecvbuf(IN socket_t sock, IN size_t size)
{
	return setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size));
}

static int socket_getrecvbuf(IN socket_t sock, OUT size_t* size)
{
	socklen_t len;
	len = sizeof(*size);
	return getsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)size, &len);
}

static int socket_setsendtimeout(IN socket_t sock, IN size_t seconds)
{
#if defined(OS_WINDOWS)
	return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&seconds, sizeof(seconds));
#else
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	return setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

static int socket_getsendtimeout(IN socket_t sock, OUT size_t* seconds)
{
	socklen_t len;
#if defined(OS_WINDOWS)
	len = sizeof(*seconds);
	return getsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)seconds, &len);
#else
	int r;
	struct timeval tv;
	len = sizeof(tv);
	r = getsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, &len);
	if(0 == r)
		*seconds = tv.tv_sec;
	return r;
#endif
}

static int socket_setrecvtimeout(IN socket_t sock, IN size_t seconds)
{
#if defined(OS_WINDOWS)
	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&seconds, sizeof(seconds));
#else
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = 0;
	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

static int socket_getrecvtimeout(IN socket_t sock, OUT size_t* seconds)
{
	socklen_t len;
#if defined(OS_WINDOWS)
	len = sizeof(*seconds);
	return getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)seconds, &len);
#else
	int r;
	struct timeval tv;
	len = sizeof(tv);
	r = getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, &len);
	if(0 == r)
		*seconds = tv.tv_sec;
	return r;
#endif
}

static int socket_setreuseaddr(IN socket_t sock, IN int enable)
{
	return socket_setopt_bool(sock, SO_REUSEADDR, enable);
}

static int socket_getreuseaddr(IN socket_t sock, OUT int* enable)
{
	return socket_getopt_bool(sock, SO_REUSEADDR, enable);
}

static int socket_setnonblock(IN socket_t sock, IN int noblock)
{
	// 0-block, 1-no-block
#if defined(OS_WINDOWS)
	return ioctlsocket(sock, FIONBIO, (u_long*)&noblock);
#else
	// http://stackoverflow.com/questions/1150635/unix-nonblocking-i-o-o-nonblock-vs-fionbio
	// Prior to standardization there was ioctl(...FIONBIO...) and fcntl(...O_NDELAY...) ...
	// POSIX addressed this with the introduction of O_NONBLOCK.
	int flags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, noblock ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
	//return ioctl(sock, FIONBIO, &noblock);
#endif
}

static int socket_setnondelay(IN socket_t sock, IN int nodelay)
{
	// 0-delay(enable the Nagle algorithm)
	// 1-no-delay(disable the Nagle algorithm)
	// http://linux.die.net/man/7/tcp
	return setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&nodelay, sizeof(nodelay));
}

static int socket_getunread(IN socket_t sock, OUT size_t* size)
{
#if defined(OS_WINDOWS)
	return ioctlsocket(sock, FIONREAD, (u_long*)size);
#else
	return ioctl(sock, FIONREAD, (int*)size);
#endif
}

static int socket_setipv6only(IN socket_t sock, IN int ipv6_only)
{
	// Windows Vista or later: default 1
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms738574%28v=vs.85%29.aspx
	// Linux 2.4.21 and 2.6: /proc/sys/net/ipv6/bindv6only defalut 0
	// http://www.man7.org/linux/man-pages/man7/ipv6.7.html
	return setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&ipv6_only, sizeof(ipv6_only));
}

static int socket_getdomain(IN socket_t sock, OUT int* domain)
{
	int r;
#if defined(OS_WINDOWS)
	WSAPROTOCOL_INFOW protocolInfo;
	int len = sizeof(protocolInfo);
	r = getsockopt(sock, SOL_SOCKET, SO_PROTOCOL_INFOW, (char*)&protocolInfo, &len);
	if (0 == r)
		*domain = protocolInfo.iAddressFamily;
#else
	socklen_t len = sizeof(domain);
	r = getsockopt(sock, SOL_SOCKET, SO_DOMAIN, (char*)domain, &len);
#endif
	return r;
}

// must be bound/connected
static int socket_getname(IN socket_t sock, OUT char ip[SOCKET_ADDRLEN], OUT u_short* port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	if(socket_error == getsockname(sock, (struct sockaddr*)&addr, &addrlen))
		return socket_error;

	return socket_addr_to((struct sockaddr*)&addr, addrlen, ip, port);
}

static int socket_getpeername(IN socket_t sock, OUT char ip[SOCKET_ADDRLEN], OUT u_short* port)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);
	if(socket_error == getpeername(sock, (struct sockaddr*)&addr, &addrlen))
		return socket_error;
	
	return socket_addr_to((struct sockaddr*)&addr, addrlen, ip, port);
}

static int socket_isip(IN const char* ip)
{
#if 1
	struct sockaddr_storage addr;
	if(1 != inet_pton(AF_INET, ip, &((struct sockaddr_in*)&addr)->sin_addr) 
		&& 1 != inet_pton(AF_INET6, ip, &((struct sockaddr_in6*)&addr)->sin6_addr))
		return -1;
	return 0;
#else
	struct addrinfo hints, *addr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_NUMERICHOST | AI_V4MAPPED | AI_ADDRCONFIG;
	if (0 != getaddrinfo(ip, NULL, &hints, &addr))
		return -1;
	freeaddrinfo(&addr);
#endif
	return 0;
}

static int socket_ipv4(IN const char* ipv4_or_dns, OUT char ip[SOCKET_ADDRLEN])
{
	int r;
	struct addrinfo hints, *addr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
//	hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
	r = getaddrinfo(ipv4_or_dns, NULL, &hints, &addr);
	if (0 != r)
		return r;

	assert(AF_INET == addr->ai_family);
	inet_ntop(AF_INET, &(((struct sockaddr_in*)addr->ai_addr)->sin_addr), ip, SOCKET_ADDRLEN);
	freeaddrinfo(addr);
	return 0;
}

static int socket_ipv6(IN const char* ipv6_or_dns, OUT char ip[SOCKET_ADDRLEN])
{
	int r;
	struct addrinfo hints, *addr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
//	hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
	r = getaddrinfo(ipv6_or_dns, NULL, &hints, &addr);
	if (0 != r)
		return r;

	assert(AF_INET6 == addr->ai_family);
	inet_ntop(AF_INET6, &(((struct sockaddr_in6*)addr->ai_addr)->sin6_addr), ip, SOCKET_ADDRLEN);
	freeaddrinfo(addr);
	return 0;
}

static int socket_addr_from_ipv4(OUT struct sockaddr_in* addr4, IN const char* ipv4_or_dns, IN u_short port)
{
	int r;
	char portstr[16];
	struct addrinfo hints, *addr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	//hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
	snprintf(portstr, sizeof(portstr), "%hu", port);
	r = getaddrinfo(ipv4_or_dns, portstr, &hints, &addr);
	if (0 != r)
		return r;

	socket_addr_setport(addr->ai_addr, addr->ai_addrlen, port); // fixed ios getaddrinfo don't set port if node is ipv4 address
	assert(sizeof(struct sockaddr_in) == addr->ai_addrlen);
	memcpy(addr4, addr->ai_addr, addr->ai_addrlen);
	freeaddrinfo(addr);
	return 0;
}

static int socket_addr_from_ipv6(OUT struct sockaddr_in6* addr6, IN const char* ipv6_or_dns, IN u_short port)
{
	int r;
	char portstr[16];
	struct addrinfo hints, *addr;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	//hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG; // AI_ADDRCONFIG linux "ff00::" return -2
	snprintf(portstr, sizeof(portstr), "%hu", port);
	r = getaddrinfo(ipv6_or_dns, portstr, &hints, &addr);
	if (0 != r)
		return r;

	socket_addr_setport(addr->ai_addr, addr->ai_addrlen, port); // fixed ios getaddrinfo don't set port if node is ipv4 address
	assert(sizeof(struct sockaddr_in6) == addr->ai_addrlen);
	memcpy(addr6, addr->ai_addr, addr->ai_addrlen);
	freeaddrinfo(addr);
	return 0;
}

static int socket_addr_from(OUT struct sockaddr_storage* ss, OUT socklen_t* len, IN const char* ipv4_or_ipv6_or_dns, IN u_short port)
{
	int r;
	char portstr[16];
	struct addrinfo *addr;
	snprintf(portstr, sizeof(portstr), "%hu", port);
	r = getaddrinfo(ipv4_or_ipv6_or_dns, portstr, NULL, &addr);
	if (0 != r)
		return r;

	socket_addr_setport(addr->ai_addr, addr->ai_addrlen, port); // fixed ios getaddrinfo don't set port if node is ipv4 address
	assert(addr->ai_addrlen <= sizeof(struct sockaddr_storage));
	memcpy(ss, addr->ai_addr, addr->ai_addrlen);
	*len = addr->ai_addrlen;
	freeaddrinfo(addr);
	return 0;
}

static int socket_addr_to(IN const struct sockaddr* sa, IN socklen_t salen, OUT char ip[SOCKET_ADDRLEN], OUT u_short* port)
{
	if (AF_INET == sa->sa_family)
	{
		struct sockaddr_in* in = (struct sockaddr_in*)sa;
		assert(sizeof(struct sockaddr_in) == salen);
		inet_ntop(AF_INET, &in->sin_addr, ip, SOCKET_ADDRLEN);
		*port = ntohs(in->sin_port);
	}
	else if (AF_INET6 == sa->sa_family)
	{
		struct sockaddr_in6* in6 = (struct sockaddr_in6*)sa;
		assert(sizeof(struct sockaddr_in6) == salen);
		inet_ntop(AF_INET6, &in6->sin6_addr, ip, SOCKET_ADDRLEN);
		*port = ntohs(in6->sin6_port);
	}
	else
	{
		return -1; // unknown address family
	}

	return 0;
}

static int socket_addr_setport(IN struct sockaddr* sa, IN socklen_t salen, u_short port)
{
	if (AF_INET == sa->sa_family)
	{
		struct sockaddr_in* in = (struct sockaddr_in*)sa;
		assert(sizeof(struct sockaddr_in) == salen);
		in->sin_port = in->sin_port ? in->sin_port : htons(port);
	}
	else if (AF_INET6 == sa->sa_family)
	{
		struct sockaddr_in6* in6 = (struct sockaddr_in6*)sa;
		assert(sizeof(struct sockaddr_in6) == salen);
		in6->sin6_port = in6->sin6_port ? in6->sin6_port : htons(port);
	}
	else
	{
		assert(0);
		return -1;
	}

	return 0;
}

static int socket_addr_name(IN const struct sockaddr* sa, IN socklen_t salen, OUT char* host, IN size_t hostlen)
{
	return getnameinfo(sa, salen, host, hostlen, NULL, 0, 0);
}

static int socket_addr_is_multicast(IN const struct sockaddr* sa, IN socklen_t salen)
{
	if (AF_INET == sa->sa_family)
	{
		const struct sockaddr_in* in = (const struct sockaddr_in*)sa;
		assert(sizeof(struct sockaddr_in) == salen);
		return (ntohl(in->sin_addr.s_addr) & 0xf0000000) == 0xe0000000 ? 1 : 0;
	}
	else if (AF_INET6 == sa->sa_family)
	{
		const struct sockaddr_in6* in6 = (const struct sockaddr_in6*)sa;
		assert(sizeof(struct sockaddr_in6) == salen);
		return in6->sin6_addr.s6_addr[0] == 0xff ? 1 : 0;
	}
	else
	{
		assert(0);
	}

	return 0;
}

static void socket_setbufvec(INOUT socket_bufvec_t* vec, IN int idx, IN void* ptr, IN size_t len)
{
#if defined(OS_WINDOWS)
	vec[idx].buf = (CHAR*)ptr;
	vec[idx].len = (ULONG)len;
#else
	vec[idx].iov_base = ptr;
	vec[idx].iov_len = len;
#endif
}

static void socket_getbufvec(IN const socket_bufvec_t* vec, IN int idx, OUT void** ptr, OUT size_t* len)
{
#if defined(OS_WINDOWS)
	*ptr = (void*)vec[idx].buf;
	*len = (size_t)vec[idx].len;
#else
	*ptr = vec[idx].iov_base;
	*len = vec[idx].iov_len;
#endif
}

static int socket_multicast_join(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local)
{
	struct ip_mreq_source imr;
	memset(&imr, 0, sizeof(imr));
	inet_pton(AF_INET, source, &imr.imr_sourceaddr);
	inet_pton(AF_INET, group, &imr.imr_multiaddr);
	inet_pton(AF_INET, local, &imr.imr_interface);
	return setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &imr, sizeof(imr));
}

static int socket_multicast_leave(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local)
{
	struct ip_mreq_source imr;
	memset(&imr, 0, sizeof(imr));
	inet_pton(AF_INET, source, &imr.imr_sourceaddr);
	inet_pton(AF_INET, group, &imr.imr_multiaddr);
	inet_pton(AF_INET, local, &imr.imr_interface);
	return setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *) &imr, sizeof(imr));
}

static int socket_multicast_join_source(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local)
{
	struct ip_mreq_source imr;
	memset(&imr, 0, sizeof(imr));
	inet_pton(AF_INET, source, &imr.imr_sourceaddr);
	inet_pton(AF_INET, group, &imr.imr_multiaddr);
	inet_pton(AF_INET, local, &imr.imr_interface);
	return setsockopt(sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (char *) &imr, sizeof(imr));
}

static int socket_multicast_leave_source(IN socket_t sock, IN const char* group, IN const char* source, IN const char* local)
{
	struct ip_mreq_source imr;
	memset(&imr, 0, sizeof(imr));
	inet_pton(AF_INET, source, &imr.imr_sourceaddr);
	inet_pton(AF_INET, group, &imr.imr_multiaddr);
	inet_pton(AF_INET, local, &imr.imr_interface);
	return setsockopt(sock, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP, (char *)&imr, sizeof(imr));
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif /* !_platform_socket_h_ */
