//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Win32.h"
#include "WICTextureLoader.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	linked_list<Texture, &Texture::mListNode> sAllTextures;
}

extern ID3D11Device *			gDevice;
extern ID3D11DeviceContext *	gContext;

//////////////////////////////////////////////////////////////////////

struct Texture::TextureImpl
{
	TextureImpl(char const *name)
		: mTexture2D(null)
		, mShaderResourceView(null)
	{
		mTextureDesc.MipLevels = 0;
		mTextureDesc.Width = 0;
		mTextureDesc.Height = 0;
		wstring w(WideStringFromString(name));
		if(!FAILED(CreateWICTextureFromFile(gDevice, gContext, w.c_str(), (ID3D11Resource **)&mTexture2D, &mShaderResourceView)))
		{
			mTexture2D->GetDesc(&mTextureDesc);
		}
	}

	TextureImpl(int width, int height, Color color)
		: mTexture2D(null)
		, mShaderResourceView(null)
	{
		mTextureDesc.MipLevels = 0;
		mTextureDesc.Width = 0;
		mTextureDesc.Height = 0;

		UINT8 *pPixels = new UINT8[width * height * 4];
		for(int y = 0; y < height; ++y)
		{
			UINT32 *row = (UINT32 *)(pPixels + y * width * 4);
			for(int x = 0; x < width; ++x)
			{
				row[x] = color.mColor;
			}
		}
		D3D11_SUBRESOURCE_DATA data[1];
		data[0].pSysMem = (void *)pPixels;
		data[0].SysMemPitch = width * 4;
		data[0].SysMemSlicePitch = 0;

		CD3D11_TEXTURE2D_DESC desc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1);
		HRESULT hr = gDevice->CreateTexture2D(&desc, data, &mTexture2D);

		Delete(pPixels);

		if(!FAILED(hr))
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = desc.MipLevels;
			srvDesc.Texture2D.MostDetailedMip = desc.MipLevels -1;	
			HRESULT hr = gDevice->CreateShaderResourceView(mTexture2D, &srvDesc, &mShaderResourceView);

			if(FAILED(hr))
			{
				::Release(mTexture2D);
			}
		}
	}

	~TextureImpl()
	{
		::Release(mShaderResourceView);
		::Release(mTexture2D);
	}

	ID3D11Texture2D	*			mTexture2D;
	ID3D11ShaderResourceView *	mShaderResourceView;
	D3D11_TEXTURE2D_DESC		mTextureDesc;
};

//////////////////////////////////////////////////////////////////////

Texture::Texture(char const *name)
	: RefCount()
	, mName(name)
	, impl(new TextureImpl(name))
{
	mSize = Size2D(impl->mTextureDesc.Width, impl->mTextureDesc.Height);
	mScale = Vec2::one;
	TRACE("LoadTexture %s %d,%d [%d,%d] : {%f,%f}\n", mName.c_str(), mSize.w, mSize.h, mSize.w, mSize.h, mScale.x, mScale.y);
	sAllTextures.push_back(this);
}

//////////////////////////////////////////////////////////////////////

Texture::Texture(int width, int height, Color color)
	: RefCount()
	, mName(Format("%dX%dX%08x", width, height, color))
	, impl(new TextureImpl(width, height, color))
	, mSize(width, height)
	, mScale(Vec2::one)
{
	TRACE("CreateTexture %d,%d:%08x\n", width, height, color);
	sAllTextures.push_back(this);
}

//////////////////////////////////////////////////////////////////////

Texture::~Texture()
{
	TRACE("Delete Texture %s\n", mName.c_str());
	mName.clear();
	sAllTextures.remove(this);
	Delete(impl);
}

//////////////////////////////////////////////////////////////////////

void Texture::Activate()
{
	if(impl != null)
	{
		gContext->PSSetShaderResources(0, 1, &impl->mShaderResourceView);
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
	for(auto &t: sAllTextures)
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

