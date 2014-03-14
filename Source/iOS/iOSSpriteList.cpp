//////////////////////////////////////////////////////////////////////

#include "pch.h"
#import <GLKit/GLKit.h>

//////////////////////////////////////////////////////////////////////

#define OFFSETOF(type, field) (&((type *)0)->field)

//#define USE_VBOS

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
	
	int						mLength;
	int						mVertexBase;
	Texture *				mTexture;
	float					mRotation;
	Vec2					mScale;
	Vec2					mOrigin;
	bool					mTransform;
	Type					mType;
	SpriteList::eBlendMode	mBlendMode;
};

//////////////////////////////////////////////////////////////////////

struct SpriteList::SpriteListImpl
{
	SpriteListImpl();
	~SpriteListImpl();
	
	Vec2							mCurrentScale;
	Vec2							mCurrentOrigin;

	int								mMaxRuns;
	int								mMaxSprites;
	
	SpriteRun::Type					mRunType;
	bool							mDirtyTexture;
	bool							mDirtyBlendMode;
	bool							mDirtyTransform;
	
	float							mCurrentRotation;
	Texture *						mCurrentTexture;
	SpriteList::eBlendMode			mCurrentBlendMode;
	SpriteRun *						mCurrentSpriteRun;
	int								mCurrentIndex;
	
	SpriteRun *						mSpriteRuns;

	enum
	{
		kNumVertexBuffers = 3
	};

#if defined(USE_VBOS)
	uint
#else
	SpriteVertex *
#endif
									mVertexBuffer[kNumVertexBuffers];

	SpriteVertex *					mCurrentVertBuffer;
	SpriteVertex *					mCurrentVert;
	SpriteVertex *					mEndVert;
	int								mCurrentVertBufferIndex;
	
	SpriteVertex					mFanBase[2];

	bool IsDirty() const;
	void SetupBlend(SpriteList::eBlendMode mode);
	
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
	
	static void LoadShaders();
	static bool CompileShader(uint *shader, GLenum type, NSString *file);
	static bool LinkProgram(GLuint prog);
	static bool ValidateProgram(GLuint prog);
	
	static List<SpriteList>			sAllSpriteLists;
	static Texture *				sWhiteTexture;

	enum
	{
		ATTRIB_POSITION,
		ATTRIB_UV,
		ATTRIB_COLOR,
		NUM_ATTRIBUTES
	};
	
	enum
	{
		UNIFORM_VERTEX_SHADER_MATRIX,
		UNIFORM_FRAGMENT_SHADER_SAMPLER,
		NUM_UNIFORMS
	};

	static uint						mShader;
	static uint						mUniformID[NUM_UNIFORMS];
};

//////////////////////////////////////////////////////////////////////

List<SpriteList>		SpriteList::SpriteListImpl::sAllSpriteLists;
Texture *				SpriteList::SpriteListImpl::sWhiteTexture = null;
uint					SpriteList::SpriteListImpl::mShader;
uint					SpriteList::SpriteListImpl::mUniformID[SpriteList::SpriteListImpl::NUM_UNIFORMS];

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
		mCurrentSpriteRun->mBlendMode = mCurrentBlendMode;
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

bool SpriteList::SpriteListImpl::IsDirty() const
{
	return mDirtyTexture || mDirtyTransform || mDirtyBlendMode || mCurrentSpriteRun == null;
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::StartNewRun(SpriteRun::Type type)
{
	if(mRunType != type || IsDirty())
	{
		Begin(type);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::AddPoint(Vec2 const &pos, Vec2 const &uv, Color color)
{
	assert(mCurrentSpriteRun->mType != SpriteRun::kSprite);
	
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
	else
	{
		TRACE("?0\n");
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
		Vec2 truv(mCurrentTexture->Scale().x,0);
		Vec2 bruv(mCurrentTexture->Scale());
		Vec2 bluv(0,mCurrentTexture->Scale().y);
		
		mCurrentVert[0].Set(pos, tluv, color);
		mCurrentVert[1].Set( tr, truv, color);
		mCurrentVert[2].Set( bl, bluv, color);
		
		mCurrentVert[3].Set( tr, truv, color);
		mCurrentVert[4].Set( bl, bluv, color);
		mCurrentVert[5].Set( br, bruv, color);
		
		mCurrentSpriteRun->mLength++;
		mCurrentVert += 6;
	}
	else
	{
		TRACE("?1\n");
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
		
		Vec2 bruv(uvBottomRight * mCurrentTexture->Scale());
		Vec2 tluv(uvTopLeft * mCurrentTexture->Scale());
		
		Vec2 truv(bruv.x, tluv.y);
		Vec2 bluv(tluv.x, bruv.y);
		
		mCurrentVert[0].Set(pos, tluv, color);
		mCurrentVert[1].Set( tr, truv, color);
		mCurrentVert[2].Set( bl, bluv, color);
		
		mCurrentVert[3].Set( tr, truv, color);
		mCurrentVert[4].Set( bl, bluv, color);
		mCurrentVert[5].Set( br, bruv, color);
		
		mCurrentSpriteRun->mLength++;
		mCurrentVert += 6;
	}
	else
	{
		TRACE("?2\n");
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
	else
	{
		TRACE("?3\n");
	}
}

//////////////////////////////////////////////////////////////////////

static void DumpMatrix(GLKMatrix4 m)
{
	for(int y=0; y<4; ++y)
	{
		for(int x=0; x<4; ++x)
		{
			TRACE("%6.3f ", m.m[x + y * 4]);
		}
		TRACE("\n");
	}
	TRACE("\n");
}

void SpriteList::SpriteListImpl::Open()
{
	sWhiteTexture = Texture::Create(8, 8, 0xffffffff);
	LoadShaders();
	glUseProgram(mShader);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Close()
{
	if (mShader)
	{
		glDeleteProgram(mShader);
		mShader = 0;
	}
	SafeRelease(sWhiteTexture);
}

//////////////////////////////////////////////////////////////////////*

SpriteList::SpriteListImpl::~SpriteListImpl()
{
	mCurrentIndex = 0;
	mCurrentVertBufferIndex = 0;
	SafeDeleteArray(mSpriteRuns);
}

//////////////////////////////////////////////////////////////////////

SpriteList::SpriteListImpl::SpriteListImpl()
{
	mSpriteRuns = null;
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

void SpriteList::SpriteListImpl::LoadShaders()
{
	GLuint vertShader;
	GLuint fragShader;
	
    NSString *vertShaderPathname;
	NSString *fragShaderPathname;
    
    mShader = glCreateProgram();
    
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    if (!CompileShader(&vertShader, GL_VERTEX_SHADER, vertShaderPathname))
	{
        NSLog(@"Failed to compile vertex shader");
        return;
    }
    
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
	if (!CompileShader(&fragShader, GL_FRAGMENT_SHADER, fragShaderPathname))
	{
        NSLog(@"Failed to compile fragment shader");
        return;
    }

    glAttachShader(mShader, vertShader);
    glAttachShader(mShader, fragShader);
    
    glBindAttribLocation(mShader, ATTRIB_POSITION, "vertexPosition");
	glBindAttribLocation(mShader, ATTRIB_UV, "vertexTexCoord");
    glBindAttribLocation(mShader, ATTRIB_COLOR, "vertexColor");
    
    if (!LinkProgram(mShader))
	{
        NSLog(@"Failed to link program: %d", mShader);
        
        if (vertShader)
		{
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader)
		{
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (mShader)
		{
            glDeleteProgram(mShader);
            mShader = 0;
        }
		return;
    }
    
    mUniformID[UNIFORM_VERTEX_SHADER_MATRIX] = glGetUniformLocation(mShader, "matrix");
    mUniformID[UNIFORM_FRAGMENT_SHADER_SAMPLER] = glGetUniformLocation(mShader, "s_texture");
    
    if (vertShader)
	{
        glDetachShader(mShader, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader)
	{
        glDetachShader(mShader, fragShader);
        glDeleteShader(fragShader);
    }
}

//////////////////////////////////////////////////////////////////////

bool SpriteList::SpriteListImpl::CompileShader(uint *shader, GLenum type, NSString *file)
{
	GLint status;
	const GLchar *source;
	
	source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
	if (!source)
	{
		TRACE("Failed to load vertex shader");
		return false;
	}
	
	*shader = glCreateShader(type);
	glShaderSource(*shader, 1, &source, NULL);
	glCompileShader(*shader);
		
#if defined(DEBUG)
	GLint logLength;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = (GLchar *)malloc(logLength);
		glGetShaderInfoLog(*shader, logLength, &logLength, log);
		TRACE("Shader compile log:\n%s", log);
		free(log);
	}
#endif
		
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
	if (status == 0)
	{
		glDeleteShader(*shader);
		return false;
	}
	return true;
}
		
//////////////////////////////////////////////////////////////////////
		
bool SpriteList::SpriteListImpl::LinkProgram(GLuint prog)
{
	GLint status;
	glLinkProgram(prog);
	
#if defined(DEBUG)
	GLint logLength;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(prog, logLength, &logLength, log);
		NSLog(@"Program link log:\n%s", log);
		free(log);
	}
#endif
	
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	return status != 0;
}
		
//////////////////////////////////////////////////////////////////////
		
#define GL(x) { TRACE(">>%s<<\n", #x); x; int n = glGetError(); if(n != 0) { TRACE("GLERROR: %d\n", n); __builtin_trap(); } }

bool SpriteList::SpriteListImpl::ValidateProgram(GLuint prog)
{
	GLint logLength, status;
	
	glValidateProgram(prog);
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 0)
	{
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(prog, logLength, &logLength, log);
		NSLog(@"Program validate log:\n%s", log);
		free(log);
	}
	
	glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);	
	return status != 0;
}

//////////////////////////////////////////////////////////////////////

bool SpriteList::SpriteListImpl::Init(int maxSprites, int maxRuns)
{
	mMaxSprites = maxSprites;
	mMaxRuns = (maxRuns == 0) ? maxSprites : maxRuns;

	mSpriteRuns = new SpriteRun[mMaxRuns];

	for(int i=0; i<kNumVertexBuffers; ++i)
	{
#if defined(USE_VBOS)
		GL(glGenBuffers(1, &mVertexBuffer[i]));
		GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[i]));
		GL(glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteVertex) * maxSprites * 6, NULL, GL_DYNAMIC_DRAW));
#else
		mVertexBuffer[i] = new SpriteVertex[maxSprites * 6];
#endif
	}
	
	mCurrentVertBufferIndex = 0;
	Map();
	return true;
}

//////////////////////////////////////////////////////////////////////

static int frame = 0;

void SpriteList::SpriteListImpl::Map()
{
	mCurrentSpriteRun = null;
	
	mDirtyTexture = false;
	mDirtyTransform = false;
	mDirtyBlendMode = false;
	
	mCurrentScale = Vec2::one;
	mCurrentRotation = 0;
	mCurrentOrigin = Vec2::zero;
	mCurrentIndex = 0;
	mCurrentBlendMode = kModulate;
	
#if defined(USE_VBOS)
	GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[mCurrentVertBufferIndex]));
	GL(mCurrentVertBuffer = (SpriteVertex *)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
#else
	mCurrentVertBuffer = mVertexBuffer[mCurrentVertBufferIndex];
#endif

	mCurrentVert = mCurrentVertBuffer;
	mEndVert = mCurrentVertBuffer + mMaxSprites * 6;
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::ToggleVertBuffer()
{
	if(++mCurrentVertBufferIndex >= kNumVertexBuffers)
	{
		mCurrentVertBufferIndex = 0;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::SetupBlend(SpriteList::eBlendMode mode)
{
	switch(mode)
	{
		case kModulate:
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			break;
			
		case kInterpolate:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
			
		case kAdditive:
			glBlendFunc(GL_ONE, GL_ONE);
			break;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SpriteListImpl::Submit()
{
	Texture::SetSamplerID(mUniformID[UNIFORM_FRAGMENT_SHADER_SAMPLER]);

	GLKMatrix4 m;
	
	// Portrait
	// m = GLKMatrix4MakeOrtho(0, Screen::Width(), Screen::Height(), 0, 0, 1);
	
	// Landscape
	
	// Screen::Height() = 320
	// Screen::Width() = 480
	
	m = GLKMatrix4MakeOrtho(0, Screen::Height(), Screen::Width(), 0, 0, 1);
	Swap(m.m[0], m.m[4]);
	Swap(m.m[1], m.m[5]);
	m.m[0] *= -1;
	m.m[4] *= -1;
	m.m[12] += 2;

	glUniformMatrix4fv(mUniformID[UNIFORM_VERTEX_SHADER_MATRIX], 1, 0, m.m);

	glEnable(GL_LINE_SMOOTH);

	Texture *activeTexture = null;
	SpriteList::eBlendMode activeBlendMode = SpriteList::eBlendMode::kModulate;

	SetupBlend(activeBlendMode);
	glEnable(GL_BLEND);

	glDisable(GL_CULL_FACE);

#if defined(USE_VBOS)
	GL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[mCurrentVertBufferIndex]));
	GL(glUnmapBufferOES(GL_ARRAY_BUFFER));
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_UV);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glVertexAttribPointer(ATTRIB_POSITION,	2, GL_FLOAT,			GL_FALSE,	sizeof(SpriteVertex), OFFSETOF(SpriteVertex, mPos));
	glVertexAttribPointer(ATTRIB_UV,		2, GL_FLOAT,			GL_FALSE,	sizeof(SpriteVertex), OFFSETOF(SpriteVertex, mTexCoord));
	glVertexAttribPointer(ATTRIB_COLOR,		4, GL_UNSIGNED_BYTE,	GL_TRUE,	sizeof(SpriteVertex), OFFSETOF(SpriteVertex, mColor));
#else
	SpriteVertex *v = mVertexBuffer[mCurrentVertBufferIndex];
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_UV);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glVertexAttribPointer(ATTRIB_POSITION,	2, GL_FLOAT,			GL_FALSE,	sizeof(SpriteVertex), &v->mPos);
	glVertexAttribPointer(ATTRIB_UV,		2, GL_FLOAT,			GL_FALSE,	sizeof(SpriteVertex), &v->mTexCoord);
	glVertexAttribPointer(ATTRIB_COLOR,		4, GL_UNSIGNED_BYTE,	GL_TRUE,	sizeof(SpriteVertex), &v->mColor);
#endif
	
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
			
			if(run.mBlendMode != activeBlendMode)
			{
				SetupBlend(run.mBlendMode);
				activeBlendMode = run.mBlendMode;
			}
			
			if(run.mTransform)
			{
				GLKMatrix4 t;
				t = GLKMatrix4Multiply(m, GLKMatrix4MakeTranslation(run.mOrigin.x, run.mOrigin.y, 0));
				t = GLKMatrix4Multiply(t, GLKMatrix4MakeRotation(run.mRotation, 0, 0, 1));
				t = GLKMatrix4Multiply(t, GLKMatrix4MakeScale(run.mScale.x, run.mScale.y, 0));
				t = GLKMatrix4Multiply(t, GLKMatrix4MakeTranslation(-run.mOrigin.x, -run.mOrigin.y, 0));
				glUniformMatrix4fv(mUniformID[UNIFORM_VERTEX_SHADER_MATRIX], 1, 0, t.m);
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
					
				case SpriteRun::kInvalid:
					break;
			}
		}
	}
	
#if defined(USE_VBOS)
	GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
#endif
	
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


int SpriteList::RunsSubmitted() const
{
	return impl->mCurrentIndex;
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

void SpriteList::SetTexture(Texture *texture)
{
	if(texture != impl->mCurrentTexture)
	{
		impl->mDirtyTexture = true;
		impl->mCurrentTexture = texture;
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::SetBlendingMode(SpriteList::eBlendMode mode)
{
	if(mode != impl->mCurrentBlendMode)
	{
		impl->mDirtyBlendMode = true;
		impl->mCurrentBlendMode = mode;
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
	++frame;

	SpriteListImpl::sAllSpriteLists.ForEach([] (SpriteList *s)
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
