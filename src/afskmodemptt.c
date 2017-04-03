/*
 * afskmodem-ptt.c
 * This module handles PTT of several modems on different hw-interfaces
 *
 * Copyright (C) 2014 Hannes Petermaier <oe5hpm@oe5xbl.#oe5.aut.eu, oe5hpm@oevsv.at>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#ifndef MACOS
#include <linux/ppdev.h>
#endif
#include <unistd.h>

/*#define _DEBUG*/

#ifdef _DEBUG
# define DBG(...)		printf(__VA_ARGS__)
#else
# define DBG(...)
#endif

#ifndef MACOS

struct ttydev_t {
	char			*name;		/* name of tty */
	int			fd;		/* responsible fd of tty */
};

struct pttcmn_t {
	struct ttydev_t		ttydevs[16];	/* ttyDevs, since we use last
						 * entry for "end-detection"
						 * this entry always must beu
						 * zero
						 */
};

struct ptt_t {
	struct pttcmn_t		*pcmn;		/* pointer to common instance */
	char			devname[1024];	/* name of the device */
	int			bit;		/* bit within device */
	unsigned int		laststate;	/* last written state from modem */
	unsigned int		origportstate;	/* Portstate on open, used to restore
	 	 	 	 		 * device-state upon exit
	 	 	 	 		 */
	int			ttyrelease;	/* close tty after every switch-cycle */
	int			fd;		/* handle of ptt-device */
	int			(*switchfct)(struct ptt_t *pInst, int value);
	int			(*destroyfct)(struct ptt_t *pInst);
};

struct pttcmn_t			*gpcmn = NULL;

static int tty_search_byname(struct pttcmn_t *pInst, char *name)
{
	struct ttydev_t *ptty = pInst->ttydevs;
	while (ptty->fd != 0) {
		if (strcmp(ptty->name, name) == 0) {
			return ptty->fd;
		}
		ptty++;
	}
	return -1;
}

static int tty_register(struct pttcmn_t *pInst, char *name, int fd)
{
	unsigned int i;
	struct ttydev_t *ptty = pInst->ttydevs;

	for (i=0; i<(sizeof(pInst->ttydevs)/sizeof(pInst->ttydevs[0])-1); i++) {
		if (ptty->fd == 0) {
			ptty->fd = fd;
			ptty->name = name;
			break;
		}
		ptty++;
	}
	if (i<(sizeof(pInst->ttydevs)/sizeof(pInst->ttydevs[0]))-1)
		return 0;

	return -1;
}

static int ptt_parport(struct ptt_t *pInst, int value)
{
	int switchval = (pInst->bit > 0 ? 0 : 1) ^ value;
	int fd;
	int rc = -1;

	unsigned short port;

	fd = open(pInst->devname, O_WRONLY);
	if (fd < 0 || ioctl(fd, PPCLAIM) < 0) {
		printf("cannot open %s!\n", pInst->devname);
	} else {
		rc = ioctl(fd, PPRDATA, &port);

		if (switchval)
			port |= (0x01 << abs(pInst->bit)-1);
		else
			port &= ~(0x01 << abs(pInst->bit)-1);
		rc = ioctl(fd, PPWDATA, &port);
	}
	if (fd > 0) {
		ioctl(fd, PPRELEASE);
		close(fd);
	}

	return rc;
}

static int ptt_gpio(struct ptt_t *pInst, int value)
{
	int switchval = (pInst->bit > 0 ? 0 : 1) ^ value;
	int fd, rc;
	char buf[64];

	snprintf(buf, sizeof(buf),
			 "/sys/class/gpio/gpio%d/value", abs(pInst->bit)-1);

	fd = open(buf, O_WRONLY);
	if (fd > 0) {
		 if (switchval)
			 rc = write(fd, "1", 1);
		 else
			 rc = write(fd, "0", 1);

		close(fd);
		return rc;
	} else {
		return -1;
	}
}

static int ptt_gpioDestroy(struct ptt_t *pInst)
{
	int fd;
	char buf[64];

	/* turn ptt off *
	 * may not be the ultimative trick, because we afterwards reset the
	 * device into original state.
	 * Further kernel takes over control upon we've closed it.
	 */
	pInst->switchfct(pInst, 0);

	/* unexport the used gpio to give them to other users */
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd > 0) {
		snprintf(buf, sizeof(buf),
				 "%d", abs(pInst->bit)-1);
		write(fd, buf, strlen(buf));
		close(fd);
		return 0;
	}
	return -1;
}

static int ptt_tty(struct ptt_t *pInst, int value)
{
	int switchval = (pInst->bit > 0 ? 0 : 1) ^ value;
	int rc, flags, first = 0;
	int fd;

	if (pInst->fd <= 0) {
		fd = tty_search_byname(pInst->pcmn, pInst->devname);
		if (fd > 0) {
			pInst->fd = fd;
			DBG("%s: reuse allready open fd (%d) on %s\n",
			    __func__, pInst->fd, pInst->devname);
		} else {
			DBG("%s: try to open %s ...\n",
			    __func__, pInst->devname);
			fd = open(pInst->devname, O_RDONLY);
			if (fd > 0) {
				if (tty_register(pInst->pcmn,
						 pInst->devname,
						 fd) == 0
				) {
					pInst->fd = fd;
					first = 1;
				} else {
					close(fd);
				}
			}
		}
	}

	if (pInst->fd > 0) {
		if (first) {
			flags = TIOCM_RTS | TIOCM_DTR;
			switchval = 0;
		}
		else {
			flags = abs(pInst->bit)-1 == 0 ? TIOCM_RTS : TIOCM_DTR;
		}

		if (switchval)
			 rc = ioctl(pInst->fd, TIOCMBIS, &flags);
		else
			 rc = ioctl(pInst->fd, TIOCMBIC, &flags);

		if (pInst->ttyrelease || rc != 0) {
			pInst->destroyfct(pInst);
			pInst->fd = -1;
		}
		return rc;
	} else {
		return -1;
	}
}

static int ptt_ttyDestroy(struct ptt_t *pInst)
{
	struct ttydev_t *ptty = pInst->pcmn->ttydevs;
	int flags = TIOCM_RTS | TIOCM_DTR;

	while (ptty->fd > 0) {
		ioctl(ptty->fd, TIOCMBIC, &flags);
		close(ptty->fd);
		ptty++;
	}

	return 0;
}

void *pttinit(char *devname, int bit)
{
	struct ptt_t	*pInst;
	int fd;
	char buf[64];
	unsigned int tmp;
	int busycnt;

	if (gpcmn == NULL) {
		gpcmn = (struct pttcmn_t *)malloc(sizeof(struct pttcmn_t));
		if (gpcmn != NULL)
			memset(gpcmn, 0, sizeof(struct pttcmn_t));
		else {
			printf("%s: cannot allocate memory for common instance",
			       __func__);
			return 0;
		}
	}

	if ( (pInst = (struct ptt_t *)malloc(sizeof(struct ptt_t))) != 0) {
		memset(pInst, 0, sizeof(struct ptt_t));
		pInst->bit = bit;
		pInst->pcmn = gpcmn;
		strncpy(pInst->devname, devname, sizeof(pInst->devname));

		if (strstr(devname, "tty") != 0) {
			if (pInst->bit > 2 || pInst->bit < -2) {
				printf("fail: %s has only to switchable bits!\n",
				       pInst->devname);
				goto errorExit;
			}
			pInst->fd = -1;
			pInst->switchfct = &ptt_tty;
			pInst->destroyfct = &ptt_ttyDestroy;
		} else if (strcmp(devname, "gpio") == 0) {
			fd = open("/sys/class/gpio/export", O_WRONLY);
			if (fd > 0) {
				snprintf(buf, sizeof(buf),
					 "%d", abs(pInst->bit)-1);
				write(fd, buf, strlen(buf));
				close(fd);
			} else {
				goto errorExit;
			}
			snprintf(buf, sizeof(buf),
				 "/sys/class/gpio/gpio%d/direction",
				 abs(pInst->bit)-1);
			fd = open(buf, O_WRONLY);
			if (fd > 0) {
				write(fd, "out", 3);
				close(fd);
			} else {
				goto errorExit;
			}
			pInst->switchfct = &ptt_gpio;
			pInst->destroyfct = &ptt_gpioDestroy;
		} else if (strstr(devname, "parport") != 0) {
			if (pInst->bit > 8 || pInst->bit < -8) {
				printf("fail: parport has only 8 bits!\n");
				goto errorExit;
			}
			pInst->switchfct = &ptt_parport;
		} else {
			goto errorExit;
		}
		return pInst;
	}

	return 0;

	errorExit:
	printf("PTT setup on device %s failed.\n", pInst->devname);
	if (fd > 0)
		close(fd);
	free(pInst);

	return 0;
}

void pttDestroy(void *arg)
{
	struct ptt_t *pInst = (struct ptt_t *)arg;

	if (!pInst)
		return;

	if(pInst->destroyfct) {
		pInst->destroyfct(pInst);
	}
	free(pInst);
}

int ptt(void *arg, unsigned int val)
{
	struct ptt_t *pInst = (struct ptt_t *)arg;

	if (!pInst)
		return -1;

	if(pInst->switchfct) {
		if (val != -1) {
			pInst->laststate = val;
			DBG("ptt (0x%08x) switch to %d\n",
			    (unsigned int)pInst, val);
			return pInst->switchfct(pInst, val);
		} else {
			DBG("ptt (0x%08x) switch to %d (cyclic)\n",
			    (unsigned int)pInst, pInst->laststate);
			return pInst->switchfct(pInst, pInst->laststate);
		}
	}

	return -1;
}

void pttSetclaim(void *arg, int val)
{
	struct ptt_t *pInst = (struct ptt_t *)arg;

	if (!pInst)
		return;
	pInst->ttyrelease = val;
}

void pttHelp(char *str, unsigned int maxsize)
{
	char *helptext = \
	"  -p <devname> <pttbit>  pttport and bit to switch\n" \
	"                         * /dev/ttyXX for serial\n" \
	"                         * /dev/parport0 for parallel\n" \
	"                         * gpio for kernel gpio-interface\n" \
	"                         choose value for <pttbit>:\n" \
	"                         * tty: 0=RTS, 1=DTR\n" \
	"                         * parport: 0...7 / -0...-7 (inverted)\n" \
	"  -u                    close ptt-tty file between switch actions, " \
				"may not work on USB tty";

	strncpy(str, helptext, maxsize);
}
#else /* MACOS */
static int ptt_ttyDestroy(struct ptt_t *pInst)
{
	return 0;
}

void *pttinit(char *devname, int bit)
{
	return NULL;
}

void pttDestroy(void *arg)
{
	return;
}

int ptt(void *arg, unsigned int val)
{
	return 0;
}

void pttSetclaim(void *arg, int val)
{
	return;
}

void pttHelp(char *str, unsigned int maxsize) {
	char *helptext = "PTT not supported yet on MacOS";
	strncpy(str, helptext, maxsize);
}
#endif
