//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Particle : list_node<Particle>
{
	Vec2			mOrigin;
	Vec2			mVelocity;
	Color			mLaunchColor;
	Color			mDecayColor;
	float			mLifeTime;

	void *operator new(size_t s);
	void operator delete(void *p);

	Particle(Vec2 const &origin, Vec2 const &velocity, float lifeTime, Color launchColor, Color decayColor);
	bool Calc(float time, Vec2 &pos, Color &color);
};

//////////////////////////////////////////////////////////////////////

struct ParticleList : list_node<ParticleList>
{
	Texture *				mTexture;
	linked_list<Particle>	mParticles;
	double					mLaunchTime;
	double					mDeathTime;

	void *operator new(size_t s);
	void operator delete(void *p);

	ParticleList(Vec2 const &origin, Texture *texture, int numParticles);

	ParticleList(Vec2 const &start, Vec2 const &end, float arcBegin, float arcEnd, Texture *texture, int numParticles);

	~ParticleList();

	void Draw(SpriteList *spriteList);
};

//////////////////////////////////////////////////////////////////////

struct Particles : Component
{
	Texture *					mBlingTexture;
	linked_list<ParticleList>	mParticleLists;

	Particles(SpriteList *spriteList);
	~Particles();

	eComponentReturnCode Update();
	void Draw();

	void Launch(Vec2 const &origin, int numParticles);
	void Launch(Vec2 const &start, Vec2 const &end, float arcBegin, float arcEnd, int numParticles);
	void LaunchHorizontal(Vec2 const &start, float len);
	void LaunchVertical(Vec2 const &start, float len);
};

//////////////////////////////////////////////////////////////////////

