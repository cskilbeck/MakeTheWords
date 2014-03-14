//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	URLLoader sURLLoader;
	uint64 sUserID = 0;
	string sUserName;
}

//////////////////////////////////////////////////////////////////////

namespace Facebook
{
	void Login(bool allowUI)
	{
		if(sURLLoader.GetState() != URLLoader::Busy)
		{
			sURLLoader.Load(WebServiceURL("friend").c_str(), string(), [&] (bool success, ValueMap values)
			{
				DumpValueMap(values);
				if(success)
				{
					if(values.find("facebook_id") != values.end())
					{
						sUserID = (uint64)values["facebook_id"];
						sURLLoader.Load(WebServiceURL("register").c_str(), Format("facebook_id=%lld", (int64)values["facebook_id"]).c_str(), [&] (bool success, ValueMap values)
						{
							sUserName = (string)values["name"];
						});
					}
				}
			});
		}
	}

	void Logout()
	{
		sURLLoader.Close();
		sUserName.clear();
		sUserID = 0;
	}

	bool IsLoggedIn()
	{
		return sUserID != 0;
	}

#if defined(IOS)
	bool HandleOpenURL(NSURL *url);
#endif

	void OnAppActivate()
	{
	}

	void RequestPostPermissions(function<void()> callback)
	{
	}

	void PostStory()
	{
	}

	string const &Username()
	{
		return sUserName;
	}

	uint64 UserID()
	{
		return sUserID;
	}
}