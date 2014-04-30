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

#include <string.h>
#include <stdlib.h>

#include "uri.h"
#include "common.h"
#include "xhash.h"

uri_t *uri_init(void)
{
    uri_t *uri;
    
    uri = (uri_t *) w_malloc(sizeof(uri_t));
    uri->proto = US_UNDEF;
    uri->host = NULL;
    uri->port = 0;
    uri->path = NULL;
    xhash_init(uri->query);

    return uri;
}

int uri_destroy(uri_t *uri)
{
    if (uri == NULL)
	return 1;

    if (uri->query != NULL)
	xhash_destroy(uri->query);

    w_free(uri->host);
    w_free(uri->path);

    w_free(uri);
    
    return 0;
}

uri_t *uri_create(uri_t *uri, int secure, char *host, unsigned int port, char *resource)
{
    /* host param is mandatory */
    if (host == NULL)
	return NULL;

    /* init struct if not */
    if (uri == NULL)
	return NULL;

    /* use SSL if selected */
    if (secure)
	uri->proto = US_HTTPS;
    else
	uri->proto = US_HTTP;

    /* put hostname */
    if (uri->host != NULL)
	w_free(uri->host);
    uri->host = strdup(host);

    /* choose port with default */
    if (port > 65535 || port <= 0) {
	switch (uri->proto)
	{
	case US_HTTP:
	    uri->port = 80;
	    break;
	case US_HTTPS:
	    uri->port = 443;
	    break;
	default:
	    uri->port = 80;
	    break;
	}
    } else {
	uri->port = port;
    }

    /* setup resource path */
    if (uri->path != NULL)
	w_free(uri->path);

    if (resource == NULL)
	uri->path = strdup("/");
    else
	uri->path = strdup(resource);

    return uri;
}


uri_t *uri_parse(char *str)
{
    uri_t *uri;
    char *p;
    int l;

    if (str == NULL)
	return NULL;

    /* prepare a struct */
    uri = uri_init();

    p = str;
    l = strlen(str);

    /* decode str by replacing %HEXHEX */


    /* try to find the http scheme */
    if (!strncmp("http://", p, 7)) {
	uri->proto =  US_HTTP;
	if (l <= 7) {
	    message(MSG_WARN, 0, "Bad URI: Nothing after http://\n");
	    if (uri_destroy(uri))
		message(MSG_DEBUG, 0, "Unable to free() uri_t\n");
	    return NULL;
	}
	p += 7;
    } else if (!strncmp("https://", p, 8)) {
	uri->proto =  US_HTTPS;
	if (l <= 8) {
	    message(MSG_WARN, 0, "Bad URI: Nothing after https://\n");
	    if (uri_destroy(uri))
		message(MSG_DEBUG, 0, "Unable to free() uri_t\n");
	    return NULL;
	}
	p += 8;
    }

    /* try to find the hostname */

    return uri;
}

char *uri_get_path(uri_t *uri)
{
    if (uri == NULL)
	return NULL;
}

char *uri_get_host_header(uri_t *uri);

int *uri_add_to_query(uri_t *uri, char *name, char *value);

char *uri_encode(uri_t *uri, int full);
char *uri_decode(char *str);

