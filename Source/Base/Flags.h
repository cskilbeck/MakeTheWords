//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Flags
{
	uint32 mFlags;

	Flags(uint32 f = 0) : mFlags(f)
	{
	}

	void set(uint32 mask)
	{
		mFlags |= mask;
	}

	void clear(uint32 mask)
	{
		mFlags &= ~mask;
	}

	bool operator()(uint32 mask)
	{
		return (mFlags & mask) != 0;
	}
};