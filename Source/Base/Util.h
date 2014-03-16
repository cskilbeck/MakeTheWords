//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

void TRACE(char const *strMsg, ...);
void TRACE(wchar const *strMsg, ...);

#if defined(DEBUG)
void DBG(int x, int y, char const *strMsg, ...);
void DBG(int x, int y, wchar const *strMsg, ...);
#else
#define DBG if (false) {} else 
#endif

uint8 *LoadFile(char const *filename, size_t *size = null);
wstring WideStringFromString(string const &str);
wstring Format(wchar const *fmt, ...);
string Format(char const *fmt, ...);

wstring GetCurrentFolder();

#ifndef PI
#define PI 3.14159265f
#define PI_2 (PI/2)
#endif

//////////////////////////////////////////////////////////////////////
// UTF8 decoding

bool GetUTF8Length(uint8 *bytes, uint32 *length);
bool DecodeUTF8(uint8 *bytes, wchar *buffer, uint32 bufferSize);

//////////////////////////////////////////////////////////////////////

string FormatTime(time_t t);
time_t ParseTime(char const *t);

//////////////////////////////////////////////////////////////////////

template <typename T> inline int64 asciiToInt64(T const *p)
{
	assert(p != 0);
	int64 result = 0;
	bool negate = false;
	if(*p == '-')
	{
		negate = true;
		++p;
	}
	while(*p)
	{
		result *= 10;
		T c = *p++;
		if(c < '0' || c > '9')
		{
			break;
		}
		result += c - '0';
	}
	return negate ? -result : result;
}

//////////////////////////////////////////////////////////////////////

template <typename T> void SafeDeleteArray(T * &arr)
{
	if(arr != null)
	{
		delete[] arr;
		arr = null;
	}
}

//////////////////////////////////////////////////////////////////////

template <typename T> void SafeDelete(T * &arr)
{
	if(arr != null)
	{
		delete arr;
		arr = null;
	}
}

//////////////////////////////////////////////////////////////////////

template <typename T> void SafeRelease(T * &ptr)
{
	if(ptr != null)
	{
		ptr->Release();
		ptr = null;
	}
}

//////////////////////////////////////////////////////////////////////
// !! BY VALUE

template <typename T> T Max(T a, T b)
{
	return a > b ? a : b;
}

//////////////////////////////////////////////////////////////////////

template <typename T> T Min(T a, T b)
{
	return a < b ? a : b;
}

//////////////////////////////////////////////////////////////////////

template <typename T> T Constrain(T a, T lower, T upper)
{
	return Min(Max(a, lower), upper);
}

//////////////////////////////////////////////////////////////////////

template <typename T> int sgn(T val)	// where T is int, short, char etc. caveat caller
{
	return (T(0) < val) - (val < T(0)); 
}

//////////////////////////////////////////////////////////////////////

template <typename T> void Swap(T &a, T &b)
{
	std::swap(a, b);
}

//////////////////////////////////////////////////////////////////////

inline float Ease(float d)
{
	float d2 = d * d;
	float d3 = d2 * d;
	return 3 * d2 - 2 * d3;
}

//////////////////////////////////////////////////////////////////////

inline string ToLower(string const &s)
{
	string r(s);
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

//////////////////////////////////////////////////////////////////////

inline wstring ToLower(wstring const &s)
{
	wstring r(s);
	std::transform(r.begin(), r.end(), r.begin(), ::tolower);
	return r;
}

//////////////////////////////////////////////////////////////////////

inline int NextPowerOf2(int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}
