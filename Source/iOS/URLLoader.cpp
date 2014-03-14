//////////////////////////////////////////////////////////////////////

#include "pch.h"
#import "URLLoaderDelegate.h"

//////////////////////////////////////////////////////////////////////

struct URLLoader::Impl
{
	URLLoaderDelegate *					mLoaderDelegate;
	std::function<void(bool, ValueMap)>	mCallback;
	eState								mState;

	//////////////////////////////////////////////////////////////////////
	
	Impl()
		: mLoaderDelegate(null)
		, mState(kNew)
	{
	}
	
	//////////////////////////////////////////////////////////////////////
	
	~Impl()
	{
		if(mLoaderDelegate)
		{
			[mLoaderDelegate dealloc];
			mLoaderDelegate = null;
		}
	}
	
	//////////////////////////////////////////////////////////////////////
	
	void Load(char const *url, string const &postData, std::function<void(bool, ValueMap)> callback)
	{
		mState = kBusy;
		mCallback = callback;
		mLoaderDelegate = [[URLLoaderDelegate alloc] init];
		[mLoaderDelegate loadURL:url withPostData:postData andCallback:[this] (bool success, ValueMap values)
		{
			mState = success ? kIdle : kError;
			mCallback(success, values);
		}];
	}
};

//////////////////////////////////////////////////////////////////////

URLLoader::URLLoader()
{
	impl = new Impl();
}

//////////////////////////////////////////////////////////////////////

URLLoader::~URLLoader()
{
	SafeDelete(impl);
}

//////////////////////////////////////////////////////////////////////

void URLLoader::Load(char const *url, string const &postData, std::function<void(bool, ValueMap)> callback)
{
	impl->Load(url, postData, callback);
}

//////////////////////////////////////////////////////////////////////

URLLoader::eState URLLoader::GetState() const
{
	return impl->mState;
}

