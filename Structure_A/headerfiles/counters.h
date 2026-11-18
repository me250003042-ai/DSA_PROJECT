
#pragma once

#ifdef COUNT_OPS

extern long long g_heap_compares;
extern long long g_heap_swaps;
extern long long g_hash_probes;
extern long long g_purge_moves;

#define COUNT(x) (++(x))

#else

#define COUNT(x) ((void)0)

#endif
