/*
 * This file is a part of SMalloc.
 * SMalloc is MIT licensed.
 * Copyright (c) 2017 Andrey Rys.
 */

#include "smalloc_i.h"

struct smalloc_pool_list pool_list; // (elsa) ADDED THIS

void sm_free_pool(struct smalloc_pool *spool, void *p)
{
	struct smalloc_hdr *shdr;
	char *s;

	if (!smalloc_verify_pool(spool)) {
		errno = EINVAL;
		return;
	}

	if (!p) return;

	shdr = USER_TO_HEADER(p);
	if (smalloc_is_alloc(spool, shdr)) {
		if (spool->do_zero) memset(p, 0, shdr->rsz);
		s = CHAR_PTR(p);
		s += shdr->usz;
		memset(s, 0, HEADER_SZ);
		if (spool->do_zero) memset(s+HEADER_SZ, 0, shdr->rsz - shdr->usz);
		memset(shdr, 0, HEADER_SZ);
		return;
	}

	smalloc_UB(spool, p);
	return;
}

void sm_free(void *p)
{
	sm_free_pool(&smalloc_curr_pool, p);
}


/* (elsa) ADDED THIS */
void sm_free_from_pool(int64_t id, void *p)
{
    if (id >= pool_list.capacity) {
        // TODO error
        return;
    }
    struct smalloc_pool pool = pool_list.pools[id];
    return sm_free_pool(&pool, p);
}

