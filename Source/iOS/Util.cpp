//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

void TRACE(char const *strMsg, ...)
{
	va_list args;
	va_start(args, strMsg);
	vprintf(strMsg, args);
	va_end(args);
}

//////////////////////////////////////////////////////////////////////

void TRACE(wchar const *strMsg, ...)
{
	va_list args;
	va_start(args, strMsg);
	vwprintf(strMsg, args);
	va_end(args);
}

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG)

void DBG(int x, int y, char const *strMsg, ...)
{
	extern Font *g_DebugFont;
	extern SpriteList *g_DebugSpriteList;

	char strBuffer[512];
	va_list args;
	va_start(args, strMsg);
	vsnprintf(strBuffer, 512, strMsg, args);
	va_end(args);
	g_DebugSpriteList->ResetTransform();
	g_DebugFont->DrawString(g_DebugSpriteList, strBuffer, Vec2((float)x, (float)y));
}

#endif

//////////////////////////////////////////////////////////////////////

uint8 *LoadFile(char const *filename, size_t *size)
{
	TRACE("LoadFile(\"%s\")\n", filename);
	NSString *fullFileName = [[NSString alloc] initWithCString:filename encoding:NSASCIIStringEncoding];
	NSString *fileName = [[fullFileName lastPathComponent] stringByDeletingPathExtension];
	NSString *extension = [fullFileName pathExtension];
	NSString *path = [[NSBundle mainBundle] pathForResource:fileName ofType:extension];
	NSData *d = [NSData dataWithContentsOfFile:path];
	assert([d bytes] != nil);
	size_t len = [d length] ;
	uint8 *buf = new uint8[len];
	*size = len;
	memcpy(buf, [d bytes], len);
	return buf;
}

//////////////////////////////////////////////////////////////////////

string Format(char const *fmt, ...)
{
	char buffer[512];
	
	va_list v;
	va_start(v, fmt);
	vsnprintf(buffer, 512, fmt, v);
	return string(buffer);
}

//////////////////////////////////////////////////////////////////////

wstring WideStringFromString(string const &str)
{
	vector<wchar> temp;
	temp.resize(str.size() + 1);
	temp[0] = (wchar)0;
	AsciiToWide(str.c_str(), &temp[0]);
	return wstring(&temp[0]);
}

//////////////////////////////////////////////////////////////////////

static char const *sDateTimeFormat = "%Y-%m-%d %H:%M:%S";

//////////////////////////////////////////////////////////////////////

string FormatTime(time_t t)
{
	struct tm *stm = gmtime(&t);
	char buffer[512];
	strftime(buffer, 512, sDateTimeFormat, stm);
	return string(buffer);
}

//////////////////////////////////////////////////////////////////////

time_t ParseTime(char const *t)
{
	struct tm stm;
	return (strptime(t, sDateTimeFormat, &stm) != null) ? mktime(&stm) : 0;
}


