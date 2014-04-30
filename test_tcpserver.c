/*
 * 
 * Copyright (C) 2008 Nicolas THAUVIN <nico@orgrim.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <signal.h>
#include <unistd.h>
#include "common.h"
#include "tcpserver.h"

extern msg_level_t general_msg_level;

tcp_server_t *server;

int readline(int fd, char *buffer, int nbytes)
{
    int i, r;

    if (fd < 0 || buffer == NULL)
        return -1;

    for (i=0;i < nbytes;i++)
    {
        if ((r = read(fd, buffer + i, 1)) < 0)
        {
            /* error */
            return(-1);
        }
        else if (r == 0)
        {
            /* nothing more to read */
            break;
        }
        else if (buffer[i] == '\n')
        {
            i++; /* count the \n */
            break;
        }
    }
    return i;
}

void service(int fd)
{
    char *readbuf;
    
    /* do whatever you want here */
    readbuf = (char *) malloc((_POSIX2_LINE_MAX+1)*sizeof(char));
    memset(readbuf, 0, (_POSIX2_LINE_MAX+1));
    
    while (readline(fd, readbuf, _POSIX2_LINE_MAX) >= 0)
    {
	message(MSG_INFO, 0, "[%lu] %s", pthread_self(), readbuf);
	if (!strncmp(readbuf, "stop", 4))
	{
	    message(MSG_INFO, 0, "[%lu] got a stop\n", pthread_self());
	    break;
	}
	memset(readbuf, 0, (_POSIX2_LINE_MAX+1));
	
    }
    free(readbuf);
    
    /* close(fd); */
    
}
    
void sighandle(int signo) {
    if ((signo==SIGTERM) || (signo==SIGINT)) {
	tcp_server_stop(server);
	tcp_server_destroy(server);
	exit(0);
    }
    if (signo == SIGHUP) {
	
    }
    if (signo == SIGALRM) {
	
    }
}

int main(int argc, char** argv)
{
    general_msg_level = MSG_INFO;

    if (signal(SIGPIPE, sighandle) == SIG_ERR)
	fprintf(stderr, "can't catch SIGPIPE signal\n");
    if (signal(SIGTERM, sighandle) == SIG_ERR)
	fprintf(stderr, "can't catch SIGTERM signal\n");
    if (signal(SIGINT, sighandle) == SIG_ERR)
	fprintf(stderr, "can't catch SIGINT signal\n");
    if (signal(SIGALRM, sighandle) == SIG_ERR)
	fprintf(stderr, "can't catch SIGALRM signal\n");
    if (signal(SIGHUP, sighandle) == SIG_ERR)
	fprintf(stderr, "can't catch SIGHUP signal\n");

    message(MSG_DEBUG, 0, "Starting server\n");
    server = tcp_server_create(NULL, 6666, service, 5);
    if (tcp_server_start(server))
    {
	message(MSG_ERR, 0, "Server start error\n");
	tcp_server_destroy(server);
    }

    while (1)
	usleep(600);

    return 0;
	
}
