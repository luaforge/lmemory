//
// $Id: lmemory.h,v 1.1 2007-05-13 15:03:27 outstanding Exp $
//

#ifndef __LMEMORY_H__
#define __LMEMORY_H__

#include <sys/types.h>

// 1		2		3		4		5		6		7		8		9		10
// 32		64		128		256		512		1024	2048	4096	8192	16384
// 32		64		128		256		512		1024	1536	2048	2560	3072
// 3584		4096	4608	5120	5632	6144	6656	7168	7680	8192
// 8704		9216	9728	10240	10752	11264	11776	12288	12800	13312
#define NBUCKETS	24

typedef struct free_list
{
	struct free_list* next_;
} free_list_t;

typedef struct buckets
{
	free_list_t blocks_;
	free_list_t first_[NBUCKETS];
	size_t fsize_[NBUCKETS];
	int bfree_;
} buckets_t;


buckets_t* ff_buckets_t_init( buckets_t* ud );
void ff_buckets_t_destroy( buckets_t* ud );
void* ff_malloc( buckets_t* ud, size_t nsize );
void ff_free( buckets_t* ud, void* ptr );
void* ff_realloc( void* bt, void* ptr, size_t nouse, size_t nsize );

#endif
