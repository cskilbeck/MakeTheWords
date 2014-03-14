//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

extern string gUserName;
extern string gUserID;

struct MainUI : UI::Screen
{
	MainUI(SpriteList *spriteList, Dictionary *dictionary);
	~MainUI();

	void Draw() override;
	eComponentReturnCode Update() override;

	Texture *				mPlayButtonTexture;
	Texture *				mLoginButtonTexture;
	Texture *				mPostButtonTexture;
	Texture *				mFBProfileTexture;

	UI::TextButton *		mTextButton;
	UI::ImageButton *		mPracticeButton;
	UI::ImageButton *		mPlayButton;
	UI::ImageButton *		mLoginButton;
	UI::ImageButton *		mPostButton;
	UI::ImageButton *		mFBProfileButton;

	UI::TextLabel *			mPlayLabel;
	UI::TextLabel *			mPracticeLabel;

	Font *					mLabelFont;

	Dictionary *			mDictionary;
	Game::Game *			mGame;
	ResultsUI *				mResultsUI;

	URLLoader				mURLLoader;
	
	bool					mRegistered;
	int64					mBoardID;
	uint64					mGameID;
	uint32					mRandomSeed;
	time_t					mStartTime;
	time_t					mEndTime;
	time_t					mNetworkTime;
	int						mGameTimeRemaining;
	int						mGameDuration;

	int						mNetworkLocalTimeDelta;
	
	double					mLastNetworkCheck;

	void PostStory();
};