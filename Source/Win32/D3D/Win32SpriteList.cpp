//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Win32.h"
#include "d3d.h"

//////////////////////////////////////////////////////////////////////

struct SpriteVertex
{
	Vec2 mPos;
	Vec2 mTexCoord;
	Color mColor;

	void Set(Vec2 const &pos, Vec2 const &texCoord, ::Color color)
	{
		mPos = pos;
		mTexCoord = texCoord;
		mColor = color;
	}
};

//////////////////////////////////////////////////////////////////////

struct Projection
{
	XMMATRIX mMatrix;
};

//////////////////////////////////////////////////////////////////////

struct SpriteRun
{
	enum Type
	{
		kInvalid,
		kSprite,
		kFan,
		kStrip,
		kLines
	};

	int mLength;
	int mVertexBase;
	Texture *mTexture;
	float mRotation;
	Vec2 mScale;
	Vec2 mOrigin;
	bool mTransform;
	Type mType;
};

//////////////////////////////////////////////////////////////////////

struct SpriteList::SpriteListImpl
{
	bool Init(int maxSprites = 4096, int maxRuns = 0);

	void SetTexture(Texture *texture);
	void SetScale(Vec2 const &scale);
	void SetRotation(float radians);
	void SetOrigin(Vec2 const &origin);
	void ResetTransform();

	void AddSprite(Vec2 const &pos, Vec2 const &size, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Vec2 const &uvTopLeft, Vec2 const &uvBottomRight, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Point2D const &uvTopLeft, Point2D const &uvBottomRight, Color color);
	void BeginFan();
	void BeginStrip();
	void BeginLines();
	void AddPoint(Vec2 const &pos, Vec2 const &uv, Color color);

	void Submit();

	static void SubmitAll();

	SpriteListImpl();
	~SpriteListImpl();

	static HRESULT Open();
	static void Close();

	int								mMaxRuns;
	int								mMaxSprites;

	SpriteRun::Type					mRunType;
	bool							mDirtyTexture;
	bool							mDirtyTransform;

	Vec2							mCurrentScale;
	Vec2							mCurrentOrigin;
	float							mCurrentRotation;
	Texture *						mCurrentTexture;
	SpriteRun *						mCurrentSpriteRun;
	int								mCurrentIndex;

	SpriteRun *						mSpriteRuns;

	SpriteVertex *					mSpriteVerts0;
	SpriteVertex *					mSpriteVerts1;

	SpriteVertex *					mCurrentVertBuffer;
	SpriteVertex *					mCurrentVert;
	SpriteVertex *					mEndVert;
	int								mCurrentVertBufferIndex;

	SpriteVertex					mFanBase[2];

	ID3D11Buffer *					mVertexBuffer[2];
	ID3D11Buffer *					mCBProjection;
	ID3D11RasterizerState *			mRasterizerState;
	ID3D11BlendState *				mBlendState;
	ID3D11SamplerState *			mSamplerLinear;

	static ID3D11InputLayout *		spVertexLayout;
	static ID3D11PixelShader *		spPixelShader;
	static ID3D11VertexShader *		spVertexShader;
	static Texture *				sWhiteTexture;

	static linked_list<SpriteList, &SpriteList::mListNode>	sAllSpriteLists;

	void StartNewRun(SpriteRun::Type type);
	void Begin(SpriteRun::Type type);
	void EndRun();

	void Map();
	void UnMap();
	void ToggleVertBuffer();
};

//////////////////////////////////////////////////////////////////////

linked_list<SpriteList, &SpriteList::mListNode> SpriteList::SpriteListImpl::sAllSpriteLists;

//////////////////////////////////////////////////////////////////////

ID3D11InputLayout *			SpriteList::SpriteListImpl::spVertexLayout = null;
ID3D11PixelShader *			SpriteList::SpriteListImpl::spPixelShader = null;
ID3D11VertexShader *		SpriteList::SpriteListImpl::spVertexShader = null;
Texture *					SpriteList::SpriteListImpl::sWhiteTexture = null;

//////////////////////////////////////////////////////////////////////

HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
extern ID3D11Device *gDevice;
extern ID3D11DeviceContext *gContext;

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Begin(SpriteRun::Type type)
{
	if(mCurrentIndex < mMaxRuns)
	{
		mRunType = type;
		mCurrentSpriteRun = mSpriteRuns + mCurrentIndex;
		mCurrentSpriteRun->mScale = mCurrentScale;
		mCurrentSpriteRun->mOrigin = mCurrentOrigin;
		mCurrentSpriteRun->mRotation = mCurrentRotation;
		mCurrentSpriteRun->mTexture = mCurrentTexture;
		mCurrentSpriteRun->mTransform = mDirtyTransform;
		mCurrentSpriteRun->mLength = 0;
		mCurrentSpriteRun->mType = type;
		mCurrentSpriteRun->mVertexBase = mCurrentVert - mCurrentVertBuffer;	// Hmmm
		mDirtyTexture = false;
		mDirtyTransform = false;
		++mCurrentIndex;
	}
	else
	{
		mCurrentVert = null;
		mEndVert = null;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::StartNewRun(SpriteRun::Type type)
{
	if(mRunType != type || mDirtyTexture || mDirtyTransform || mCurrentSpriteRun == null)	// other kinds of dirtiness to follow - fix how this works...
	{
		Begin(type);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::AddPoint(Vec2 const &pos, Vec2 const &uv, Color color)
{
	if(mCurrentVert < mEndVert)
	{
		switch (mCurrentSpriteRun->mType)
		{
		case SpriteRun::kFan:
			if(mCurrentSpriteRun->mLength == 0)
			{
				mFanBase[0].Set(pos, uv, color);
			}
			else if(mCurrentSpriteRun->mLength == 1)
			{
				mFanBase[1].Set(pos, uv, color);
			}
			else
			{
				mCurrentVert[0] = mFanBase[0];
				mCurrentVert[1] = mFanBase[1];
				mCurrentVert[2].Set(pos, uv, color);
				mFanBase[1] = mCurrentVert[2];
				mCurrentVert += 3;
			}
			mCurrentSpriteRun->mLength++;
			break;

		case SpriteRun::kStrip:
		case SpriteRun::kLines:
			mCurrentVert->Set(pos, uv, color);
			mCurrentSpriteRun->mLength++;
			++mCurrentVert;
			break;

		default:
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::AddSprite(Vec2 const &pos, Vec2 const &size, Color color)
{
	StartNewRun(SpriteRun::kSprite);

	if(mCurrentVert < mEndVert)
	{
		Vec2 tr(pos.x + size.x, pos.y);
		Vec2 br(pos.x + size.x, pos.y + size.y);
		Vec2 bl(pos.x, pos.y + size.y);

		Vec2 tluv(0,0);
		Vec2 truv(1,0);
		Vec2 bruv(1,1);
		Vec2 bluv(0,1);

		mCurrentVert[0].Set(pos, tluv, color);
		mCurrentVert[1].Set( tr, truv, color);
		mCurrentVert[2].Set( bl, bluv, color);

		mCurrentVert[3].Set( tr, truv, color);
		mCurrentVert[4].Set( bl, bluv, color);
		mCurrentVert[5].Set( br, bruv, color);

		mCurrentSpriteRun->mLength++;
		mCurrentVert += 6;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::AddSprite(Vec2 const &pos, Vec2 const &size, Vec2 const &uvTopLeft, Vec2 const &uvBottomRight, Color color)
{
	StartNewRun(SpriteRun::kSprite);

	if(mCurrentVert < mEndVert)
	{
		Vec2 tr(pos.x + size.x, pos.y);
		Vec2 br(pos.x + size.x, pos.y + size.y);
		Vec2 bl(pos.x, pos.y + size.y);

		Vec2 truv(uvBottomRight.x, uvTopLeft.y);
		Vec2 bluv(uvTopLeft.x, uvBottomRight.y);

		mCurrentVert[0].Set(pos, uvTopLeft, color);
		mCurrentVert[1].Set( tr, truv, color);
		mCurrentVert[2].Set( bl, bluv, color);

		mCurrentVert[3].Set( tr, truv, color);
		mCurrentVert[4].Set( bl, bluv, color);
		mCurrentVert[5].Set( br, uvBottomRight, color);

		mCurrentSpriteRun->mLength++;
		mCurrentVert += 6;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::AddSprite(Vec2 const &pos, Vec2 const &size, Point2D const &uvTopLeft, Point2D const &uvBottomRight, Color color)
{
	StartNewRun(SpriteRun::kSprite);

	if(mCurrentVert < mEndVert)
	{
		float tw = (float)mCurrentTexture->Width();
		float th = (float)mCurrentTexture->Height();

		Vec2 tluv(uvTopLeft.x / tw, uvTopLeft.y / th);
		Vec2 bruv(uvBottomRight.x / tw, uvBottomRight.y / th);
		Vec2 truv(bruv.x, tluv.y);
		Vec2 bluv(tluv.x, bruv.y);

		Vec2 tr(pos.x + size.x, pos.y);
		Vec2 br(pos.x + size.x, pos.y + size.y);
		Vec2 bl(pos.x, pos.y + size.y);

		mCurrentVert[0].Set(pos, tluv, color);
		mCurrentVert[1].Set( tr, truv, color);
		mCurrentVert[2].Set( bl, bluv, color);

		mCurrentVert[3].Set( tr, truv, color);
		mCurrentVert[4].Set( bl, bluv, color);
		mCurrentVert[5].Set( br, bruv, color);

		mCurrentSpriteRun->mLength++;
		mCurrentVert += 6;
	}
}


//////////////////////////////////////////////////////////////////////

SpriteList::SpriteListImpl::SpriteListImpl()
{
	mDirtyTexture = false;
	mDirtyTransform = false;

	mCurrentIndex = 0;
	mCurrentVertBufferIndex = 0;

	mCurrentVert = null;
	mCurrentVertBuffer = null;

	mCurrentTexture = null;

	mSpriteRuns = null;
	mSpriteVerts0 = null;
	mSpriteVerts1 = null;

	mVertexBuffer[0] = null;
	mVertexBuffer[1] = null;
	mCBProjection = null;
	mRasterizerState = null;
	mBlendState = null;
	mSamplerLinear = null;

	mRunType = SpriteRun::kInvalid;
}

//////////////////////////////////////////////////////////////////////

SpriteList::SpriteListImpl::~SpriteListImpl()
{
	UnMap();

	mCurrentIndex = 0;
	mCurrentVertBufferIndex = 0;

	SafeDeleteArray(mSpriteRuns);
	SafeDeleteArray(mSpriteVerts0);
	SafeDeleteArray(mSpriteVerts1);

	SafeRelease(mVertexBuffer[0]);
	SafeRelease(mVertexBuffer[1]);

	SafeRelease(mCBProjection);
	SafeRelease(mRasterizerState);
	SafeRelease(mBlendState);
	SafeRelease(mSamplerLinear);
}

//////////////////////////////////////////////////////////////////////

HRESULT SpriteList::SpriteListImpl::Open()
{
	if(spVertexShader == null)
	{
		HRESULT hr;
		ID3DBlob *pVSBlob = NULL;

		hr = CompileShaderFromFile(L"Main.fx", "SpriteVertexShader", "vs_4_0_level_9_1", &pVSBlob);
		if(FAILED(hr))
		{
			assert(false);
			Close();
			return hr;
		}

		size_t size = pVSBlob->GetBufferSize();
		void *buffer = pVSBlob->GetBufferPointer();

		//size_t size1;
		//void *buffer1 = LoadFile(L"svs.bin", &size1);

		hr = gDevice->CreateVertexShader(buffer, size, NULL, &spVertexShader);

		if(FAILED(hr))
		{	
			pVSBlob->Release();
			assert(false);
			Close();
			return hr;
		}

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "SV_Position",	0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",			0, DXGI_FORMAT_R8G8B8A8_UNORM,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);

		hr = gDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &spVertexLayout);
		pVSBlob->Release();
		if(FAILED(hr))
		{
			assert(false);
			Close();
			return hr;
		}
	}

	if(spPixelShader == null)
	{
		ID3DBlob *pPSBlob = NULL;
		HRESULT hr = CompileShaderFromFile(L"Main.fx", "SpritePixelShader", "ps_4_0_level_9_1", &pPSBlob);
		if(FAILED(hr))
		{
			assert(false);
			Close();
			return hr;
		}

		hr = gDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &spPixelShader);
		pPSBlob->Release();
		if(FAILED(hr))
		{
			assert(false);
			Close();
			return hr;
		}
	}
	sWhiteTexture = Texture::Create(8, 8, Color::White);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Close()
{
	SafeRelease(sWhiteTexture);
	SafeRelease(spVertexLayout);
	SafeRelease(spVertexShader);
	SafeRelease(spPixelShader);
}

//////////////////////////////////////////////////////////////////////

bool SpriteList::SpriteListImpl::Init(int maxSprites, int maxRuns)
{
	mMaxSprites = maxSprites;
	mMaxRuns = (maxRuns == 0) ? maxSprites : maxRuns;

	mSpriteRuns = new SpriteRun[mMaxRuns];

	mSpriteVerts0 = new SpriteVertex[mMaxSprites * 4];
	mSpriteVerts1 = new SpriteVertex[mMaxSprites * 4];

	mCurrentScale = Vec2(1,1);
	mCurrentRotation = 0;
	mCurrentOrigin = Vec2(0,0);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(SpriteVertex) * mMaxSprites * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	InitData.pSysMem = mSpriteVerts0;
	DXB(gDevice->CreateBuffer(&bd, &InitData, &mVertexBuffer[0]));

	InitData.pSysMem = mSpriteVerts1;
	DXB(gDevice->CreateBuffer(&bd, &InitData, &mVertexBuffer[1]));

	bd.ByteWidth = sizeof(Projection);
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	DXB(gDevice->CreateBuffer(&bd, NULL, &mCBProjection));

	CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.AntialiasedLineEnable = TRUE;
	DXB(gDevice->CreateRasterizerState(&rasterizerDesc, &mRasterizerState));

	CD3D11_BLEND_DESC blendDesc(D3D11_DEFAULT);
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc;
	rtBlendDesc.BlendEnable = true;
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;

	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;


	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.RenderTarget[0] = rtBlendDesc;

	DXB(gDevice->CreateBlendState(&blendDesc, &mBlendState));

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	DXB(gDevice->CreateSamplerState(&sampDesc, &mSamplerLinear));

	Map();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Map()
{
	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hResult = gContext->Map(mVertexBuffer[mCurrentVertBufferIndex], 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	if(!FAILED(hResult))
	{
		mCurrentVertBuffer = (SpriteVertex *)resource.pData;
		mCurrentVert = mCurrentVertBuffer;
		mEndVert = mCurrentVert + (mMaxSprites * 6);
	}
	else
	{
		assert(false);
		mCurrentVertBuffer = null;
		mCurrentVert = null;
		mEndVert = null;
	}
	mCurrentSpriteRun = null;
	mDirtyTexture = false;
	mDirtyTransform = false;
	mCurrentScale = Vec2::one;
	mCurrentRotation = 0;
	mCurrentOrigin = Vec2::zero;
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::UnMap()
{
	if(mCurrentVertBuffer != null)
	{
		gContext->Unmap(mVertexBuffer[mCurrentVertBufferIndex], 0);
		mCurrentVert = null;
		mEndVert = null;
		mCurrentVertBuffer = null;
	}
}

//////////////////////////////////////////////////////////////////////

XMVECTOR XMVec2(Vec2 const &src)
{
	return XMLoadFloat2((CONST XMFLOAT2*)&src);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::ToggleVertBuffer()
{
	mCurrentIndex = 0;
	mCurrentVertBufferIndex = 1 - mCurrentVertBufferIndex;
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Submit()
{
	UnMap();

	Projection projection;

	float halfWidth = 2.0f / Screen::Width();
	float halfHeight = -2.0f / Screen::Height();

	XMMATRIX matrix( halfWidth,	0.0f,		0.0f,	0.0f,
					 0.0f,		halfHeight,	0.0f,	0.0f,
					 0.0f,		0.0f,		1.0f,	0.0f,
					-1.0f,		1.0f,		0.0f,	1.0f);

	projection.mMatrix = XMMatrixTranspose((XMMATRIX)matrix);

	UINT stride = sizeof(SpriteVertex);
	UINT offset = 0;

	gContext->UpdateSubresource(mCBProjection, 0, NULL, &projection, 0, 0);
	gContext->VSSetShader(spVertexShader, NULL, 0);
	gContext->PSSetShader(spPixelShader, NULL, 0);
	gContext->PSSetSamplers(0, 1, &mSamplerLinear);
	gContext->RSSetState(mRasterizerState);
	gContext->OMSetBlendState(mBlendState, NULL, 0xffffffff);
	gContext->VSSetConstantBuffers(0, 1, &mCBProjection);
	gContext->IASetInputLayout(spVertexLayout);
	gContext->IASetVertexBuffers(0, 1, &mVertexBuffer[mCurrentVertBufferIndex], &stride, &offset);
	gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Texture *activeTexture = null;

	for(int i=0; i<mCurrentIndex; ++i)
	{
		SpriteRun &run = mSpriteRuns[i];
		if(run.mLength > 0)
		{
			if(run.mTexture != activeTexture)
			{
				run.mTexture->Activate();
				activeTexture = run.mTexture;
			}

			if(run.mTransform)
			{
				XMVECTOR scalingOrigin = XMVec2(run.mOrigin);
				float scaleOrientation = 0;
				XMVECTOR scale = XMVec2(run.mScale);
				XMVECTOR rotationOrigin = XMVec2(run.mOrigin);
				float rotation = run.mRotation;
				XMVECTOR translate = XMVec2(Vec2::zero);
				XMMATRIX m2d = XMMatrixTransformation2D(scalingOrigin, scaleOrientation, scale, rotationOrigin, rotation, translate);
				Projection dynamic;
				dynamic.mMatrix = XMMatrixTranspose(m2d * matrix);
				gContext->UpdateSubresource(mCBProjection, 0, NULL, &dynamic, 0, 0);
				gContext->VSSetConstantBuffers(0, 1, &mCBProjection);
			}

			switch(run.mType)
			{
			case SpriteRun::kSprite:
				gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				gContext->Draw(run.mLength * 6, run.mVertexBase);
				break;

			case SpriteRun::kFan:
				gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				gContext->Draw((run.mLength - 2) * 3, run.mVertexBase);
				break;

			case SpriteRun::kStrip:
				gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				gContext->Draw(run.mLength, run.mVertexBase);
				break;
			
			case SpriteRun::kLines:
				gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
				gContext->Draw(run.mLength, run.mVertexBase);
				break;
			}
		}
	}

	ToggleVertBuffer();

	Map();
}

//////////////////////////////////////////////////////////////////////
// SPRITELIST - not platform specific...
//////////////////////////////////////////////////////////////////////

SpriteList::SpriteList()
	: RefCount()
	, impl(new SpriteListImpl)
{
	if(SpriteListImpl::sAllSpriteLists.empty())
	{
		SpriteListImpl::Open();
	}
	SpriteListImpl::sAllSpriteLists.push_back(this);
}

//////////////////////////////////////////////////////////////////////

bool SpriteList::Init(int maxSprites, int maxRuns)
{
	return impl->Init(maxSprites, maxRuns);
}

//////////////////////////////////////////////////////////////////////

SpriteList::~SpriteList()
{
	SafeDelete(impl);

	SpriteListImpl::sAllSpriteLists.remove(this);
	if(SpriteListImpl::sAllSpriteLists.empty())
	{
		SpriteListImpl::Close();
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::Submit()
{
	impl->Submit();
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SetScale(Vec2 const &scale)
{
	if(scale != impl->mCurrentScale)
	{
		impl->mCurrentScale = scale;
		impl->mDirtyTransform = true;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SetRotation(float radians)
{
	if(radians != impl->mCurrentRotation)
	{
		impl->mCurrentRotation = radians;
		impl->mDirtyTransform = true;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SetOrigin(Vec2 const &origin)
{
	if(origin != impl->mCurrentOrigin)
	{
		impl->mCurrentOrigin = origin;
		impl->mDirtyTransform = true;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::ResetTransform()
{
	SetScale(Vec2(1,1));
	SetRotation(0);
	SetOrigin(Vec2(0,0));
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SetBlendingMode(SpriteList::eBlendMode mode)
{
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SetTexture(Texture *texture)
{
	if(texture != impl->mCurrentTexture)
	{
		impl->mDirtyTexture = true;
		impl->mCurrentTexture = (texture != null) ? texture : SpriteListImpl::sWhiteTexture;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::BeginFan()
{
	impl->Begin(SpriteRun::kFan);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::BeginStrip()
{
	impl->Begin(SpriteRun::kStrip);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::BeginLines()
{
	impl->Begin(SpriteRun::kLines);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddPoint(Vec2 const &pos, Vec2 const &uv, Color color)
{
	impl->AddPoint(pos, uv, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddSprite(Vec2 const &pos, Vec2 const &size, Color color)
{
	impl->AddSprite(pos, size, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddSprite(Vec2 const &pos, Vec2 const &size, Point2D const &uvTopLeft, Point2D const &uvBottomRight, Color color)
{
	impl->AddSprite(pos, size, uvTopLeft, uvBottomRight, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddSprite(Vec2 const &pos, Vec2 const &size, Vec2 const &uvTopLeft, Vec2 const &uvBottomRight, Color color)
{
	impl->AddSprite(pos, size, uvTopLeft, uvBottomRight, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SubmitAll()
{
	for(auto &l: SpriteListImpl::sAllSpriteLists)
	{
		l.Submit();
	}
}

//////////////////////////////////////////////////////////////////////

Texture *SpriteList::WhiteTexture()
{
	return SpriteListImpl::sWhiteTexture;
}

//////////////////////////////////////////////////////////////////////

SpriteList *SpriteList::Create(int maxSprites, int maxRuns)
{
	SpriteList *s = new SpriteList();
	s->Init(maxSprites, maxRuns);
	return s;
}
