//////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <gl/GL.h>
#include "Win32.h"
#include "PngLoader.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	List<Texture> sAllTextures;
}

//////////////////////////////////////////////////////////////////////

struct Texture::TextureImpl
{
	TextureImpl(uint8 *pixels, Size2D size)
		: mID(0)
	{
		glGenTextures(1, (GLuint *)&mID);
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.w, size.h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels);
	}

	~TextureImpl()
	{
		glDeleteTextures(1, (GLuint *)&mID);
		mID = 0;
	}

	int		mID;
};

//////////////////////////////////////////////////////////////////////

Texture::Texture(char const *name)
	: RefCount()
	, mName(ToLower(name))
	, impl(null)
{
	sAllTextures.AddTail(this);

	wstring w = WideStringFromString(mName);

	uint8_t *pixels;
	Size2D tsize;

	if(!FAILED(LoadPNGFile(w.c_str(), pixels, mSize, tsize)))
	{
		mScale = Vec2(mSize) / Vec2(tsize);
		impl = new TextureImpl(pixels, tsize);
		TRACE("LoadTexture %s %d,%d [%d,%d] : {%f,%f}\n", mName.c_str(), mSize.w, mSize.h, tsize.w, tsize.h, mScale.x, mScale.y);
		SafeDeleteArray(pixels);
	}
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color)
	: RefCount()
	, mName(Format("SOLID%08x(%d,%d)", color, width, height))
	, mSize(width, height)
	, impl(null)
{
	sAllTextures.AddTail(this);

	TRACE("CreateTexture %d,%d:%08x\n", width, height, color);

	Size2D size(mSize);
	size.w = NextPowerOf2(size.w);
	size.h = NextPowerOf2(size.h);

	size_t buffersize = size.w * size.h;

	UINT8 *pPixels = new UINT8[buffersize * 4];

	for(uint t=0; t<buffersize; ++t)
	{
		((uint32 *)pPixels)[t] = color;
	}
	impl = new TextureImpl(pPixels, size);
	mScale = Vec2(mSize) / Vec2(size);
	SafeDeleteArray(pPixels);
}

//////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
	SafeDelete(impl);
	sAllTextures.Remove(this);
}

//////////////////////////////////////////////////////////////////////

void Texture::Activate()
{
	if(impl != null)
	{
		glBindTexture(GL_TEXTURE_2D, impl->mID);
	}
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::FromFBID(uint64 id, int width, int height)
{
	return Texture::Create(width, height, Color::Magenta);
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Load(char const *name)
{
	string n = ToLower(name);
	for(Texture *t = sAllTextures.head; t != null; t = t->next)
	{
		if(t->mName == name)
		{
			t->AddRef();
			TRACE("TEXREFERENCE (%d) %s\n", t->mRefCount, t->mName.c_str());
			return t;
		}
	}
	return new Texture(name);
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Create(int width, int height, Color color)
{
	return new Texture(width, height, color);
}
