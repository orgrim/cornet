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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "tcpserver.h"


static int check_handler(tcp_server_t *srv_handle);
static tcp_server_thread_t *tcp_server_thread_create(void);
static void *tcp_server_thread_routine(void *th_handle);
static int tcp_server_thread_destroy(tcp_server_thread_t *th_handle);

static void *tcp_server_run(void *srv_handle);

static int check_handler(tcp_server_t *srv_handle)
{
    if (srv_handle == NULL)
    {
	message(MSG_DEBUG, 0, "tcp_server: handler NULL\n");
	return -1;
    }
    if (srv_handle->max_threads <= 0)
    {
	message(MSG_DEBUG, 0, "tcp_server: max threads negatif or zero\n");
	return -1;
    }
    if (srv_handle->worker == NULL)
    {
	message(MSG_DEBUG, 0, "tcp_server: bad function\n");
	return -1;
    }
    return 0;
}

static tcp_server_thread_t *tcp_server_thread_create(void)
{
    tcp_server_thread_t *new;

    new = (tcp_server_thread_t *) w_malloc(sizeof(tcp_server_thread_t));
    new->fd = -1;
    memset(&(new->client_addr), 0, sizeof(struct sockaddr_in));
    new->running = 0;
    new->thread = (pthread_t *) w_malloc(sizeof(pthread_t));
    pthread_mutex_init(&new->th_mutex, NULL);
    pthread_cond_init(&new->run_cond, NULL);

    message(MSG_DEBUG, 0, "tcp_server_thread_create: working thread struct created\n");
    return new;
}

static void *tcp_server_thread_routine(void *th_handle)
{
    tcp_server_thread_t *st;
    char client_ip[INET_ADDRSTRLEN];

    if (th_handle == NULL)
	pthread_exit(NULL);

    st = (tcp_server_thread_t *)th_handle;

    /* change state when starting */
    message(MSG_DEBUG, 0, "[%lu] changing running state to 1\n", pthread_self());
    pthread_mutex_lock(&(st->th_mutex));
    st->running = 1;
    pthread_mutex_unlock(&(st->th_mutex));

    message(MSG_DEBUG, 0, "[%lu] entering loop\n", pthread_self());
    /* main loop for the thread */
    while (st->running)
    {
	/* check for a valid connection to process */
	message(MSG_DEBUG, 0, "[%lu] getting lock\n", pthread_self());
	pthread_mutex_lock(&(st->th_mutex));
	if (st->fd < 0)
	{
	    message(MSG_DEBUG, 0, "[%lu] waiting for cond\n", pthread_self());
	    pthread_cond_wait(&(st->run_cond), &(st->th_mutex));
	}
	message(MSG_DEBUG, 0, "[%lu] unlocking\n", pthread_self());
	pthread_mutex_unlock(&(st->th_mutex));
	
	if (st->fd < 0)
	    continue;

	/* who is it */
	
	inet_ntop(AF_INET, &(((struct sockaddr_in *)&st->client_addr)->sin_addr), client_ip, sizeof client_ip);
	message(MSG_INFO, 0, "[%lu] handling connection from %s\n", pthread_self(), client_ip);

	    
	/* process connection */
	message(MSG_DEBUG, 0, "[%lu] launching connection handler\n", pthread_self());
	st->work(st->fd);

	/* finish */
	message(MSG_DEBUG, 0, "[%lu] closing connection\n", pthread_self());
	close(st->fd);

	pthread_mutex_lock(&(st->th_mutex));
	st->fd = -1;
	pthread_mutex_unlock(&(st->th_mutex));
    }

    message(MSG_DEBUG, 0, "[%lu] exiting loop\n", pthread_self());
    pthread_exit(NULL);

    return (void *)NULL;
}

static int tcp_server_thread_destroy(tcp_server_thread_t *th_handle)
{
    if (th_handle == NULL)
	return -1;

    if (th_handle->fd >= 0)
	close(th_handle->fd);
    pthread_mutex_destroy(&th_handle->th_mutex);
    pthread_cond_destroy(&th_handle->run_cond);
    w_free(th_handle->thread);
    w_free(th_handle);

    return 0;
}

/* init struct */
tcp_server_t *tcp_server_create(char *addr, unsigned int port,
				void (*func)(int), int max)
{
    tcp_server_t *new;

    /* input sanity check */
    if (func == NULL || max <= 0)
    {
	message(MSG_ERR, 0, "tcp_server: bad input\n");
	return NULL;
    }
    if (addr == NULL)
	message(MSG_INFO, 0, "Initialiting TCP server on *:%d with %d threads\n", port, max); 
    else
	message(MSG_INFO, 0, "Initialiting TCP server on %s:%d with %d threads\n", addr,  port, max); 

    new = (tcp_server_t *) w_malloc(sizeof(tcp_server_t));
    new->max_threads = max;
    new->state = SRV_OFF;
    new->threads = (tcp_server_thread_t **) w_malloc(max*sizeof(tcp_server_thread_t));
    new->worker = func;
    new->fd = -1;
    new->srv_addr.sin_family = AF_INET;
    new->srv_addr.sin_port = htons(port);
    if (addr == NULL)
    {
	new->srv_addr.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
	inet_aton(addr, &(new->srv_addr.sin_addr));
    }
    memset(&(new->srv_addr.sin_zero), 0, sizeof(new->srv_addr.sin_zero));
    pthread_mutex_init(&new->srv_mutex, NULL);

    message(MSG_DEBUG, 0, "tcp_server_create: struct initialized\n");

    return new;
}

/* bind and launch threads */
int tcp_server_start(tcp_server_t *srv_handle)
{
    int yes, i, err;
    pthread_attr_t attr;

    yes = 1;
    /* sanity check */
    if (check_handler(srv_handle))
	return -1;

    /* go to load level */
    srv_handle->state = SRV_LOAD;

    /* create socket */
    if ((srv_handle->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
	message(MSG_ERR, errno, "Could not open socket");
	return -1;
    }
    /* set socket option */
    if (setsockopt(srv_handle->fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)))
    {
	message(MSG_WARN, errno, "Could not set socket option");
    }
    /* bind to local address */
    if (bind(srv_handle->fd, (struct sockaddr *) &(srv_handle->srv_addr),
	     sizeof(struct sockaddr)) == -1)
    {
	message(MSG_ERR, errno, "Unable to bind on local address/port");
	close(srv_handle->fd);
	return -1;
    }
    /* listen */
    if (listen(srv_handle->fd, 9) < 0)
    {
	message(MSG_ERR, errno, "Unable to listen on socket");
	close(srv_handle->fd);
	return -1;
    }
    
    /* setup attributes */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);

    /* create threads */
    message(MSG_DEBUG, 0, "tcp_server_start: creating %d threads...\n", srv_handle->max_threads);
    for (i = 0; i < srv_handle->max_threads; i++)
    {
	srv_handle->threads[i] = tcp_server_thread_create();
	srv_handle->threads[i]->work = srv_handle->worker;

	err = pthread_create(srv_handle->threads[i]->thread, &attr, tcp_server_thread_routine,
			 (void *)srv_handle->threads[i]);
	if(err != 0)
	{
	    message(MSG_ERR, err, "Unable to create thread number %d", i);
	    continue;
	}
    }

    /* accept thread */
    message(MSG_DEBUG, 0, "tcp_server_start: creating accept thread...\n");
    srv_handle->srv_thread = (pthread_t *) w_malloc(sizeof(pthread_t));
    err = pthread_create(srv_handle->srv_thread, &attr, tcp_server_run, (void *)srv_handle);
    if(err != 0)
    {
	message(MSG_ERR, err, "Unable to create accept thread\n");
	/* join threads, close socket and exit */
	for (i=0; i < srv_handle->max_threads; i++)
	{
	    message(MSG_ERR, 0, "Terminating thread %d...", i);
	    pthread_mutex_lock(&(srv_handle->threads[i]->th_mutex));
	    srv_handle->threads[i]->running = 0;
	    pthread_mutex_unlock(&(srv_handle->threads[i]->th_mutex));
	    
	    pthread_join(*(srv_handle->threads[i]->thread), NULL);

	    tcp_server_thread_destroy(srv_handle->threads[i]);
	}
	w_free(srv_handle->srv_thread);
	close(srv_handle->fd);
	srv_handle->state = SRV_OFF;
	return -1;
    }
    message(MSG_INFO, 0, "TCP server started\n");
    return 0;
}

static void *tcp_server_run(void *srv_handle)
{
    tcp_server_t *srv_h;
    socklen_t sin_size;
    int n, i, found;

    srv_h = (tcp_server_t *)srv_handle;
    if (check_handler(srv_h))
	pthread_exit(NULL);

    sin_size = sizeof(struct sockaddr_in);

    pthread_mutex_lock(&srv_h->srv_mutex);
    srv_h->state = SRV_ON;
    pthread_mutex_unlock(&srv_h->srv_mutex);

    message(MSG_INFO, 0, "TCP server running\n");

    while (srv_h->state == SRV_ON)
    {
	found = 0;
	/* search an available thread */
	for (i = 0; i < srv_h->max_threads; i++)
	{
	    if (srv_h->threads[i]->fd < 0)
	    {
		n = i;
		found = 1;
		break;
	    }
	}

	if (found) {
	    pthread_mutex_lock(&(srv_h->threads[n]->th_mutex));
	    if ((srv_h->threads[n]->fd = accept(srv_h->fd, &srv_h->threads[n]->client_addr, &sin_size)) < 0)
	    {
		message(MSG_DEBUG, errno, "tcp_server: unable to accept");
		srv_h->threads[n]->fd = -1;
	    }
	    message(MSG_INFO, 0, "Accepted connection from bla (%d)\n", n);
	    pthread_cond_signal(&(srv_h->threads[n]->run_cond));
	       pthread_mutex_unlock(&(srv_h->threads[n]->th_mutex));
	}
	else
	{
	    message(MSG_INFO, 0, "Too many connection, waiting a little\n");
	    /* find some thread safe sleep */
	    usleep(500);
	}
    }
    pthread_exit(NULL);

    return (void *)NULL;
}


/* stop all thread stop service */
int tcp_server_stop(tcp_server_t *srv_handle)
{
    int i;
    if (check_handler(srv_handle))
	return -1;

    switch (srv_handle->state)
    {
    case SRV_OFF:
	message(MSG_ERR, 0, "Unable to stop: Server not started\n");
	return -2;
    case SRV_LOAD:
	message(MSG_ERR, 0, "Unable to stop: Server not fully started or already stopping\n");
	return -2;
    case SRV_ON:
    default:
	break;
    }

    message(MSG_INFO, 0, "Terminating dispatcher thread...\n");
    pthread_mutex_lock(&(srv_handle->srv_mutex));
    srv_handle->state = SRV_LOAD;
    pthread_mutex_unlock(&(srv_handle->srv_mutex));
    /* shutdown and close connection */
    message(MSG_INFO, 0, "Stopping server and closing connection\n");
    shutdown(srv_handle->fd, SHUT_RDWR);
    close(srv_handle->fd);
    /* terminate threads */
    pthread_join(*(srv_handle->srv_thread), NULL);
    
    for (i=0; i < srv_handle->max_threads; i++)
    {
	message(MSG_WARN, 0, "Terminating thread %d...\n", i);
	message(MSG_DEBUG, 0, "locking mutex for %d\n", i);
	pthread_mutex_lock(&(srv_handle->threads[i]->th_mutex));
	message(MSG_DEBUG, 0, "mutex locked for %d\n", i);

	message(MSG_DEBUG, 0, "setting running to 0 for %d\n", i);
	srv_handle->threads[i]->running = 0;

	message(MSG_DEBUG, 0, "sending cond signal to %d\n", i);
	pthread_cond_signal(&(srv_handle->threads[i]->run_cond));

	message(MSG_DEBUG, 0, "cond signal send, unlocking mutex for %d\n", i);
	pthread_mutex_unlock(&(srv_handle->threads[i]->th_mutex));

	message(MSG_DEBUG, 0, "joining thread for %d\n", i);
	pthread_join(*(srv_handle->threads[i]->thread), NULL);

	message(MSG_DEBUG, 0, "thread has exited for %d, destroying data\n", i);
	tcp_server_thread_destroy(srv_handle->threads[i]);
    }

    w_free(srv_handle->srv_thread);
    srv_handle->fd = -1;
    srv_handle->state = SRV_OFF;

    message(MSG_INFO, 0, "TCP server stopped\n");
    return 0;
}

/* destroy struct */
int tcp_server_destroy(tcp_server_t *srv_handle)
{
    if (srv_handle == NULL)
	return -1;

    if (srv_handle->state != SRV_OFF)
	tcp_server_stop(srv_handle);

    if (srv_handle->fd >= 0)
	close(srv_handle->fd);

    pthread_mutex_destroy(&srv_handle->srv_mutex);
    w_free(srv_handle);

    message(MSG_DEBUG, 0, "TCP Server uninitialized\n");
    return 0;
}
