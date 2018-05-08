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

//--------------------




int32_t connecttob(char *url, char *port)
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

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) < 0)
{
    int flags=fcntl(sockfd, F_GETFL, 0);
    if (flags<0) return -1;

    flags&=~O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

 break;
}

    close(sockfd);
  }
  freeaddrinfo(res);
  if (rp = NULL) return -1;

  return sockfd;  
}


int32_t readsockb(int fd, char *buf, int len)
{
  int res=recv(fd, buf, len, MSG_WAITALL);
  if (res==0) return -1;
  if ((res<0) && (errno==EAGAIN)) return 0;
  return res;
}


