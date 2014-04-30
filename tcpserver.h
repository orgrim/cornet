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

#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef enum {
    SRV_OFF, SRV_LOAD, SRV_ON
} srv_state_t;

typedef struct tcp_server_thread
{
    int fd; /* client fd */
    struct sockaddr client_addr;
    int running; /* to stop the thread */
    void (*work)(int); /* communication routine that gets a fd */
    pthread_t *thread;
    pthread_mutex_t th_mutex;
    pthread_cond_t run_cond;
} tcp_server_thread_t;

typedef struct tcp_server
{
    int fd; /* socket file descriptor */
    struct sockaddr_in srv_addr; /* address to bind to */
    void (*worker)(int); /* communication routine */
    int max_threads; /* max number of threads at runtime */
    srv_state_t state;
    tcp_server_thread_t **threads;
    pthread_t *srv_thread;
    pthread_mutex_t srv_mutex; /* to lock this resource */
} tcp_server_t;



/* ------- API -------- */
tcp_server_t *tcp_server_create(char *addr, unsigned int port,
				void (*func)(int), int max);
int tcp_server_start(tcp_server_t *srv_handle);
int tcp_server_stop(tcp_server_t *srv_handle);
int tcp_server_destroy(tcp_server_t *srv_handle);


#endif /* __TCP_SERVER_H__ */
