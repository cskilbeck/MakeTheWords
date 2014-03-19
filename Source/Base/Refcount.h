//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct RefCount
{
	RefCount() : mRefCount(1)
	{
		allRefCountedObjects.push_back(this);
		me = allRefCountedObjects.end();
		--me;
	}

	virtual ~RefCount()
	{
		allRefCountedObjects.remove(*me);
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

	static void ShowLeaks()
	{
		for(auto p: allRefCountedObjects)
		{
			TRACE("Leaked Object at %p\n", p);
		}
	}

	int mRefCount;
	list<RefCount *>::iterator me;
	static list<RefCount *>allRefCountedObjects;
};
