//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct GridCell;

struct Particle : public Position
{
	float mDensity;
	float mNearDensity;
	float mViscositySigma;
	float mViscosityBeta;
	float mMass;
	int mIndex;
	int mNeighbourListIndex;
	int mNeighbourCount;
	GridCell *mCell;	// which cell am I in?

	Particle() : Position()
	{
	}

	void UpdateVelocity(float deltaTime)
	{
		Vec2 newPos = mPosition + mForce * deltaTime;
		mVelocity = (newPos - mOldPosition) / deltaTime;
	}

	void UpdatePosition(float deltaTime)
	{
		mPosition += mForce * deltaTime;
		mOldPosition = mPosition;
		mPosition += mVelocity * deltaTime;
	}
};