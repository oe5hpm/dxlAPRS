/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
//#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/tcp.h>

#ifndef MSG_MORE
#define MSG_MORE 0
#endif
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
//--------------------

int resolv(char *url, uint32_t *host)
{
  struct hostent *h;

  h=gethostbyname(url);
  if ((h==0) || (h->h_addr_list==0)) return -1;
  memcpy(host, *(h->h_addr_list), 4);
  return 0;
}

//--------------------

int opentcp()
{
return socket(PF_INET, SOCK_STREAM, 0);
}


int tcpconnect(int fd, int toport, int toip)
{
//struct sockaddr addr;

//memset(&addr, 0, sizeof(addr));
//addr.sa_family = AF_INET;
//addr.sa_port=htons(toport);
//addr.sa_addr.s_addr=htonl(toip);
//return sendto(fd, buf, blen, 0, (struct sockaddr *) &addr, sizeof(addr));
}

//--------------------

/*
int connectto1(char *url, char *port)
{
  struct gaicb req;
  struct addrinfo *rp;
  int sockfd;

  memset(&req, 0, sizeof req);
  req.ar_name = url;
  if (getaddrinfo_a(GAI_WAIT, &req, 1, NULL) != 0) return -1;

usleep(5000000);
  if (gai_error(&req != 0)) return -1;

  for (rp = req.ar_result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd<0) continue;

    int flags=fcntl(sockfd, F_GETFL, 0);
    if (flags<0) return -1;

    flags|=O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) < 0) break;

    close(sockfd);
  }
  freeaddrinfo(req.ar_result);
  if (rp = NULL) return -1;

  return sockfd;  
}
*/
//--------------------


int connectto(char *url, char *port)
{
  struct addrinfo hints, *res, *rp;
  int sockfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(url, port, &hints, &res) != 0) return -1;
//  if (res == 0) return -1;
  for (rp = res; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd<0) continue;

//nodelay 
    int flag_TCP_nodelay = 1;
    if ( (setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY,
        (char *)&flag_TCP_nodelay, sizeof(flag_TCP_nodelay))) < 0) return -1;
//nodelay

    int flags=fcntl(sockfd, F_GETFL, 0);
    if (flags<0) return -1;

    flags|=O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) < 0) break;

    close(sockfd);
  }
  freeaddrinfo(res);
  if (rp == NULL)
	return -1;

  return sockfd;
}

//--------------------

int waitconnect(char *port, int maxconn)
{
struct addrinfo hints, *res;
int sockfd;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
//  hints.ai_family = AF_INET6;  // use IPv6
//  hints.ai_family = AF_INET;  // use IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

  getaddrinfo(NULL, port, &hints, &res);
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd<0) return -1;

  if (bind(sockfd, res->ai_addr, res->ai_addrlen)<0) {close(sockfd); return -1;}
  listen(sockfd, maxconn);
  return sockfd;
}

//---------------------

int acceptconnect(int fd, char *addr, uint32_t *len)
{
  return accept(fd, (struct sockaddr *)addr, len);
}

void ipnum2str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
    }
}

//---------------------

void ipnumport2str(const struct sockaddr *sa, socklen_t salen, char *ip, size_t maxiplen, char *port, size_t maxportlen)
{
  getnameinfo(sa, salen, ip, maxiplen, port, maxportlen, NI_NUMERICHOST|NI_NUMERICSERV);
}


//--------------------

int sendmore(int fd, char *buf, int len)
{
  return send(fd, buf, len, MSG_NOSIGNAL | MSG_MORE);
}

//--------------------

int sendsock(int fd, char *buf, int len)
{
  return send(fd, buf, len, MSG_NOSIGNAL);
}

//--------------------

int readsock(int fd, char *buf, int len)
{
  int res=recv(fd, buf, len, MSG_DONTWAIT);
  if (res==0) return -1;
  if ((res<0) && (errno==EAGAIN)) return 0;
  return res;
}


//--------------------

int getsockipnum(int fd, char *addr, uint32_t *len)
{
  return getsockname(fd, (struct sockaddr *)addr, len);
}

//--------------------

int getpeeripnum(int fd, char *addr, uint32_t *len)
{
  return getpeername(fd, (struct sockaddr *)addr, len);
}

//--------------------

int getunack(int fd)
{
  int unak=-1;
  ioctl(fd, TIOCOUTQ, &unak);
  return unak; 
}

//--------------------

void stoptxrx(int fd, int how)
// how 0 rx, 1 tx, 2 rxtx
{
  shutdown(fd, how);
}

