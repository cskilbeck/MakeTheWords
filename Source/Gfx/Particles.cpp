//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	Pool<Particle> sParticlePool(1024);
	Pool<ParticleList> sParticleListPool(64);
	Random sRandom;
}

//////////////////////////////////////////////////////////////////////

void *Particle::operator new(size_t s)
{
	return sParticlePool.Alloc();
}

//////////////////////////////////////////////////////////////////////

void Particle::operator delete(void *p)
{
	sParticlePool.Free((Particle *)p);
}

//////////////////////////////////////////////////////////////////////

Particle::Particle(Vec2 const &origin, Vec2 const &velocity, float lifeTime, Color launchColor, Color decayColor)
	: mOrigin(origin)
	, mVelocity(velocity)
	, mLifeTime(lifeTime)
	, mLaunchColor(launchColor)
	, mDecayColor(decayColor)
{
}

//////////////////////////////////////////////////////////////////////

// change

bool Particle::Calc(float time, Vec2 &pos, Color &color)
{
	float timeScalar = time / mLifeTime;
	if(timeScalar <= 1.0f)
	{
		pos = mOrigin + mVelocity * timeScalar;
		color = mDecayColor.Lerp(mLaunchColor, (int)(timeScalar * 256));
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////

void *ParticleList::operator new(size_t s)
{
	return sParticleListPool.Alloc();
}

//////////////////////////////////////////////////////////////////////

void ParticleList::operator delete(void *p)
{
	sParticleListPool.Free((ParticleList *)p);
}

//////////////////////////////////////////////////////////////////////

ParticleList::ParticleList(Vec2 const &origin, Texture *texture, int numParticles /* bunch of ranges */)
	: mTexture(texture)
	, mLaunchTime(g_Time)
{
	float maxAge = 0;
	for(int i=0; i<numParticles; ++i)
	{
		float a = sRandom.NextFloat() * PI * 2.0f;
		float v = sRandom.NextFloat() * 20.0f + 100.0f;
		float l = sRandom.NextFloat() * 0.2f + 0.2f;
		float xv = sinf(a) * v;
		float yv = cosf(a) * v;
		maxAge = Max(l, maxAge);
		Particle *p = new Particle(origin, Vec2(xv, yv), l, Color::White, 0x0000ffff);
		if(p != null)
		{
			mParticles.push_back(p);
		}
	}
	mDeathTime = mLaunchTime + maxAge;
}

//////////////////////////////////////////////////////////////////////

ParticleList::ParticleList(Vec2 const &start, Vec2 const &end, float arcBegin, float arcEnd, Texture *texture, int numParticles)
	: mTexture(texture)
	, mLaunchTime(g_Time)
{
	float maxAge = 0;
	Vec2 d(end-start);
	float arc = arcEnd - arcBegin;
	for(int i=0; i<numParticles; ++i)
	{
		float f = sRandom.NextFloat();
		float a = sRandom.NextFloat() * arc + arcBegin;
		float v = sRandom.NextFloat() * 20.0f + 100.0f;
		float l = sRandom.NextFloat() * 0.2f + 0.2f;
		maxAge = Max(l, maxAge);
		Particle *p = new Particle(d * f + start, Vec2(sinf(a) * v, cosf(a) * v), l, Color::White, 0x00ffffff);
		if(p != null)
		{
			mParticles.push_back(p);
		}
	}
	mDeathTime = mLaunchTime + maxAge;
}

//////////////////////////////////////////////////////////////////////

ParticleList::~ParticleList()
{
	chs::remove_and_delete_all(mParticles);
}

//////////////////////////////////////////////////////////////////////

void ParticleList::Draw(SpriteList *spriteList)
{
	float timeDiff = (float)(g_Time - mLaunchTime);
	for(auto &p : mParticles)
	{
		Vec2 pos;
		Color color;
		if(p.Calc(timeDiff, pos, color))
		{
			spriteList->AddSprite(pos, Vec2(16, 16), color);
		}
	}
}

//////////////////////////////////////////////////////////////////////

Component::eComponentReturnCode Particles::Update()
{
	chs::delete_if(mParticleLists, [] (ParticleList &p)
	{
		return g_Time > p.mDeathTime;
	});
	return kAllow;
}

//////////////////////////////////////////////////////////////////////

void Particles::Draw()
{
	mSpriteList->SetTexture(mBlingTexture);
	for(auto &l : mParticleLists)
	{
		l.Draw(mSpriteList);
	}
}

//////////////////////////////////////////////////////////////////////

Particles::Particles(SpriteList *spriteList)
	: Component(spriteList)
{
	mBlingTexture = Texture::Load("Blob.png");
}

//////////////////////////////////////////////////////////////////////

Particles::~Particles()
{
	SafeRelease(mBlingTexture);
}

//////////////////////////////////////////////////////////////////////

void Particles::Launch(Vec2 const &start, Vec2 const &end, float arcBegin, float arcEnd, int numParticles)
{
	ParticleList *p = new ParticleList(start, end, arcBegin, arcEnd, mBlingTexture, numParticles);
	if(p != null)
	{
		mParticleLists.push_back(p);
	}
}

//////////////////////////////////////////////////////////////////////

void Particles::Launch(Vec2 const &origin, int numParticles)
{
	ParticleList *p = new ParticleList(origin, mBlingTexture, numParticles);
	if(p != null)
	{
		mParticleLists.push_back(p);
	}
}

//////////////////////////////////////////////////////////////////////

void Particles::LaunchVertical(Vec2 const &start, float len)
{
	const float pi2 = PI/2;
	Vec2 s(start);
	Vec2 e = start + Vec2(0, len);
	int n = (int)(len * 20 / 96);
	Launch(s, e, -pi2, -pi2, n);
	Launch(s, e, PI-pi2, PI-pi2, n);
	Launch(s, s, pi2, PI+pi2, 20);
	Launch(e, e, PI+pi2, PI*2+pi2, 20);
}

//////////////////////////////////////////////////////////////////////

void Particles::LaunchHorizontal(Vec2 const &start, float len)
{
	const float pi = 3.14159265f;
	Vec2 s(start);
	Vec2 e = start + Vec2(len, 0);
	int n = (int)(len * 20 / 96);
	Launch(s, e, pi, pi, n);
	Launch(s, e, 0, 0, n);
	Launch(s, s, pi, pi*2, 20);
	Launch(e, e, 0, pi, 20);
}