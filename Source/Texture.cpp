//////////////////////////////////////////////////////////////////////

#include "pch.h"

extern bool LoadPNGFile(char const *filename, uint8 * &pixels, Size2D &size, Size2D &tsize);

//////////////////////////////////////////////////////////////////////

namespace
{
	linked_list<Texture, &Texture::mListNode> sAllTextures;
}

//////////////////////////////////////////////////////////////////////

struct Texture::TextureImpl
{
	TextureImpl(uint8 *pixels, Size2D size)
		: mID(0)
	{
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.w, size.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	}
	
	~TextureImpl()
	{
		glDeleteTextures(1, &mID);
		mID = 0;
	}
	
	uint	mID;
};

//////////////////////////////////////////////////////////////////////

static int sTextureSamplerID = 0;

void Texture::SetSamplerID(int id)
{
	sTextureSamplerID = id;
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(char const *name)
	: RefCount()
	, mName(ToLower(name))
	, impl(null)
{
	sAllTextures.push_back(this);
	
	uint8_t *pixels;
	Size2D tsize;

	if(LoadPNGFile(name, pixels, mSize, tsize))
	{
		mScale = Vec2(mSize) / Vec2(tsize);
		impl = new TextureImpl(pixels, mSize);
		TRACE("LoadTexture %s %d,%d [%d,%d] : {%f,%f}\n", mName.c_str(), mSize.w, mSize.h, tsize.w, tsize.h, mScale.x, mScale.y);
		SafeDeleteArray(pixels);
	}
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color)
	: RefCount()
	, mName(Format("SOLID%08x(%d,%d)", (uint32)color, width, height))
	, mSize(width, height)
	, impl(null)
{
	sAllTextures.push_back(this);
	
	TRACE("CreateTexture %d,%d:%08x\n", width, height, (uint32)color);
	
	Size2D size(mSize);
	size.w = NextPowerOf2(size.w);
	size.h = NextPowerOf2(size.h);
	
	size_t buffersize = size.w * size.h;
	
	uint8 *pPixels = new uint8[buffersize * 4];
	
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
	sAllTextures.remove(this);
}

//////////////////////////////////////////////////////////////////////

void Texture::Activate()
{
	if(impl != null)
	{
		glEnable(GL_TEXTURE0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, impl->mID);
		glUniform1i(sTextureSamplerID, 0);
	}
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Load(char const *name)
{
	string n = ToLower(name);
	for(auto &t : sAllTextures)
	{
		if(t.mName == name)
		{
			t.AddRef();
			TRACE("TEXREFERENCE (%d) %s\n", t.mRefCount, t.mName.c_str());
			return &t;
		}
	}
	return new Texture(name);
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Create(int width, int height, Color color)
{
	return new Texture(width, height, color);
}
