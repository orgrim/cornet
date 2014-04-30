/*
 * Copyright 2006 Chistopher Faulet. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



#ifndef __XLIST_H__
#define __XLIST_H__

/*
 * This file define some Macro to construct, use and free lists
 *
 *	- DEF_XLIST define a struct with the first and last pointer on XLIST
 *	- NEW_XLIST allocate memory for XLIST
 *	- FOR_XLIST is the for instruction on XLIST
 *	- FREE_XLIST free memory of XLIST
 *	- INSERT_XLIST insert new elmt in XLIST
 *	- REMOVE_XLIST remove an elmt in XLIST
 *	- FIND_BY_FIELD_XLIST search a elmt on XLIST
 */

#define DEF_XLIST(NAME, TYPE)		\
	typedef struct NAME {		\
		TYPE	*first, *last;	\
} NAME##_t;

#define NEW_XLIST(LIST, TYPE)			\
{						\
	LIST = (TYPE *)w_malloc(sizeof(TYPE));	\
	LIST->first = LIST->last = NULL;	\
}

#define FOR_XLIST(VAR, LIST)					\
	for (VAR = LIST->first; VAR != NULL; VAR = VAR->next)

#define FREE_XLIST(LIST, TYPE, FREE)	\
if (LIST != NULL)			\
{					\
	TYPE	*tmp1, *tmp2;		\
	tmp1 = LIST->first;		\
	while (tmp1)			\
	{				\
	  tmp2 = tmp1->next;		\
	  FREE(tmp1);			\
	  tmp1 = tmp2;			\
	}				\
	free(LIST);			\
	LIST = NULL ; \
} 

#define XLIST_ISEMPTY(LIST)	(LIST->first == NULL)

#define INSERT_XLIST(LIST, NEW_ITEM)	\
if (NEW_ITEM != NULL)			\
{					\
	if (LIST->first == NULL)	\
	  LIST->first = NEW_ITEM;	\
	else				\
	  LIST->last->next = NEW_ITEM;	\
	NEW_ITEM->next = NULL;		\
	LIST->last = NEW_ITEM;		\
}

#define REMOVE_XLIST(LIST, LIST_ITEM, TYPE, FREE_ITEM)			\
{									\
	TYPE	*tmp;							\
									\
	if (LIST_ITEM == NULL || LIST == NULL)				\
	  break;							\
	if (LIST_ITEM == LIST->first && LIST_ITEM == LIST->last)	\
	{								\
	    FREE_ITEM(LIST_ITEM);					\
	    LIST->first = NULL;						\
	    LIST->last = NULL;						\
	    break;							\
	}								\
	else if (LIST_ITEM == LIST->first)				\
	{								\
	    LIST->first = LIST_ITEM->next;				\
	    FREE_ITEM(LIST_ITEM);					\
            LIST_ITEM = LIST->first;					\
	}								\
	else if (LIST_ITEM == LIST->last)				\
	{								\
	    FOR_XLIST(tmp, LIST)					\
	      if (tmp->next == LIST->last)				\
			break;							\
	    FREE_ITEM(LIST_ITEM);					\
	    tmp->next = NULL;						\
	    LIST->last = tmp;						\
	    LIST_ITEM = LIST->last;					\
	}								\
	else								\
	  {								\
	     FOR_XLIST(tmp, LIST)					\
	      if (tmp->next == LIST_ITEM)				\
			break;						\
	     tmp->next = LIST_ITEM->next;				\
	     FREE_ITEM(LIST_ITEM);					\
	     LIST_ITEM = tmp;						\
	  }								\
}


#define FIND_BY_ID_XLIST(VAR, LIST, TYPE, ID, VALUE)	\
{							\
	TYPE	*tmp;					\
							\
	VAR = NULL;					\
	FOR_XLIST(tmp, LIST)				\
	  if (tmp->ID == VALUE)				\
	    {						\
	      VAR = tmp;				\
	      break;					\
	    }						\
}

#define DUMP_XLIST(LIST, TYPE, DUMP_ITEM) \
{                                         \
	TYPE	*tmp;                     \
	                                  \
	FOR_XLIST(tmp, LIST)              \
		DUMP_ITEM(tmp);           \
}

/*#define FIND_BY_FIELD_XLIST(VAR, LIST, TYPE, FIELD, VALUE)	\
{								\
	TYPE	*tmp;						\
								\
	VAR = NULL;						\
	FOR_XLIST(tmp, LIST)					\
	  if (tmp->FIELD != NULL && !xstrcasecmp(tmp->FIELD, VALUE)) \
	    {							\
	      VAR = tmp;					\
	      break;						\
	    }							\
}
*/

#endif /* __XLIST_H__ */
