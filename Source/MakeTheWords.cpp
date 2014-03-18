//////////////////////////////////////////////////////////////////////
// * Register in people table at boot
// * Post final board when game has ended (and make server accept it)
// Stop DefinitionUI blocking update
// Show status of current game on results screen [GameManager]
// View other peoples boards
// Leaderboard paging
// Suggest missing word

#include "pch.h"

//////////////////////////////////////////////////////////////////////

MakeTheWords g_MakeTheWords;
char const *g_WebService="http://107.21.245.151/mtw";

string WebServiceURL(char const *action)
{
	return Format("%s?action=%s", g_WebService, action);
}

//////////////////////////////////////////////////////////////////////

MakeTheWords::MakeTheWords()
	: App()
	, mSpriteList(null)
	, mMainUI(null)
	, mDictionary(null)
{
}

//////////////////////////////////////////////////////////////////////

void MakeTheWords::Draw()
{
	Screen::Clear(0xff304050);
	App::Draw();
}

//////////////////////////////////////////////////////////////////////

void MakeTheWords::Init()
{
	Reference::Letter::Open();
	DefinitionScreen::Open();

	mDictionary = Dictionary::Load("english.dictionary");
	mSpriteList = SpriteList::Create(8000, 800);
	mMainUI = new MainUI(mSpriteList, mDictionary);
}

//////////////////////////////////////////////////////////////////////

void MakeTheWords::Release()
{
	::Release(mDictionary);
	::Release(mSpriteList);
	Delete(mMainUI);
	Reference::Letter::Close();
	DefinitionScreen::Close();
}
