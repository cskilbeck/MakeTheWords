//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct MakeTheWords : App
{
	MakeTheWords();

	void Init();
	void Draw();
	void Release();

	SpriteList *				mSpriteList;
	Dictionary *				mDictionary;
	MainUI *					mMainUI;
};

string WebServiceURL(char const *action);
extern char const *g_WebService;