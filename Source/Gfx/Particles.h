//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Particle : list_node<Particle>, Pooled<Particle, 1024>
{
	static pool_t			pool;

	Vec2					mOrigin;
	Vec2					mVelocity;
	Color					mLaunchColor;
	Color					mDecayColor;
	float					mLifeTime;

	Particle(linked_list<Particle> &list, Vec2 const &origin, Vec2 const &velocity, float lifeTime, Color launchColor, Color decayColor);
	bool Calc(float time, Vec2 &pos, Color &color);
};

//////////////////////////////////////////////////////////////////////

struct ParticleList : list_node<ParticleList>, Pooled<ParticleList, 64>
{
	static pool_t			pool;

	Texture *				mTexture;
	linked_list<Particle>	mParticles;
	double					mLaunchTime;
	double					mDeathTime;

	ParticleList(linked_list<ParticleList> &list, Vec2 const &origin, Texture *texture, int numParticles);
	ParticleList(linked_list<ParticleList> &list, Vec2 const &start, Vec2 const &end, float arcBegin, float arcEnd, Texture *texture, int numParticles);
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

