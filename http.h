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

#ifndef __HTTP_H__
#define __HTTP_H__

#include "xhash.h"

/* Version struct to hold major and minor parts of the version string */
typedef struct http_version {
    unsigned int major;
    unsigned int minor;
} http_version_t;

/* Constants to identify HTTP methods */
typedef enum {
    HM_UNDEF = 0,
    HM_OPTIONS = 1,
    HM_GET = 2,
    HM_HEAD = 3,
    HM_POST = 4,
    HM_PUT = 5,
    HM_DELETE = 6,
    HM_TRACE = 7,
    HM_CONNECT = 8
} http_method_t;

/* Message type. Request or Response */
typedef enum {
    HTTP_UNDEF = 0,
    HTTP_REQUEST = 1,
    HTTP_RESPONSE = 2
} http_msg_type_t;

/* Header list elem. A hash to allow fast search */
typedef struct http_header {
    char *name;
    char *value;
} http_header_t;

/* Struct to hold a request */
typedef struct http_message {
    http_msg_type_t type;
    http_method_t method;
    int status_code;
    uri_t *uri;
    http_version_t version;
    xhash_t *headers; /* separate General Header ??? */
    int body_size;
    char *body;
} http_message_t;


/* Message setup function */
http_message_t *http_message_init(void);
int http_message_destroy(http_message_t *message);

http_message_t *http_request_init(http_method_t method, uri_t *uri, int s,
				  char *body);
http_message_t *http_response_init(int status, int s, char *body);

/* Header manipulation functions */
int http_header_set(http_message_t *message, char *name, char *value);
char *http_header_get(http_message_t *message, char *name);
int http_header_validate(http_message_t *message);

/* send and receive full message */
int *http_message_send(int fd, http_message_t *message);
int *http_message_recv(int fd, http_message_t *message);

/* URI */

#endif /* __HTTP_H__ */
