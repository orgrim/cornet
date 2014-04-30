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

#include "http.h"
#include "common.h"
#include "xhash.h"

http_message_t *http_message_init(void)
{
    http_message_t *message;

    message = (http_message_t *) w_malloc(sizeof(http_message_t));

    message->type = HTTP_UNDEF;
    message->method = HM_UNDEF;
    message->status_code = 0;
    message->uri = NULL;
    message->version.major = 1;
    message->version.minor = 1;
    xhash_init(message->headers);
    message->body_size = -1;
    message->body = NULL;

    return message;
}

int http_message_destroy(http_message_t *message)
{
    if (message == NULL)
	return 1;

    if (message->uri != NULL)
	w_free(message->uri);

    if (message->headers != NULL)
	xhash_destroy(message->headers);

    if (message->body != NULL)
	w_free(message->body);

    w_free(message);
    message = NULL;
    return 0;
}

http_message_t *http_request_init(http_method_t method, char *uri, int s,
				  char *body)
{
    if (method == NULL)
	return NULL;

    if (http_uri_check(uri))
    {
	
}

http_message_t *http_response_init(int status, int s, char *body);

int http_header_set(http_message_t *message, char *name, char *value);
char *http_header_get(http_message_t *message, char *name);
int http_header_validate(http_message_t *message);

int *http_message_send(int fd, http_message_t *message);
int *http_message_recv(int fd, http_message_t *message);
