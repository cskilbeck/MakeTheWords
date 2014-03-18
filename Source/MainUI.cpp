/////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Facebook.h"

//////////////////////////////////////////////////////////////////////

void MainUI::PostStory()
{
	Facebook::PostStory();
}

//////////////////////////////////////////////////////////////////////

MainUI::MainUI(SpriteList *spriteList, Dictionary *dictionary)
	: Screen(spriteList)
	, mDictionary(dictionary)
	, mFBProfileTexture(null)
	, mFBProfileButton(null)
	, mStartTime(0)
	, mEndTime(0)
	, mNetworkTime(0)
	, mRandomSeed(0)
	, mLastNetworkCheck(0)
	, mBoardID(-1)
	, mGameID(0)
	, mRegistered(false)
{
	mGame = new Game::Game(mSpriteList, dictionary);
	mPlayButtonTexture = Texture::Load("PlayButton.png");
	mLoginButtonTexture = Texture::Load("redball.png");
	mPostButtonTexture = Texture::Load("post.png");

	mLabelFont = FontManager::Load("definitionBody");

	mTextButton = new UI::TextButton(mLabelFont, "Hello", Vec2(300, 100), Color(96, 96, 96));
	
	mPracticeButton = new UI::ImageButton(mPlayButtonTexture, Vec2(0, 80));
	mPlayButton = new UI::ImageButton(mPlayButtonTexture, Vec2(0, 0));
	mLoginButton = new UI::ImageButton(mLoginButtonTexture, Vec2(200, 160));
	mPostButton = new UI::ImageButton(mPostButtonTexture, Vec2(400, 160));

	mPracticeLabel = new UI::TextLabel(mLabelFont, "Play", Vec2(64, 20));
	mPlayLabel = new UI::TextLabel(mLabelFont, "Practice", Vec2(64, 100));

	mPracticeButton->mEnabled = true;
	mPlayButton->mEnabled = false;
	
	mResultsUI = new ResultsUI(mSpriteList);
	mResultsUI->mVisible = false;
	mResultsUI->mActive = false;

	mResultsUI->OnClose = [this]
	{
		mResultsUI->Activate(false);
		mVisible = true;
		mActive = true;
	};

	AddUIItem(mTextButton);

	AddUIItem(mPracticeLabel);
	AddUIItem(mPlayLabel);
	AddUIItem(mPracticeButton);
	AddUIItem(mPlayButton);
	AddUIItem(mLoginButton);
	AddUIItem(mPostButton);
	
	mLoginButton->OnPress = [this]
	{
		if(Facebook::IsLoggedIn())
		{
			Facebook::Logout();
		}
		else
		{
			Facebook::Login(true);
		}
 	};
	
	mPostButton->OnPress = [this]
	{
		Facebook::RequestPostPermissions([this]()
		{
			 PostStory();
		});
	};

	mPracticeButton->OnPress = [this]
	{
		mResultsUI->mLeaderboard->mGameID = 0;
		mVisible = false;
		mActive = false;
		
		int gameTime = 600;
		int gameTimeRemaining = 600;
		
		if(gDebuggerAttached)
		{
			//gameTimeRemaining = 2;
		}

		Random r;
		mRandomSeed = r.Next();
		
		mGame->Init(mRandomSeed, 0, Facebook::UserID(), 0, gameTimeRemaining, gameTime);
		mGame->Start();
	};
		
	mPlayButton->OnPress = [this]
	{
		// for later...
		mResultsUI->mLeaderboard->mLastCheckTime = g_Time;
		mResultsUI->mLeaderboard->mGotResults = false;
		mResultsUI->mLeaderboard->mResultsAreIn = false;
		mResultsUI->mLeaderboard->mBoardID = mBoardID;
		mResultsUI->mLeaderboard->mGameID = mGameID;
		mVisible = false;
		mActive = false;
		mGame->Init(mRandomSeed, mBoardID, Facebook::UserID(), mGameID, mGameTimeRemaining, mGameDuration);
		mGame->Start();
	};

	mGame->OnQuit = [this]
	{
		mResultsUI->mSummary->SetBoard(&mGame->mBoard);
		mResultsUI->mLeaderboard->mBoardID = mGame->mBoardID;
		mResultsUI->Activate(true);
		mStartTime = 0;
		mBoardID = 0;
	};
}

//////////////////////////////////////////////////////////////////////

Component::eComponentReturnCode MainUI::Update()
{
	// check out the status of the current game
	if(mRegistered && mURLLoader.GetState() != URLLoader::State::Busy)
	{
		if(mURLLoader.GetState() == URLLoader::State::New || g_Time - mLastNetworkCheck >= 1)
		{
			mLastNetworkCheck = g_Time;
			mURLLoader.Load(WebServiceURL("game").c_str(), Format("facebook_id=%llu", Facebook::UserID()), [this] (bool success, ValueMap values)
			{
				if(success)
				{
					mRandomSeed = (uint32)values["random_seed"];
					mStartTime = values["start_time"].mDateTime;
					mEndTime = values["end_time"].mDateTime;
					mGameDuration = (int)values["duration"];
					mNetworkTime = values["current_time"].mDateTime;
					mGameID = values["game_id"];
					mBoardID = values["board_id"];
				}
				else
				{
					TRACE("Network error(%d): %s", (int)values["error_code"], values["error_string"].mString.c_str());
				}
			});
		}
	}
	
	if(Facebook::IsLoggedIn())
	{
		if(!mRegistered)
		{
			if(mURLLoader.GetState() != URLLoader::State::Busy && g_Time - mLastNetworkCheck >= 3)
			{
				mURLLoader.Load(WebServiceURL("register").c_str(), Format("facebook_id=%llu", Facebook::UserID()), [this] (bool success, ValueMap values)
				{
					if(success)
					{
						TRACE("Registered, name is %s\n", values["name"].mString.c_str());
						mRegistered = true;
					}
					else
					{
						TRACE("Not registered: %s\n", values["errorDescription"].mString.c_str());
					}
				});
			}
		}
		
		if(mFBProfileTexture == null)
		{
			if(Facebook::UserID() != 0)
			{
				mFBProfileTexture = Texture::FromFBID(Facebook::UserID(), 64, 64);
			}
		}
		else
		{
			if(mFBProfileTexture->IsValid())
			{
				if(mFBProfileButton == null)
				{
					mFBProfileButton = new UI::ImageButton(mFBProfileTexture, Vec2(300, 160));
					AddUIItem(mFBProfileButton);
				}
			}
		}
	}
	else
	{
		if(mFBProfileButton != null)
		{
			RemoveUIItem(mFBProfileButton);
			Delete(mFBProfileButton);
		}
		if(mFBProfileTexture != null)
		{
			::Release(mFBProfileTexture);
		}
	}
	mPlayButton->mEnabled = mGameID != 0 && mBoardID != -1 && mStartTime != 0 && mStartTime <= mNetworkTime && mNetworkTime < mEndTime;
	mPostButton->mEnabled = Facebook::IsLoggedIn();
	return UI::Screen::Update();
}

//////////////////////////////////////////////////////////////////////

void MainUI::Draw()
{
	UI::Screen::Draw();
	mSpriteList->ResetTransform();
	if(Facebook::IsLoggedIn())
	{
		g_DebugFont->DrawString(mSpriteList, Format("Logged in as %s", Facebook::Username().c_str()), Vec2(160, 240));
	}
	
	g_DebugFont->DrawString(mSpriteList, Format("GameID: %lld, BoardID: %lld", mGameID, mBoardID).c_str(), Vec2(160, 260));

	string message;
	if(mStartTime != 0)
	{
		if(mStartTime > mNetworkTime)
		{
			message = Format("%d seconds until game starts", (int)(mStartTime - mNetworkTime));
		}
		else if(mEndTime > mNetworkTime)
		{
			int seconds = (int)(mEndTime - mNetworkTime);
			mGameTimeRemaining = seconds;
			int minutes = seconds / 60;
			seconds %= 60;
			message = Format("Game ends in %d:%02d minutes", minutes, seconds);
		}
		else
		{
			message = Format("Game ended, waiting for another...");
		}
	}
	g_DebugFont->DrawString(mSpriteList, message.c_str(), Vec2(160, 300));
}

//////////////////////////////////////////////////////////////////////

MainUI::~MainUI()
{	
	if(mFBProfileButton != null)
	{
		RemoveUIItem(mFBProfileButton);
		Delete(mFBProfileButton);
	}
	RemoveUIItem(mPracticeButton);
	RemoveUIItem(mPlayButton);
	RemoveUIItem(mLoginButton);
	RemoveUIItem(mPostButton);
	Delete(mPracticeButton);
	Delete(mPlayButton);
	Delete(mLoginButton);
	Delete(mPostButton);
	::Release(mFBProfileTexture);
	::Release(mPlayButtonTexture);
	Delete(mGame);
	Delete(mResultsUI);
	::Release(mLabelFont);
}

