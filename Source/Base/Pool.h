//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct PoolFake : list_node<PoolFake>
{
};

//////////////////////////////////////////////////////////////////////

template <typename T, size_t numItems = 0> struct Pool
{
	byte *mPool;
	linked_list<PoolFake> mFreeList;

	//////////////////////////////////////////////////////////////////////

	Pool() : mPool(null)
	{
		Resize(numItems);
	}

	//////////////////////////////////////////////////////////////////////

	Pool(int n) : mPool(null)
	{
		Resize(n);
	}

	//////////////////////////////////////////////////////////////////////

	~Pool()
	{
		Clear();
	}

	//////////////////////////////////////////////////////////////////////

	void Clear()
	{
		SafeDeleteArray(mPool);
		mFreeList.clear();
	}

	//////////////////////////////////////////////////////////////////////

	void Resize(int n)
	{
		Clear();
		if(n > 0)
		{
			mPool = new byte[n * sizeof(T)];
			for(int i=0; i<n; ++i)
			{
				mFreeList.push_back(reinterpret_cast<PoolFake *>(mPool + i * sizeof(T)));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	T *Alloc()
	{
		if(mFreeList.empty())
		{
			return null;
		}
		return reinterpret_cast<T *>(mFreeList.pop_back());
	}

	//////////////////////////////////////////////////////////////////////

	void Free(T *o)
	{
		mFreeList.push_back(reinterpret_cast<PoolFake *>(o));
	}
};

//////////////////////////////////////////////////////////////////////

template <typename T, size_t N> struct Pooled
{
	typedef Pool<T, N> pool_t;

	static pool_t &pool()
	{
		static pool_t sPool;
		return sPool;
	}

	void *operator new(size_t s)
	{
		return (void *)pool().Alloc();
	}

	void operator delete(void *p, size_t s)
	{
		pool().Free((T *)p);
	}

private:

	void operator delete[](void *p, size_t s)
	{
	}
};

//////////////////////////////////////////////////////////////////////
