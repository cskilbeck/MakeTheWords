//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Facebook.h"

//////////////////////////////////////////////////////////////////////

Leaderboard::Leaderboard(SpriteList *spriteList)
	: UI::Screen(spriteList)
	, mResultsAreIn(false)
	, mGotResults(false)
	, mSecondsRemaining(-1)
	, mGameID(0)
{
	mFont = FontManager::Load("definitionBody");
	mTopTexture = Texture::Load("top.png");
	mUpTexture = Texture::Load("up.png");
	mMiddleTexture = Texture::Load("middle.png");
	mDownTexture = Texture::Load("down.png");
	mBottomTexture = Texture::Load("bottom.png");

	float left = ::Screen::Width() - 48.0f;
	Vec2 size = Vec2(32, 32);

	mTopButton		= new UI::ImageButton(mTopTexture		, Vec2(left,  64), size);
	mUpButton		= new UI::ImageButton(mUpTexture		, Vec2(left, 112), size);
	mMiddleButton	= new UI::ImageButton(mMiddleTexture	, Vec2(left, 160), size);
	mDownButton		= new UI::ImageButton(mDownTexture		, Vec2(left, 208), size);
	mBottomButton	= new UI::ImageButton(mBottomTexture	, Vec2(left, 256), size);

	AddUIItem(mTopButton);
	AddUIItem(mUpButton);
	AddUIItem(mMiddleButton);
	AddUIItem(mDownButton);
	AddUIItem(mBottomButton);

}

//////////////////////////////////////////////////////////////////////

Leaderboard::~Leaderboard()
{
	RemoveUIItem(mTopButton);
	RemoveUIItem(mUpButton);
	RemoveUIItem(mMiddleButton);
	RemoveUIItem(mDownButton);
	RemoveUIItem(mBottomButton);

	SafeDelete(mTopButton);
	SafeDelete(mUpButton);
	SafeDelete(mMiddleButton);
	SafeDelete(mDownButton);
	SafeDelete(mBottomButton);

	SafeRelease(mTopTexture);
	SafeRelease(mUpTexture);
	SafeRelease(mMiddleTexture);
	SafeRelease(mDownTexture);
	SafeRelease(mBottomTexture);

	SafeRelease(mFont);
}

//////////////////////////////////////////////////////////////////////

void Leaderboard::Draw()
{
	UI::Screen::Draw();
	mSpriteList->ResetTransform();
	mSpriteList->SetBlendingMode(SpriteList::eBlendMode::kModulate);

	if(mGameStateKnown && !mGotResults)
	{
		char const *message;
		int64 sr;
		if(mSecondsRemaining < 0)
		{
			message = "Game ended %lld seconds ago";
			sr = -mSecondsRemaining;
		}
		else
		{
			message = "Game ends in %lld seconds";
			sr = mSecondsRemaining;
		}
		mFont->DrawString(mSpriteList, Format(message, sr).c_str(), Vec2(50, 120));
	}

	if(mGotResults)
	{
		char const *white = "#FFFFFFFF#";
		char const *yellow = "#FFFFFF00#";
		static int lineSpace = 4;
		int lineHeight = mFont->GetHeight() + lineSpace;
		static int lineTop = 64;
		for(uint i=0; i<mLeaderboardRows; ++i)
		{
			float y = (float)(i * lineHeight + lineTop);
			char const *color = white;
			if(mLeaderboardScreen[i].mFacebookID == Facebook::UserID())
			{
				color = yellow;
			}
			mFont->DrawString(mSpriteList, Format("%d", mLeaderboardScreen[i].mRanking).c_str(), Vec2(10, y));
			mFont->DrawString(mSpriteList, Format("%d", mLeaderboardScreen[i].mScore).c_str(), Vec2(55, y));
			mFont->DrawString(mSpriteList, Format("%s%s", color, mLeaderboardScreen[i].mName.c_str()), Vec2(120, y));
		}
	}
	else
	{
		mFont->DrawString(mSpriteList, Format("Waiting for results%.*s", (int)(g_Time * 4) % 4 + 1, "...").c_str(), Vec2(100, 80), Font::HLeft, Font::VTop);
	}
}

//////////////////////////////////////////////////////////////////////

Component::eComponentReturnCode Leaderboard::Update()
{
	if(!mResultsAreIn && mURLLoader.GetState() != URLLoader::Busy && (g_Time - mLastCheckTime) > 1)
	{
		mLastCheckTime = g_Time;
		mURLLoader.Load(WebServiceURL("game").c_str(), Format("game_id=%lld", mGameID).c_str(), [this] (bool success, ValueMap values)
		{
			if(success)
			{
				// check game end time here
				// also count of players
				//DumpValueMap(values);

				mGameStateKnown = true;
				mSecondsRemaining = (int64)values["end_time"] - (int64)values["current_time"];
				mResultsAreIn = (int)values["rank_calculated"] != 0;
			}
		});
	}

	if(mResultsAreIn && !mGotResults && mURLLoader.GetState() != URLLoader::Busy && (g_Time - mLastCheckTime) > 1)
	{
		mLastCheckTime = g_Time;
		mURLLoader.Load(WebServiceURL("results").c_str(), Format("game_id=%lld&board_id=%lld", mGameID, mBoardID).c_str(), [this] (bool success, ValueMap values)
		{
			if(success)
			{
				//DumpValueMap(values);
				mLeaderboardRows = (uint32)values["rows"];
				mLeaderboardOffset = (uint32)values["offset0"];
				mLeaderboardSize = (uint32)values["total_rows"];

				for(uint i=0; i<mLeaderboardRows; ++i)
				{
					// mLeaderboardScreen[i].mBoard = (uint32)values[Format("board%d", i).c_str()];
					mLeaderboardScreen[i].mFacebookID = (uint64)values[Format("facebook_id%d", i).c_str()];
					mLeaderboardScreen[i].mRanking = (uint32)values[Format("ranking%d", i).c_str()];
					mLeaderboardScreen[i].mScore = (uint32)values[Format("score%d", i).c_str()];
					mLeaderboardScreen[i].mName = (string)values[Format("name%d", i).c_str()];
				}
				mGotResults = true;
			}
		});
	}
	return UI::Screen::Update();
}

//////////////////////////////////////////////////////////////////////

