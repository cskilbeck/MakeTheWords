//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

extern string gUserName;
extern string gUserID;

struct Leaderboard : UI::Screen
{
	Leaderboard(SpriteList *spriteList);
	~Leaderboard();

	void Draw() override;
	eComponentReturnCode Update() override;

	struct LeaderboardRow
	{
		uint64			mFacebookID;		// if this is 0, it's not a valid row
		uint32			mScore;
		uint32			mRanking;
		string			mName;
	};

	int64				mGameID;		// game they were playing
	int64				mBoardID;		// their attempt

	URLLoader			mURLLoader;
	bool				mResultsAreIn;
	bool				mGotResults;
	double				mLastCheckTime;
	int64				mSecondsRemaining;
	bool				mGameStateKnown;

	LeaderboardRow		mLeaderboardScreen[9];	// one screen of results
	uint32				mLeaderboardRows;		// how many on this screen
	uint32				mLeaderboardOffset;		// offset for queries
	uint32				mLeaderboardSize;		// total rows from the game

	Font *				mFont;

	Texture *			mTopTexture;
	Texture *			mUpTexture;
	Texture *			mMiddleTexture;
	Texture *			mDownTexture;
	Texture *			mBottomTexture;

	UI::ImageButton *	mTopButton;
	UI::ImageButton *	mUpButton;
	UI::ImageButton *	mMiddleButton;
	UI::ImageButton *	mDownButton;
	UI::ImageButton *	mBottomButton;
};

//////////////////////////////////////////////////////////////////////

