//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

template<typename T> struct ScopedObject
{
	explicit ScopedObject(T *p = 0)
		: mPointer(p)
	{
	}

	~ScopedObject()
	{
		SafeRelease(mPointer);
	}

	bool IsNull() const
	{
		return mPointer == null;
	}

	T &operator *()
	{
		return *mPointer;
	}

	T *operator->()
	{
		return mPointer;
	}

	T **operator&()
	{
		return &mPointer;
	}

	void Reset(T *p = 0)
	{
		SafeRelease(mPointer);
		mPointer = p;
	}

	T* Get() const
	{
		return mPointer;
	}

private:
	ScopedObject(const ScopedObject&);
	ScopedObject& operator=(const ScopedObject&);

	T *mPointer;
};

