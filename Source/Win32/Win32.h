//////////////////////////////////////////////////////////////////////

#include "graphics.h"

//////////////////////////////////////////////////////////////////////

inline void CD(wchar const *relative)
{
	vector<wchar> s;
	s.resize(GetCurrentDirectory(0, null));
	GetCurrentDirectory(s.size(), &s[0]);
	s.resize(s.size() - 1);
	s.push_back(L'\\');
	for(; *relative; ++relative)
	{
		s.push_back(*relative);
	}
	s.push_back(0);
	wchar_t* p = &s[0];
	SetCurrentDirectory(p);
}

namespace chs
{
	void *memory_alloc(size_t size);
	void memory_free(void *p);
}
