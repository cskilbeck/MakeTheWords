//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct World
{
	static const Vec2 kGravity;

	float mWidth;
	float mHeight;
	Particle *mParticles;
	Neighbourhood mNeighbourhood;
	Neighbourhood mNeighbourhoodOther;

	World();

	void Init(float w, float h);
	void Release();

	void Step();

private:

	FlagMap mSpringFlags;
	Timer mTimer;
	Grid mGrid;
	Springs mSprings;

	float timeBank;
	float deltaTime;
	float deltaTimeScaled;
	float deltaTimeScaledSquared;

	void UpdateGrid();
	void CalculateViscoElasticity();
	void CalculateDensity();
	void CalculatePressure();
	void UpdateParticles();
	void DoCollision();
	void CalculateViscosity();

	void CreateNeighbours(Neighbourhood &n, Particle &p, Particle &q, float &density, float &nearDensity);
};