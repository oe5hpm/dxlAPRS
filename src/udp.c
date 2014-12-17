/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>



int openudp()
{
return socket(PF_INET, SOCK_DGRAM, 0);
}

int bindudp(int fd, unsigned port)
{
struct sockaddr_in addr;

memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port=htons(port);
addr.sin_addr.s_addr=0;   /* =inet_addr(ipnum); */
return bind(fd, (struct sockaddr *)&addr, sizeof(addr));
}

int socknonblock(int fd)
{
  int flags=fcntl(fd, F_GETFL, 0);
  if (flags<0) return -1;
  flags|=O_NONBLOCK;
  return fcntl(fd, F_SETFL, flags);
}


int udpreceive(int fd, char *buf, int blen, int *fromport, int *fromip)
{
int adrlen, len;
struct sockaddr_in addr;

adrlen=sizeof(addr);
len=recvfrom(fd, buf, blen, MSG_DONTWAIT, (struct sockaddr *) &addr, &adrlen);
*fromport=ntohs(addr.sin_port);
*fromip=ntohl(addr.sin_addr.s_addr);
return len;
}

int udpreceiveblock(int fd, char *buf, int blen, int *fromport, int *fromip)
{
int adrlen, len;
struct sockaddr_in addr;

adrlen=sizeof(addr);
len=recvfrom(fd, buf, blen, 0, (struct sockaddr *) &addr, &adrlen);
*fromport=ntohs(addr.sin_port);
*fromip=ntohl(addr.sin_addr.s_addr);
return len;
}

int udpsend(int fd, char *buf, int blen, int toport, int toip)
{
struct sockaddr_in addr;

memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_port=htons(toport);
addr.sin_addr.s_addr=htonl(toip);
return sendto(fd, buf, blen, 0, (struct sockaddr *) &addr, sizeof(addr));
}
