//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Random
{
	uint32 mW;
	uint32 mZ;

	static Timer sTimer;

	Random()
	{
		uint64 t = sTimer.GetCounter();
		mZ = (uint32)t;
		mW = (uint32)(t >> 32);
	}

	void Seed(uint32 seed)
	{
		mZ = seed;
		mW = ~seed;
	}

	Random(uint32 seed)
	{
		Seed(seed);
	}

	uint32 Next()
	{
		mZ = 36969 * (mZ & 65535) + (mZ >> 16);
		mW = 18000 * (mW & 65535) + (mW >> 16);
		return (mZ << 16) + mW;
	}

	float NextFloat()
	{
		return Next() / (float)0xffffffff;
	}
};
