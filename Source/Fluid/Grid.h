//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct GridCell
{
	int mStart;	// where in the big list of indices does this gridcell's entries begin
	int mLength;	// how many particles in this cell
	int mOffset;	// where it got to
	u32 mColor;	// for debugging
};

//////////////////////////////////////////////////////////////////////

struct Grid
{
	int mWidth;
	int mHeight;
	Vec2 mScale;
	int mNumCells;
	int mTotalPoints;
	int *mIndices;
	GridCell *mCells;

	Grid();
	virtual ~Grid();
	virtual void Release();
	void Reset();
	void Init(float w, float h, float particleRadius);
	void CalcOffsets();

	GridCell &GetCell(int x, int y)
	{
		return mCells[x + y * mWidth];
	}

	GridCell &GetCell(Vec2 const &pos)
	{
		int x = (int)(pos.x	* mScale.x);
		int y = (int)(pos.y	* mScale.y);
		if(x < 0)
		{
			x = 0;
		}
		if(x >= mWidth)
		{
			x = mWidth - 1;
		}
		if(y < 0)
		{
			y = 0;
		}
		if(y >= mHeight)
		{
			y = mHeight - 1;
		}
		return GetCell(x, y);
	}

	void CountPoint(Vec2 const &pos)
	{
		++mTotalPoints;
		GridCell &cell = GetCell(pos);
		cell.mLength++;
	}

	void AddPoint(Particle &p, int index)
	{
		GridCell &cell = GetCell(p.mPosition);
		p.mCell = &cell;
		int offset = cell.mOffset++;
		mIndices[offset] = index;
		assert(offset <= cell.mStart + cell.mLength);
	}
};