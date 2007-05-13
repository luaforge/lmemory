//
// $Id: main.c,v 1.1 2007-05-13 14:39:02 outstanding Exp $
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "lmemory.h"

typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);

static void *l_alloc( void *ud, void *ptr, size_t osize, size_t nsize )
{
	(void)ud;
	(void)osize;
	if ( nsize == 0 )
	{
		free(ptr);
		return NULL;
	}
	else
		return realloc( ptr, nsize );
}


static void test_routine( lua_Alloc f, int lbound, int rbound, int count, int repeat )
{
	if( lbound < 0 || rbound < 0 || lbound > rbound || count <= 0 || repeat <= 0 )
		return;
	buckets_t b;
	void** ptrs = (void**)malloc( sizeof(void*) * count );
	if( !ptrs ) return;

	int interval = rbound - lbound;
	interval = (interval == 0) ? 1 : interval;
	ff_buckets_t_init( &b );
	while( repeat-- )
	{
		int loop = count;
		while( loop-- )
			ptrs[loop] = (*f)( &b, NULL, 0, lbound + loop % interval );

		loop = count;
		while( loop-- )
			(*f)( &b, ptrs[loop], lbound + loop % interval, 0 );
	}
	ff_buckets_t_destroy( &b );
	free( ptrs);
}

int main( int argc, char** argv )
{
	int i;
	int ch;
	lua_Alloc f;
	static int data[][4] = {
		{ 0, 32, 1000, 1000 },
		{ 32, 256, 1000, 1000 },
	};
	
	f = l_alloc;
	while ( ( ch = getopt( argc, argv, ":sf" ) ) != EOF )
	{
		if( ch == 's' ) {
			f = l_alloc;
		}
		else if( ch == 'f' ) {
			f = ff_realloc;
		}
	}
	for( i = 0; i < sizeof(data) / (sizeof(int)*4); ++i )
	{
		test_routine( f, data[i][0], data[i][1], data[i][2], data[i][3] );
	}
	return 0;
}
