/*
 * This file is a part of SMalloc.
 * SMalloc is MIT licensed.
 * Copyright (c) 2017 Andrey Rys.
 */

#include "smalloc_i.h"
// (elsa) ADDED THESE for calloc/free and mmap
#include <stdlib.h>
#include <sys/mman.h> 

struct smalloc_pool smalloc_curr_pool;
struct smalloc_pool_list pool_list; // (elsa) ADDED THIS

int smalloc_verify_pool(struct smalloc_pool *spool)
{
	if (!spool->pool || !spool->pool_size) return 0;
	if (spool->pool_size % HEADER_SZ) return 0;
	return 1;
}

int sm_align_pool(struct smalloc_pool *spool)
{
	size_t x;

	if (smalloc_verify_pool(spool)) return 1;

	x = spool->pool_size % HEADER_SZ;
	if (x) spool->pool_size -= x;
	if (spool->pool_size <= MIN_POOL_SZ) {
		errno = ENOSPC;
		return 0;
	}

	return 1;
}

int sm_set_pool(struct smalloc_pool *spool, void *new_pool, size_t new_pool_size, int do_zero, smalloc_oom_handler oom_handler)
{
	if (!spool) {
		errno = EINVAL;
		return 0;
	}

	if (!new_pool || !new_pool_size) {
		if (smalloc_verify_pool(spool)) {
			if (spool->do_zero) memset(spool->pool, 0, spool->pool_size);
			memset(spool, 0, sizeof(struct smalloc_pool));
			return 1;
		}

		errno = EINVAL;
		return 0;
	}

	spool->pool = new_pool;
	spool->pool_size = new_pool_size;
	spool->oomfn = oom_handler;
	if (!sm_align_pool(spool)) return 0;

	if (do_zero) {
		spool->do_zero = do_zero;
		memset(spool->pool, 0, spool->pool_size);
	}

	return 1;
}

int sm_set_default_pool(void *new_pool, size_t new_pool_size, int do_zero, smalloc_oom_handler oom_handler)
{
	return sm_set_pool(&smalloc_curr_pool, new_pool, new_pool_size, do_zero, oom_handler);
}

int sm_release_pool(struct smalloc_pool *spool)
{
	return sm_set_pool(spool, NULL, 0, 0, NULL);
}

int sm_release_default_pool(void)
{
	return sm_release_pool(&smalloc_curr_pool);
}


/* (elsa) ADDED THIS */
int sm_pools_init(size_t capacity)
{
    if (pool_list.pools != NULL) {
        // TODO error: already initialized
        return 0;
    }

    struct smalloc_pool *pools = calloc(capacity, sizeof(struct smalloc_pool));
    if (pools == NULL) {
        // TODO error: no more memory
        return 0;
    }
    pool_list.pools = pools;
    pool_list.capacity = capacity;
    return 1;
}

int sm_add_pool(int64_t id, size_t size)
{
    if (id >= pool_list.capacity) {
        size_t new_capacity = 2*pool_list.capacity; // is that too much ? rather add a constant ?
        struct smalloc_pool *new_pools = reallocarray(pool_list.pools, new_capacity, sizeof(struct smalloc_pool));
        if (new_pools == NULL) {
            // TODO: error no more mem
            return 0;
        }
        pool_list.pools = new_pools;
        pool_list.capacity = new_capacity;
    }
    void *memptr = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (!sm_set_pool(&(pool_list.pools[id]), memptr, size, 0, NULL)) {
        return 0;
    }
    return 1;
}

int sm_release_pools(void)
{
    for (size_t i = 0; i < pool_list.capacity; ++i) {
        struct smalloc_pool pool = pool_list.pools[i];
        if (smalloc_verify_pool(&pool) && !sm_release_pool(&pool)) {
            // TODO error
            return 0;
        }
    }
    free(pool_list.pools);
    return 1;
}

