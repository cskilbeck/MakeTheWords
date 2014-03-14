//////////////////////////////////////////////////////////////////////

#pragma once

#if defined(IOS)
#import <FacebookSDK/FacebookSDK.h>
#endif

//////////////////////////////////////////////////////////////////////

namespace Facebook
{
	void Login(bool allowUI);
	void Logout();
	bool IsLoggedIn();
#if defined(IOS)
	bool HandleOpenURL(NSURL *url);
#endif
	void OnAppActivate();
	void RequestPostPermissions(function<void()> callback);
	void PostStory();
	string const &Username();
	uint64 UserID();
}

//////////////////////////////////////////////////////////////////////
