//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef uint64_t		uint64;
typedef int8_t			int8;
typedef int16_t			int16;
typedef int32_t			int32;
typedef int64_t			int64;
typedef unsigned int	uint;
typedef wchar_t			wchar;

typedef uint8			byte;

#define null nullptr

using std::vector;
using std::function;
using std::string;
using std::wstring;
using std::wstringstream;
using std::map;
using std::multimap;
using std::list;
using std::pair;
using std::sort;

#if !defined(ARRAYSIZE)
#define ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif