//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

Grid::Grid() : mCells(null), mIndices(null)
{
}

//////////////////////////////////////////////////////////////////////

void Grid::Init(float w, float h, float particleRadius)
{
	Release();

	srand(timeGetTime());

	mWidth = (int)ceilf(w / particleRadius);
	mHeight = (int)ceilf(h / particleRadius);

	mScale.x = mWidth / w;
	mScale.y = mHeight / h;

	mNumCells = mWidth * mHeight;

	mIndices = new int[NumParticles];
	mCells = new GridCell[mNumCells];

	for(int i=0; i<mNumCells; ++i)
	{
		int r = (rand() & 0xff) << 16;
		int g = (rand() & 0xff) << 8;
		int b = (rand() & 0xff);
		mCells[i].mColor = 0xff000000 | r | g | b;
	}
}

//////////////////////////////////////////////////////////////////////

void Grid::CalcOffsets()
{
	int offset = 0;
	for(int i=0; i<mNumCells; ++i)
	{
		mCells[i].mStart = offset;
		mCells[i].mOffset = offset;
		offset += mCells[i].mLength;
	}
	assert(offset == mTotalPoints);
}

//////////////////////////////////////////////////////////////////////

void Grid::Release()
{
	SafeDeleteArray(mCells);
	SafeDeleteArray(mIndices);
}

//////////////////////////////////////////////////////////////////////

Grid::~Grid()
{
	Release();
}

//////////////////////////////////////////////////////////////////////

void Grid::Reset()
{
	mTotalPoints = 0;
	for(int i=0; i<mNumCells; ++i)
	{
		mCells[i].mLength = 0;
		mCells[i].mOffset = 0;
	}
}

