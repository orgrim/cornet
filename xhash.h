#ifndef __X_HASH__
#define __X_HASH__

/*
 * Perl's like hash library using the fnv algo 
 * original source taken from FreeBSD source tree .
 * 
 * This file is placed in the public domain .
 * 
 * Fowler / Noll / Vo Hash (FNV Hash)
 * http://www.isthe.com/chongo/tech/comp/fnv/
 *
 *	API:
 *
 *	xhash_t * xhash_init(xhash_t *xhash)
 *	xhash_t * xhash_destroy(xhash_t *xhash)
 *	unsigned short xhash_add(xhash_t *xhash,const char *key,void *value)
 *	unsigned short xhash_remove(xhash_t *xhash,char *key)
 *	unsigned short xhash_exists(xhash_t *xhash,char *key)
 *	unsigned long  xhash_numkeys(xhash_t *xhash)
 *	void *  xhash_value(xhash_t *xhash,char *key)
 *	void ** xhash_values(xhash_t *xhash,void **values)
 *	char ** xhash_keys(xhash_t *xhash,char **keys)
 */

#include	<sys/types.h>
#include	<stdlib.h>

#define FNV1_64_INIT ((Fnv64_t) 0xcbf29ce484222325ULL)
#define FNV_64_PRIME ((Fnv64_t) 0x100000001b3ULL)

typedef u_int64_t Fnv64_t;

/* Hash list type definition */
#define DEF_HASH(NAME, TYPE) \
    typedef struct NAME {    \
        TYPE *first;	     \
        TYPE *last;	     \
    } NAME##_t;

/* Element type definition */
#define DEF_HASH_ELEM(NAME, TYPE)	  \
    typedef struct NAME {		  \
        Fnv64_t hashkey;		  \
        char *key;			  \
        TYPE *value;			  \
        struct NAME *next;		  \
    } NAME##_t;
/*
typedef struct _xhash_elem
{
	Fnv64_t	hashkey ;
	char 	*key ;
	void 	*value ;
	struct 	_xhash_elem	*next ;
} xhash_elem_t ;

typedef struct _xhash
{
	xhash_elem_t *first; 
	xhash_elem_t *last;
	unsigned long	entries ;
} xhash_t ;
*/

static __inline Fnv64_t
fnv_64_str(const char *str, Fnv64_t hval)
{
	const u_int8_t *s = (const u_int8_t *)str;
	int64_t c;		 /* 32 bit on i386, 64 bit on alpha,ia64 */

	while ((c = *s++) != 0) {
		hval *= FNV_64_PRIME;
		hval ^= c;
	}
	return hval;
}

static __inline xhash_t *
xhash_init(xhash_t *xhash)
{
	xhash = (xhash_t *)malloc(sizeof(xhash_t));
	xhash->first = xhash->last = NULL ;
	xhash->entries = 0 ;
	return(xhash);
}

/* return 1 if entry have benn inserted 
 * else returns 0 for allready existing values*/
static __inline unsigned short
xhash_add(xhash_t *xhash,const char *key,void *value)
{
	Fnv64_t	hashkey ;
	xhash_elem_t *xhash_elem ;

	if (!xhash || !key)
		return(0);

	hashkey = fnv_64_str(key, FNV1_64_INIT) ; 

	if (!xhash->first)
	{
		xhash_elem = (xhash_elem_t *)malloc(sizeof(xhash_elem_t));
		xhash_elem->hashkey = hashkey ;
		xhash_elem->key = (char *)key ;
		xhash_elem->value = value ;
		xhash_elem->next = NULL ;
		xhash->last = xhash->first = xhash_elem ;
		xhash->entries++;
	}
	else
	{
		/* first find if the entry already exists */
		for (xhash_elem = xhash->first; xhash_elem != NULL; xhash_elem = xhash_elem->next)
			if (xhash_elem->hashkey == hashkey)
				return(0); /* entry already exists */

		if (!xhash_elem)
		{
			xhash_elem = (xhash_elem_t *)malloc(sizeof(xhash_elem_t));
			xhash_elem->hashkey = hashkey ;
			xhash_elem->key = (char *)key ;
			xhash_elem->value = value ;
			xhash_elem->next = NULL ;
			xhash->last->next = xhash_elem; /* previous last xhash_elem->next points to new xhash_elem */
			xhash->last = xhash_elem ; /* insert to list */
			xhash->entries++;
		}
	} 
	return(1); /* huh ?  */
}

/* returns 1 if entry is deleted */
static __inline unsigned short
xhash_remove(xhash_t *xhash,char *key)
{
	Fnv64_t	hashkey ;
	xhash_elem_t *xhash_elem ;
	
	if (!key || !xhash || !(xhash->first))
		return(0);

	hashkey = fnv_64_str(key,FNV1_64_INIT);

	for (xhash_elem = xhash->first; xhash_elem != NULL; xhash_elem = xhash_elem->next)
	{
		if (xhash_elem->hashkey == hashkey)
		{
			/* if there's only one entry */
			if (xhash_elem == xhash->first && xhash_elem == xhash->last)
			{
				xhash->first = xhash->last = NULL ;

				/* free elem */
				free(xhash_elem->key); 
				if (xhash_elem->value)
					free(xhash_elem->value); 
				xhash_elem->key = NULL ;
				xhash_elem->value = NULL ;
				free(xhash_elem);
			}
			else if (xhash_elem == xhash->first)
			{
				xhash->first = xhash_elem->next ;
				/* free elem */
				free(xhash_elem->key); 
				if (xhash_elem->value)
					free(xhash_elem->value); 
				xhash_elem->key = NULL ;
				xhash_elem->value = NULL ;
				free(xhash_elem);
			}
			else if (xhash_elem == xhash->last) /* sucks ! boubler la liste */
			{
				xhash_elem_t	*tmp;
				for (tmp = xhash->first; tmp != NULL; tmp = tmp->next)
	      			if (tmp->next == xhash->last)
						break;

	    		tmp->next = NULL;
	    		xhash->last = tmp;
				
				/* free elem */
				free(xhash_elem->key); 
				if (xhash_elem->value)
					free(xhash_elem->value); 
				xhash_elem->key = NULL ;
				xhash_elem->value = NULL ;
				free(xhash_elem);
			}
			else
			{
				xhash_elem_t	*tmp;
				for (tmp = xhash->first; tmp != NULL; tmp = tmp->next)
	      			if (tmp->next == xhash_elem)
						break;

	    		tmp->next = xhash_elem->next;
				
				/* free elem */
				free(xhash_elem->key); 
				if (xhash_elem->value)
					free(xhash_elem->value); 
				xhash_elem->key = NULL ;
				xhash_elem->value = NULL ;
				free(xhash_elem);
			}
			xhash->entries--;
			return(1);
		}
	}
	return(0);
}

static __inline xhash_t *
xhash_destroy(xhash_t *xhash)
{
	xhash_elem_t *tmp1, *tmp2 ;
	if (!xhash)
		return(NULL);

	tmp1 = xhash->first ;
	while(tmp1)
	{
		tmp2 = tmp1->next ;

		/* free elem */
		if (tmp1->key != NULL) free(tmp1->key); 
		if (tmp1->value != NULL) free(tmp1->value); 
		tmp1->key = NULL ;
		tmp1->value = NULL ;
		free(tmp1);

		tmp1 = tmp2;
	}
	free(xhash);
	xhash = NULL ;
	return(xhash);
}

static __inline unsigned short
xhash_exists(xhash_t *xhash,char *key)
{
	Fnv64_t	hashkey ;
	xhash_elem_t *xhash_elem ;
	
	if (!key || !xhash || !xhash->first)
		return(0);

	hashkey = fnv_64_str(key,FNV1_64_INIT);

	for (xhash_elem = xhash->first; xhash_elem != NULL; xhash_elem = xhash_elem->next)
		if (xhash_elem->hashkey == hashkey)
			return(1);

	return(0);
}

static __inline void *
xhash_value(xhash_t *xhash,char *key)
{
	Fnv64_t	hashkey ;
	xhash_elem_t *xhash_elem ;
	
	if (!key || !xhash || !xhash->first)
		return(NULL);

	hashkey = fnv_64_str(key,FNV1_64_INIT);

	for (xhash_elem = xhash->first; xhash_elem != NULL; xhash_elem = xhash_elem->next)
		if (xhash_elem->hashkey == hashkey)
			return(xhash_elem->value);

	return(NULL);
}

static __inline void **
xhash_values(xhash_t *xhash,void **values)
{
	xhash_elem_t *xhash_elem ;
	unsigned long i ;
	if (!xhash || !xhash->entries)
		return(NULL);
	if (values)
		free(values);
	values = (void **) malloc(sizeof(void *)*(xhash->entries));
	for (xhash_elem = xhash->first, i=0; i < xhash->entries && xhash_elem != NULL; xhash_elem = xhash_elem->next,i++)
		values[i] = xhash_elem->value ;
	return(values);
}

static __inline char **
xhash_keys(xhash_t *xhash,char **keys)
{
	xhash_elem_t *xhash_elem ;
	unsigned long i ;
	if (!xhash || !xhash->entries)
		return(NULL);
	if (keys)
		free(keys);
	keys = (char **) malloc(sizeof(char *)*(xhash->entries));
	for (xhash_elem = xhash->first, i=0; i < xhash->entries && xhash_elem != NULL; xhash_elem = xhash_elem->next,i++)
		keys[i] = xhash_elem->key ;
	return(keys);
}

static __inline unsigned long
xhash_numkeys(xhash_t *xhash)
{
	return(xhash->entries);
}

#endif /* __X_HASH__ */
