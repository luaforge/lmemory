//
// $Id: lmemory.c,v 1.1 2007-05-13 15:03:27 outstanding Exp $
//

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "lmemory.h"

#define MINSIZE		32
#define MAXSIZE		512
#define BLOCKSIZE	( 1024 * 1024 * 1 )

#define FHEADER_SIZE (sizeof(free_list_t))
#define FREELIST_T( m ) ((free_list_t*)m)
#define FHEADER( m ) ( (free_list_t*)((char*)m - FHEADER_SIZE) )


buckets_t* ff_buckets_t_init( buckets_t* ud )
{
	buckets_t* ret;
	size_t size;
	int i;

	if( ud == 0 )
	{
		ret = (buckets_t*)malloc( sizeof(buckets_t) );
		if( ret == 0 )		return 0;
		ret->bfree_ = 1;
	}
	else
	{
		ret = (buckets_t*)ud;
		ret->bfree_ = 0;
	}
	
	ret->blocks_.next_ = 0;
	memset( ret->first_, 0, sizeof(ret->first_) );
	
	size = MINSIZE;
	for( i = 0; i < NBUCKETS; ++i )
	{
		ret->fsize_[i] = size;
		if( size < MAXSIZE )
			size *= 2;
		else
			size += MAXSIZE;
	}
	return ret;
}

void ff_buckets_t_destroy( buckets_t* ud )
{
	free_list_t* node, *next;
	if( ud == 0 )
		return;
	node = ud->blocks_.next_;
	while( node )
	{
		next = node->next_;
		free( node );
		node = next;
	}
	if( ud->bfree_ )
	{
		free( ud );
	}
}

void ff_create_memory( buckets_t* ud, free_list_t* fl )
{
	assert( ud && fl );
	register void* mb;
	register void* ptr;
	register void* h;
	register size_t n;
	register size_t size;

	mb = (void*)malloc( BLOCKSIZE );
	if( mb == 0 )	return;

	FREELIST_T(mb)->next_ = ud->blocks_.next_;
	ud->blocks_.next_ = mb;

	assert( fl - ud->first_ >=0 && fl - ud->first_ < NBUCKETS );
	size = ( ud->fsize_[ fl - ud->first_ ] + FHEADER_SIZE );
	n = ( BLOCKSIZE - FHEADER_SIZE ) / size;
	ptr = (char*)mb + FHEADER_SIZE;

	while( n > 0 )
	{
		h = (char*)ptr + size;
		FREELIST_T(ptr)->next_ = h;
		ptr = h;
		n--;
	}
	FREELIST_T((char*)ptr - size)->next_ = fl->next_;
	fl->next_ = (free_list_t*)((char*)mb + FHEADER_SIZE);	
}

void* ff_malloc( buckets_t* ud, size_t nsize )
{
	assert( ud );
	register int n;
	register free_list_t* h;

	if( nsize <= 0 )
		return 0;

	n = 0;
	while( nsize > ud->fsize_[n] && n < NBUCKETS ) ++n;

	if( n >= 0 && n < NBUCKETS )
	{
		if( ud->first_[n].next_ == 0 )
		{
			ff_create_memory( ud, ud->first_+n );
		}
		if( ud->first_[n].next_ == 0 )
			return 0;
		
		h = ud->first_[n].next_;
		ud->first_[n].next_ = h->next_;
		h->next_ = ud->first_ + n;
		return (char*)h + FHEADER_SIZE;
	}
	else
	{
		h = (free_list_t*)malloc( nsize + FHEADER_SIZE );
		if( h )
			h->next_ = ud->first_ + NBUCKETS;
		return h ? ( (char*)h + FHEADER_SIZE ) : 0;
	}
}

void ff_free( buckets_t* ud, void* ptr )
{
	assert( ud );
	register int n;
	register free_list_t* h;

	if( ptr == 0 )	return;
	h = FHEADER( ptr );
	n = h->next_ - ud->first_;
	if( n >= 0 && n < NBUCKETS )
	{
		h->next_ = ud->first_[n].next_;
		ud->first_[n].next_ = h;
	}
	else
	{
		free( h );
	}
}


void* ff_realloc( void* bt, void* ptr, size_t nouse, size_t nsize )
{
	assert( bt );	
	register buckets_t* ud;
	register int n;
	register int osize;
	register free_list_t* h;
	register void* nb;

	ud = (buckets_t*)bt;
	if( ptr == 0 )
		return ff_malloc( ud, nsize );

	h = FHEADER( ptr );
	n = h->next_ - ud->first_;
	if( n >=0 && n < NBUCKETS )
	{
		osize = ud->fsize_[n];
		if( nsize <= osize && nsize > ( (n>=1) ? ud->fsize_[n-1] : 0 ) )
			return ptr;

		nb = ff_malloc( ud, nsize );
		if( nb )
			memcpy( nb, ptr, ( (osize < nsize) ? osize : nsize ) );
		if( nb || nsize == 0 )
			ff_free( ud, ptr );
		return nb;

	}
	else
	{
		if( nsize > 0 )
			h = (free_list_t*)realloc( h, nsize + FHEADER_SIZE );
		else
		{
			free( h );
			h = 0;
		}
		return h ? ( (char*)h + FHEADER_SIZE ) : 0;
	}
}



