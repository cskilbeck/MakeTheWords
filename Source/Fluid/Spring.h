//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Spring : Node<Spring>
{
	Particle *mP0;
	Particle *mP1;
	float mRestLength;
	float mCoefficient;

	Vec2 CalculateForce()
	{
		Vec2 diff = mP0->mPosition - mP1->mPosition;
		float length = diff.length();
		return diff * (mRestLength - length) * mCoefficient;
	}
};

//////////////////////////////////////////////////////////////////////

struct Springs
{
	int mMaxSprings;
	Spring *mSprings;
	List<Spring> mFreeList;
	List<Spring> mBusyList;

	Springs()
	{
	}

	void Init(int numSprings)
	{
		mMaxSprings = numSprings;

		mSprings = new Spring[mMaxSprings];

		for(int i=0; i<mMaxSprings; ++i)
		{
			mFreeList.AddTail(mSprings + i);
		}
	}

	void Release()
	{
		SafeDeleteArray(mSprings);
		mFreeList.Clear();
		mBusyList.Clear();
	}

	Spring *Alloc()
	{
		Spring *s = mFreeList.PopTail();
		if(s)
		{
			mBusyList.AddTail(s);
		}
		return s;
	}

	void Free(Spring *s)
	{
		mBusyList.Remove(s);
		mFreeList.AddTail(s);
	}
};