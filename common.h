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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <limits.h>

/*
void print_err(int err_code, const char *fmt, ...)

void *w_malloc(size_t count)
void w_free(void *data)

SBUF_NEW(NAME, SIZE)
SBUF_FREE(NAME)
SBUF_CHECK(NAME)

int file_stat(const char *path, int *size)
int file_open(int *size, const char *path, int flags)
int file_close(const int fd);
int file_read(const int fd, char *buffer, int nbytes)
int file_readline(const int fd, char *buffer, int nbytes)
char *file_mmap(const char *path, int *size)
int file_munmap(char *buf, int size)
sbuf_t *file_map_load(const char *path)
void file_map_unload(sbuf_t *fm)

*/

static __inline void print_err(int err_code, const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);

    vfprintf(stderr, fmt, va);
    if (err_code)
        {
            fprintf(stderr, ": %s\n", strerror(err_code));
        }
        
    va_end(va);

}

typedef enum {
    MSG_DEBUG, MSG_INFO, MSG_NOTICE, MSG_WARN, MSG_ERR, MSG_CRIT, MSG_ALERT,
    MSG_EMERG
} msg_level_t;

msg_level_t general_msg_level;

static __inline void message(msg_level_t level, int err_code, const char *fmt, ...)
{
    va_list va;
    char buffer[_POSIX2_LINE_MAX];
    char *tags[] = { "[DEBUG]", "[INFO]", "[NOTICE]",
		    "[WARNING]", "[ERROR]", "[CRITICAL]",
		    "[ALERT]", "[FATAL]" };

    if (level < general_msg_level)
	return;

    va_start(va, fmt);
    vsnprintf(buffer, _POSIX2_LINE_MAX, fmt, va);

    if (err_code)
    {
	fprintf(stderr, "%s %s: %s\n", tags[level], buffer, strerror(err_code));
    } else {
	fprintf(stderr, "%s %s", tags[level], buffer);
    }
    va_end(va);
}

/* ------------------- memory --------------------- */

static __inline void *w_malloc(size_t count)
{
    void *data;
	
    if ((data = malloc(count)) == NULL)
    {
	print_err(errno, "FATAL: could not allocate");
	exit(1);
    }
    memset(data, 0, count);
    return data;
}

static __inline void w_free(void *data)
{
    if (data != NULL)
	    free(data);
}
/* ------------------- end memory --------------------- */

/* ----------------------- sized buffer -------------------- */
typedef struct size_buffer {
	unsigned long size;
	char *buffer;
} sbuf_t;

#define SBUF_NEW(NAME, SIZE)                         \
{                                                    \
	NAME = (sbuf_t *) w_malloc(sizeof(sbuf_t));  \
	NAME->size = (unsigned long) SIZE;           \
	NAME->buffer = (char *) w_malloc(SIZE);      \
	memset(NAME->buffer, '\0', SIZE);            \
}

#define SBUF_FREE(NAME)        \
{                              \
	w_free(NAME->buffer);  \
	w_free(NAME);          \
}

#define SBUF_CHECK(NAME) \
if ((NAME->size <= 0) || (NAME->buffer == NULL))
/* ----------------------- end sized buffer -------------------- */

/* ----------------------- file routines ----------------------- */

/* -1 error
   0 reg
   1 not reg
*/
static __inline int file_stat(const char *path, int *size)
{
    struct stat sb;

    if (stat(path, &sb) == -1) /* error */
    {
	print_err(errno, "stat() failed");
	*size = -1;
	return -1;
    }

    if (sb.st_mode & S_IFREG)
    {
	*size = sb.st_size;
	return 0;
    }
    else 
    {
	*size = -1;
	return 1;
    }

}

static __inline int file_open(int *size, const char *path, int flags)
{
    int fd;

    if (size == NULL || path == NULL)
    {
	print_err(0, "file_open: Bad path or size buffer given !!\n");
	return -1;
    }

    if (file_stat(path, size))
	return -1;

    if ((fd=open(path, flags)) < 0)
    {
	print_err(errno, "Unable to open file %s", path);
	return -1;
    }

    return fd;
}

static __inline int file_close(const int fd)
{
    if (fd < 0)
    {
	print_err(0, "file_close: Bad fd given.\n");
	return -1;
    }

    if (close(fd) < 0)
    {
	print_err(errno, "Error while close() call");
	return -1;
    }

    return 0;
}

static __inline int file_read(const int fd, char *buffer, int nbytes)
{
    int i;

    if ((i=read(fd, buffer, nbytes)) < 0)
    {
    	/* errno == */
	print_err(errno, "file_read: Unable to read()");
    }

    return i;
}

static __inline int file_readline(const int fd, char *buffer, int nbytes)
{
    int i, r;


    for (i=0;i < nbytes;i++)
    {
        if ((r = file_read(fd, buffer + i, 1)) < 0)
        {
            /* error */
            return(-1);
        }
        else if (r == 0)
        {
            /* nothing more to read */
            break;
        }
        else if ((buffer[i] == '\n') || (buffer[i] == '\r'))
        {
            /* end of the line, force \n for input from non-unix text files */
            buffer[i] = '\n';
            i++; /* count the \n */
            break;
        }
    }
    return i;
}

static __inline char *file_mmap(const char *path, int *size)
{

    int fd;
    void *buf = NULL;

    if (path == NULL || size == NULL)
	return NULL;

    if ((fd = file_open(size, path, O_RDONLY)) < 0) {
        return NULL;
    }

    if ((buf = mmap(0, *size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
	print_err(errno, "Unable to map file (%s) into memory", path);
	return NULL;
    }
    file_close(fd);

    return buf;
}

static __inline int file_munmap(char *buf, int size)
{
    if (buf == NULL || size <= 0)
	return -1;

    if (munmap((void *)buf, size) < 0)
    {
	print_err(errno, "Unable to unmap buffer");
	return -1;
    }
    return 0;
}

static __inline sbuf_t *file_map_load(const char *path)
{
	sbuf_t *fm;
	int s;
	char *buffer = NULL;
	
	if ((buffer = file_mmap(path, &s)) == NULL)
		return NULL;
	
	fm = (sbuf_t *) w_malloc(sizeof(sbuf_t));
	fm->buffer = buffer;
	fm->size = s;
	
	return fm;
}

static __inline void file_map_unload(sbuf_t *fm)
{
	if (fm == NULL) return;
	
	file_munmap(fm->buffer, fm->size);
	w_free(fm);
}
/* ------------- end file routines ---------- */

#endif /* __COMMON_H__ */

