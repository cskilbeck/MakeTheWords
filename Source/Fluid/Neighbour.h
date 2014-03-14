//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Neighbour
{
	float mSmoothingKernel;
	float mSmoothingKernel2;
	float mDistance;
	Particle *mParticle;
};

//////////////////////////////////////////////////////////////////////

struct Neighbourhood
{
	static const int kMaxNeighboursPerParticle = 256;

	int mNeighbourLimit;
	Neighbour *mNeighbours;
	int mCurrentNeighbour;

	Neighbourhood()
	{
	}

	void Init(int NumParticles)
	{
		mNeighbourLimit = NumParticles * kMaxNeighboursPerParticle;
		mNeighbours = new Neighbour[mNeighbourLimit];
		Reset();
	}

	~Neighbourhood()
	{
		Release();
	}

	void Reset()
	{
		mCurrentNeighbour = 0;
	}

	void Release()
	{
		SafeDeleteArray(mNeighbours);
	}

	bool AddNeighbour(float sk, float sk2, float dist, Particle *particle)
	{
		if(mCurrentNeighbour < mNeighbourLimit)
		{
			Neighbour &n = mNeighbours[mCurrentNeighbour++];

			n.mSmoothingKernel = sk;
			n.mSmoothingKernel2 = sk2;
			n.mDistance = dist;
			n.mParticle = particle;
			return true;
		}
		else
		{
			return false;
		}
	}

	Neighbour &GetNeighbour(int index)
	{
		return mNeighbours[index];
	}
};
