//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct FlagMap
{
	int width;
	int height;
	int rowWidth;
	int size;
	u32 *flags;

	FlagMap() : flags(null)
	{
	}

	void Init(int w, int h)
	{
		Release();

		width = w;
		height = h;
		assert(width > 0);
		assert(height > 0);
		rowWidth = (width + 31) >> 5;
		size = rowWidth * height;
		flags = new u32[size];
		Reset();
	}

	void Release()
	{
		SafeDeleteArray(flags);
	}

	~FlagMap()
	{
		Release();
	}

	void Reset()
	{
		assert(flags != null);
		assert(size != 0);
		memset(flags, 0, size * sizeof(u32));
	}

	void Set(int x, int y)
	{
		assert(x < width);
		assert(x >= 0);
		assert(y < height);
		assert(y >= 0);
		int bit = x & 31;
		int word = x >> 5;
		flags[word + y * rowWidth] |= 1 << bit;
	}
	
	void Clear(int x, int y)
	{
		assert(x < width);
		assert(x >= 0);
		assert(y < height);
		assert(y >= 0);
		int bit = x & 31;
		int word = x >> 5;
		flags[word + y * rowWidth] &= ~(1 << bit);
	}

	bool IsSet(int x, int y)
	{
		assert(x < width);
		assert(x >= 0);
		assert(y < height);
		assert(y >= 0);
		int bit = x & 31;
		int word = x >> 5;
		return (flags[word + y * rowWidth] & (1 << bit)) != 0;
	}
};
