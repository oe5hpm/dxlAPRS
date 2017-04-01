/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <sys/types.h>
#include <time.h>

#include <fcntl.h>
#include <utmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>

int logout1(int pid, char *line)
{
	int fd;
	struct utmp ut;

	if ((fd = open(_PATH_UTMP, O_RDWR, 0)) < 0) return 0;

	while (read(fd, &ut, sizeof(struct utmp)) == sizeof(struct utmp)) {
		if(ut.ut_pid!=pid) continue;
		strncpy(line, ut.ut_line, UT_LINESIZE);
		bzero(ut.ut_name, UT_NAMESIZE);
		bzero(ut.ut_host, UT_HOSTSIZE);
		time(&ut.ut_time);
		lseek(fd, -(off_t)sizeof(struct utmp), L_INCR);
		write(fd, &ut, sizeof(struct utmp));
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}


void logwtmp(const char *line, const char *name, const char *host)
{
	struct utmp ut;
	struct stat buf;
	int fd;

	if ((fd = open(_PATH_WTMP, O_WRONLY|O_APPEND, 0)) < 0) return;
	if (fstat(fd, &buf) == 0) {
		ut.ut_pid = getpid();
		ut.ut_type = (name[0] != '\0')? USER_PROCESS : DEAD_PROCESS;
		strncpy(ut.ut_id, "", 2);
		strncpy(ut.ut_line, line, sizeof(ut.ut_line));
		strncpy(ut.ut_name, name, sizeof(ut.ut_name));
		strncpy(ut.ut_host, host, sizeof(ut.ut_host));
		time(&ut.ut_time);
		if (write(fd, &ut, sizeof(struct utmp)) != sizeof(struct utmp))
			ftruncate(fd, buf.st_size);
	}
	close(fd);
}



#define DEVPATH "/dev/"
void cleanup(int sig) {
	int status, pid;
	char line[UT_LINESIZE+sizeof(DEVPATH)];
	
	if(sig!=SIGCHLD) return;
	while( (pid=waitpid(-1, &status, WNOHANG /* |WUNTRACED */ )) > 0 ) {
/*
		if(WIFSTOPPED(status)) {
			fprintf(stderr,"process %d stopped by signal %d!\n",
				pid, WSTOPSIG(status));
			continue;
		}
*/
		sprintf(line,DEVPATH);
		if( logout1(pid, line+sizeof(DEVPATH)-1) ) {
			logwtmp(line+sizeof(DEVPATH)-1, "", "");
			chmod(line, 0666);
			chown(line, 0, 0);
			line[sizeof(DEVPATH)-1]='p';  /* ttyXX -> ptyXX */
			chmod(line, 0666);
			chown(line, 0, 0);
		}
	}
}
