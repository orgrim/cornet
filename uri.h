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

#ifndef __URI_H__
#define __URI_H__

#include "xhash.h"

typedef enum {
    US_UNDEF, US_HTTP, US_HTTPS
} uri_scheme_t;

typedef struct uri {
    uri_scheme_t proto;
    char *host;
    unsigned int port;
    char *path;
    xhash_t *query;
    char *related; /* after # */
} uri_t;

uri_t *uri_init(void);
int uri_destroy(uri_t *uri);

uri_t *uri_create(uri_t *uri, int secure, char *host, int port, char *resource);
uri_t *uri_parse(char *str);

char *uri_get_path(uri_t *uri);
char *uri_get_host_header(uri_t *uri);

int *uri_add_to_query(uri_t *uri, char *name, char *value);

char *uri_encode(uri_t *uri, int full);
char *uri_decode(char *str);

#endif /* __URI_H__ */
