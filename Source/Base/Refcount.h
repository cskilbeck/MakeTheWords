//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct RefCount
{
	RefCount() : mRefCount(1)
	{
	}

	virtual ~RefCount()
	{
	}

	void AddRef()
	{
		++mRefCount;
	}

	int Release()
	{
		int rc = mRefCount--;
		if(mRefCount == 0)
		{
			delete this;
		}
		return rc;
	}

	int mRefCount;
};