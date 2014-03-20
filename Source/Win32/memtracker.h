//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#if defined(new)
#undef new
#undef delete
#endif

#if defined(_aligned_malloc)
#undef _aligned_malloc
#undef _aligned_free
#endif

#if defined(malloc)
#undef malloc
#undef free
#define malloc(x) chs::memory_alloc((x), 1)
#define free(x) chs::memory_free(x)
#endif

//////////////////////////////////////////////////////////////////////

namespace chs
{
	void *memory_alloc(size_t s, size_t alignment);
	void memory_free(void *p);
	void dump_leaks();
}

inline void *_aligned_malloc(size_t s, size_t alignment)
{
	return chs::memory_alloc(s, alignment);
}

inline void _aligned_free(void *s)
{
	chs::memory_free(s);
};

//////////////////////////////////////////////////////////////////////

