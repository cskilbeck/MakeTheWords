//////////////////////////////////////////////////////////////////////

#include "pch.h"
#import "ImageFetcher.h"

extern bool LoadPNGFile(char const *filename, uint8 * &pixels, Size2D &size, Size2D &tsize);

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

Texture::Texture()
	: RefCount()
	, mName("EMPTY")
	, mSize(0,0)
	, mScale(0,0)
	, impl(null)
{
	sAllTextures.AddTail(this);
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(char const *name)
	: RefCount()
	, mName(ToLower(name))
	, impl(null)
{
	sAllTextures.AddTail(this);
	
	uint8_t *pixels;
	Size2D tsize;
	Size2D imageSize;

	if(LoadPNGFile(name, pixels, imageSize, tsize))
	{
		mScale = Vec2(imageSize) / Vec2(tsize);
		impl = new TextureImpl(pixels, tsize);
		mSize = imageSize;
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
	sAllTextures.AddTail(this);
	
	TRACE("CreateTexture %d,%d:%08x\n", width, height, (uint32)color);
	
	Size2D size(mSize);
	
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
	sAllTextures.Remove(this);
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

Texture *Texture::FromFBID(uint64 FBID, int width, int height)
{
	Texture *t = new Texture();
	NSString *resourceAddress = [[NSString alloc] initWithFormat:@"https://graph.facebook.com/%llu/picture?width=%d&height=%d", FBID, width, height];

	ImageFetcher *fetcher = [[ImageFetcher alloc] init];
	[fetcher fetchImageWithUrl:resourceAddress andCompletionBlock:^(UIImage *image)
	{
		if (image == nil)
		{
			return false;
		}
	 
		t->mSize = Size2D(width, height);
		t->mScale = Vec2::one;
	 
		CGSize newSize;
		newSize.width = width;
		newSize.height = height;
	 
		UIGraphicsBeginImageContextWithOptions(newSize, NO, 0.0);
		[image drawInRect:CGRectMake(0, 0, newSize.width, newSize.height)];
		UIImage *resizednewImage = UIGraphicsGetImageFromCurrentImageContext();
		UIGraphicsEndImageContext();
	 
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		void *imageData = malloc( height * width * 4 );
		CGContextRef context = CGBitmapContextCreate( imageData, width, height, 8, 4 * width, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
		CGColorSpaceRelease( colorSpace );
		CGContextClearRect( context, CGRectMake( 0, 0, width, height ) );
		CGContextDrawImage( context, CGRectMake( 0, 0, width, height ), resizednewImage.CGImage );

		t->mScale = Vec2::one;
		t->impl = new TextureImpl((uint8 *)imageData, t->mSize);
		t->mName = Format("Profile%llu", FBID);
		CGContextRelease(context);
	
		free(imageData);
		[image release];

		return true;
	 }];
	
	return t;
}

//////////////////////////////////////////////////////////////////////

Texture *Texture::Create(int width, int height, Color color)
{
	return new Texture(width, height, color);
}
