//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <gl/GL.h>
#include "Win32.h"

//////////////////////////////////////////////////////////////////////

#pragma pack(push,1)

struct SpriteVertex
{
	Vec2 mPos;
	Vec2 mTexCoord;
	Color mColor;

	void Set(Vec2 const &pos, Vec2 const &texCoord, Color color)
	{
		mPos = pos;
		mTexCoord = texCoord;
		mColor = color;
	}
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////

struct Projection
{
	float mMatrix[16];
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

	int			mLength;
	int			mVertexBase;
	Texture *	mTexture;
	float		mRotation;
	Vec2		mScale;
	Vec2		mOrigin;
	bool		mTransform;
	Type		mType;
};

//////////////////////////////////////////////////////////////////////

struct SpriteList::SpriteListImpl
{
	SpriteListImpl();
	~SpriteListImpl();

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

	SpriteVertex *					mSpriteVerts[2];
	SpriteVertex *					mCurrentVertBuffer;
	SpriteVertex *					mCurrentVert;
	SpriteVertex *					mEndVert;
	int								mCurrentVertBufferIndex;

	SpriteVertex					mFanBase[2];

	void StartNewRun(SpriteRun::Type type);
	void Begin(SpriteRun::Type type);
	void EndRun();

	void Map();
	void UnMap();
	void ToggleVertBuffer();

	void Submit();

	bool Init(int maxSprites, int maxRuns);

	void AddPoint(Vec2 const &pos, Vec2 const &uv, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Vec2 const &uvTopLeft, Vec2 const &uvBottomRight, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Point2D const &uvTopLeft, Point2D const &uvBottomRight, Color color);

	void AddSprite(Vec2 const &pos, Vec2 const &size, Color color);

	static void Open();
	static void Close();

	static List<SpriteList>			sAllSpriteLists;
	static Texture *				sWhiteTexture;
};

//////////////////////////////////////////////////////////////////////

List<SpriteList>		SpriteList::SpriteListImpl::sAllSpriteLists;
Texture *				SpriteList::SpriteListImpl::sWhiteTexture = null;

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
		Vec2 suv(uv * mCurrentTexture->Scale());

		switch (mCurrentSpriteRun->mType)
		{
		case SpriteRun::kFan:
			if(mCurrentSpriteRun->mLength == 0)
			{
				mFanBase[0].Set(pos, suv, color);
			}
			else if(mCurrentSpriteRun->mLength == 1)
			{
				mFanBase[1].Set(pos, suv, color);
			}
			else
			{
				mCurrentVert[0] = mFanBase[0];
				mCurrentVert[1] = mFanBase[1];
				mCurrentVert[2].Set(pos, suv, color);
				mFanBase[1] = mCurrentVert[2];
				mCurrentVert += 3;
			}
			mCurrentSpriteRun->mLength++;
			break;

		case SpriteRun::kStrip:
			mCurrentVert->Set(pos, suv, color);
			mCurrentSpriteRun->mLength++;
			++mCurrentVert;
			break;

		case SpriteRun::kLines:
			mCurrentVert->Set(pos, suv, color);
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

		Vec2 const &bruv = mCurrentTexture->Scale();

		Vec2 tluv(0, 0);
		Vec2 truv(bruv.x, 0);
		Vec2 bluv(0, bruv.y);

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

		Vec2 const &scale = mCurrentTexture->Scale();

		truv *= scale;
		bluv *= scale;

		mCurrentVert[0].Set(pos, uvTopLeft, color);
		mCurrentVert[1].Set( tr, truv, color);
		mCurrentVert[2].Set( bl, bluv, color);

		mCurrentVert[3].Set( tr, truv, color);
		mCurrentVert[4].Set( bl, bluv, color);
		mCurrentVert[5].Set( br, uvBottomRight * scale, color);

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

		tluv *= mCurrentTexture->Scale();
		truv *= mCurrentTexture->Scale();
		bruv *= mCurrentTexture->Scale();
		bluv *= mCurrentTexture->Scale();

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

void SpriteList::SpriteListImpl::Open()
{
	sWhiteTexture = Texture::Create(8, 8, 0xffffffff);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Close()
{
	SafeRelease(sWhiteTexture);
}

//////////////////////////////////////////////////////////////////////*

SpriteList::SpriteListImpl::~SpriteListImpl()
{
	mCurrentIndex = 0;
	mCurrentVertBufferIndex = 0;

	SafeDeleteArray(mSpriteRuns);
	SafeDeleteArray(mSpriteVerts[0]);
	SafeDeleteArray(mSpriteVerts[1]);
}

//////////////////////////////////////////////////////////////////////*

SpriteList::SpriteListImpl::SpriteListImpl()
{
	mSpriteRuns = null;
	mSpriteVerts[0] = null;
	mSpriteVerts[1] = null;
	mMaxRuns = 0;
	mMaxSprites = 0;
	mDirtyTexture = false;
	mDirtyTransform = false;
	mCurrentIndex = 0;
	mCurrentVertBufferIndex = 0;
	mCurrentVert = null;
	mCurrentVertBuffer = null;
	mCurrentTexture = null;
	mRunType = SpriteRun::kInvalid;
}

//////////////////////////////////////////////////////////////////////

bool SpriteList::SpriteListImpl::Init(int maxSprites, int maxRuns)
{
	mMaxSprites = maxSprites;
	mMaxRuns = (maxRuns == 0) ? maxSprites : maxRuns;

	mSpriteRuns = new SpriteRun[mMaxRuns];
	mSpriteVerts[0] = new SpriteVertex[mMaxSprites * 4];
	mSpriteVerts[1] = new SpriteVertex[mMaxSprites * 4];

	Map();
	return true;
}

//////////////////////////////////////////////////////////////////////*

void SpriteList::SpriteListImpl::Map()
{
	mCurrentSpriteRun = null;

	mDirtyTexture = false;
	mDirtyTransform = false;

	mCurrentScale = Vec2::one;
	mCurrentRotation = 0;
	mCurrentOrigin = Vec2::zero;

	mCurrentIndex = 0;

	mCurrentVertBuffer = mSpriteVerts[mCurrentVertBufferIndex];
	mCurrentVert = mCurrentVertBuffer;
	mEndVert = mCurrentVert + (mMaxSprites * 6);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::ToggleVertBuffer()
{
	mCurrentVertBufferIndex = 1 - mCurrentVertBufferIndex;
}

//////////////////////////////////////////////////////////////////////*

void SpriteList::SpriteListImpl::Submit()
{
	SpriteVertex *verts = mSpriteVerts[mCurrentVertBufferIndex];

	GLfloat modelView[16];

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(0, Screen::Width(), Screen::Height(), 0, 0, 1);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

	glEnable(GL_LINE_SMOOTH);

	Texture *activeTexture = null;
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(SpriteVertex), &verts->mColor);
	glTexCoordPointer(2, GL_FLOAT, sizeof(SpriteVertex), &verts->mTexCoord.x);
	glVertexPointer(2, GL_FLOAT, sizeof(SpriteVertex), &verts->mPos.x);

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
				glLoadMatrixf(modelView);
				glTranslatef(run.mOrigin.x, run.mOrigin.y, 0);
				glRotatef(run.mRotation * 180.0f / PI, 0.0f, 0.0f, 1.0f);
				glScalef(run.mScale.x, run.mScale.y, 0);
				glTranslatef(-run.mOrigin.x, -run.mOrigin.y, 0);
			}

			switch(run.mType)
			{
			case SpriteRun::kSprite:
				glDrawArrays(GL_TRIANGLES, run.mVertexBase, run.mLength * 6);
				break;

			case SpriteRun::kFan:
				glDrawArrays(GL_TRIANGLES, run.mVertexBase, (run.mLength - 2) * 3);
				break;

			case SpriteRun::kStrip:
				glDrawArrays(GL_TRIANGLE_STRIP, run.mVertexBase, run.mLength);
				break;

			case SpriteRun::kLines:
				glDrawArrays(GL_LINE_STRIP, run.mVertexBase, run.mLength);
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
	if(SpriteListImpl::sAllSpriteLists.IsEmpty())
	{
		SpriteListImpl::Open();
	}
	SpriteListImpl::sAllSpriteLists.AddHead(this);
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

	SpriteListImpl::sAllSpriteLists.Remove(this);
	if(SpriteListImpl::sAllSpriteLists.IsEmpty())
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
		impl->mCurrentTexture = texture;
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
	SpriteListImpl::sAllSpriteLists.ForEach([&] (SpriteList *s)
	{
		s->Submit();
	});
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
