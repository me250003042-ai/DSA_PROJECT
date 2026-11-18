
#pragma once

#ifdef COUNT_OPS

inline long long g_heap_compares = 0;
inline long long g_heap_swaps = 0;
inline long long g_hash_probes = 0;
inline long long g_purge_moves = 0;

#define COUNT(x) (++(x))

#else

#define COUNT(x) ((void)0)

#endif
