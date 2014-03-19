//////////////////////////////////////////////////////////////////////
// Put everything in namespaces
	// Game
	// Base
	// Gfx
	// *UI
// Deal with Portrait / Landscape transition
// Loading screen
// Save/Restore state
// Proper UI
// Facebook leaderboards
//

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace Game
{
	//////////////////////////////////////////////////////////////////////

	static const float definitionRadius = 12.0f;

	//////////////////////////////////////////////////////////////////////

	Game::Game(SpriteList *spriteList, Dictionary *dictionary)
		: Component(spriteList)
		, mSpriteList(spriteList)
		, mDictionary(dictionary)
		, mUndoStack(null)
		, mUndoTiles(null)
		, mHoverTile(null)
	{
		mActive = false;
		mVisible = false;

		TRACE(L"Current folder: %s\n", GetCurrentFolder().c_str());


#if !defined(IOS)
		mScoreFont = FontManager::Load("score");
#else
		mScoreFont = FontManager::Load("score");
#endif
		mTextFont = FontManager::Load("definitionBody");

		mDefinitionScreen = new DefinitionScreen(spriteList, Screen::Width(), Screen::Height());
		mDefinitionScreen->mVisible = false;
		mDefinitionScreen->mActive = false;
		mDefinitionScreen->mZIndex = 3;

		mBoard.Init(7, 5, mDictionary, mSpriteList);

		Tile::Open(mSpriteList);

		InitUndo();

		mInGameUI = new InGameUI(mSpriteList);
		mInGameUI->mActive = false;
		mInGameUI->mVisible = false;
		mInGameUI->mUndoButton->OnPress = [this]
		{
			PopUndo();
		};
		mInGameUI->mQuitButton->OnPress = [this]
		{
			Quit();
		};

		mParticles = new Particles(mSpriteList);

		mSimpleMode = true;

		Init(0, 0, 0, 0, 0, 0);
	}

	//////////////////////////////////////////////////////////////////////

	void Game::Init(uint32 seed, uint64 boardID, uint64 facebookID, uint64 gameID, double gameTimeRemaining, double gameDuration)
	{
		mLastPostTime = g_Time;
		mGameTimeRemaining = gameTimeRemaining;
		mGameDuration = gameDuration;
		mGameOver = false;
		mPostedScore = 0;
		mBoardID = boardID;
		mFacebookID = facebookID;
		mGameID = gameID;
		TRACE("GameID is %d\n", mGameID);
		mSeed = seed;
		mActiveTile = null;
		mFirstClickedTile = null;
		mFrames = 0;
		mState = kInit;
		mReScoreRequired = false;
		mStateTime = 0;
		mCurrentScore = 0;
		mBoard.Reset(mSeed);
		ResetUndo();

		// highest ZIndex goes at the front (drawn last, processed first)

		mInGameUI->mZIndex = 0;
		mZIndex = 1;			// process and draw 'this' before the in-game UI
		mParticles->mZIndex = 2;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::Start()
	{
		mDefinitionScreen->mActive = false;
		mDefinitionScreen->mVisible = false;
		mVisible = true;
		mActive = true;
		mInGameUI->mActive = true;
		mInGameUI->mVisible = true;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::Quit()
	{
		mDefinitionScreen->mVisible = false;
		mDefinitionScreen->mActive = false;
		mActive = false;
		mVisible = false;
		mInGameUI->mActive = false;
		mInGameUI->mVisible = false;
		if(OnQuit != null)
		{
			OnQuit();
		}
	}

	//////////////////////////////////////////////////////////////////////

	Game::~Game()
	{
		URLLoader::TerminateAll();
		::Release(mParticles);
		Delete(mInGameUI);
		::Release(mScoreFont);
		::Release(mTextFont);
		Delete(mDefinitionScreen);
		Delete(mUndoStack);
		Delete(mUndoTiles);
		Tile::Close();
	}

	//////////////////////////////////////////////////////////////////////

	void Game::Flip()
	{
		mBoard.Flip();
		for(int i=0; i<kMaxUndo * mBoard.mNumTiles; ++i)
		{
			mUndoStack[i].Flip();
		}
	}

	//////////////////////////////////////////////////////////////////////

	Component::eComponentReturnCode Game::Update()
	{
		++mFrames;
		
		mGameTimeRemaining -= g_DeltaTime;
		
		if(mState != mOldState)
		{
			mStateTime = 0;
			mOldState = mState;
			mStateFrames = 0;
		}
		else
		{
			mStateTime += g_DeltaTime;
			++mStateFrames;
		}

		// end practice game...
		if(mGameID == 0 && mGameTimeRemaining <= 0)
		{
			Quit();
		}
		
		// if idle, grab status every N seconds
		if(mStateFrames > 30 && mURLLoader.GetState() != URLLoader::State::Busy && (g_Time - mLastPostTime) > 1)			// and score has changed from when we last posted...
		{
			PostBoard(false);
		}

		if(mGameOver)
		{
			// Pass back some info to the MainUI for final scoring...?
			Quit();
		}

		if(Keyboard::Pressed[' '])
		{
			Flip();
		}

		bool rescan = mBoard.Update((float)g_DeltaTime);

		if(rescan && mReScoreRequired)
		{
			MarkAllWords(false);
			mReScoreRequired = false;
			for(int i=0; i<mBoard.mNumTiles; ++i)
			{
				mBoard.mTiles[i].mLayer = 0;
			}
		}

		switch(mState)
		{
		case kInit:
			if(mBoard.mAtRest)
			{
				mState = kIdle;
			}
			break;

		case kIdle:
			Idle();
			break;

		case kPickDirection:
			PickDirection();
			break;

		case kDragTile:
			DragTile();
			break;

		case kSwapTiles:
			SwapTiles();
			break;

		case kShowDefinition:
			if(mDefinitionScreen->mActive == false)
			{
				mState = kIdle;
			}
			break;
				
		case kGameOver:
			if(mGameID == 0)	// offline game just exits
			{
				mGameOver = true;
			}
			break;

		case kWaveTileAround:
			WaveTileAround();
			break;
		}
		return kAllow;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::Draw()
	{
		mSpriteList->ResetTransform();
		mBoard.Draw();

		if(mState == kGameOver)
		{
			mTextFont->DrawString(mSpriteList, "Game Over", Vec2(240, 160), Font::HCentre, Font::VCentre);
		}
		else
		{
			int leaderboardHeight = 200;
			int scorePos = 0;
			if(mLeaderboardSize > 0)
			{
				scorePos = mLeaderboardPosition * leaderboardHeight / mLeaderboardSize;
			}
			mSpriteList->ResetTransform();
			mSpriteList->SetBlendingMode(SpriteList::kModulate);
			string score = Format("%d", mCurrentScore);
			Vec2 offset;
			Vec2 s = mScoreFont->MeasureString(score.c_str(), offset);
			float diff = (Screen::Width() - mBoard.mBoardSize.x) / 2.0f;
			mScoreFont->DrawString(mSpriteList, score, Vec2(Screen::Width() - diff, scorePos + 70.0f), Font::HCentre, Font::VCentre);

			mSpriteList->SetTexture(SpriteList::WhiteTexture());
			mSpriteList->SetBlendingMode(SpriteList::kAdditive);
			float r = (float)(mGameTimeRemaining * PI * 2 / mGameDuration);
			Color c = Color(255, 255, 0);
			int mod = 0;
			if(mGameTimeRemaining < 15)	// TODO: these 15/30 second values need to be set server side...
			{
				mod = 2;
			}
			else if(mGameTimeRemaining < 30)
			{
				mod = 4;
			}
			if(mod && ((int)(g_Time * 4) % mod) < (mod / 2))
			{
				c = Color(255, 0, 0);
			}
			Vec2 org(Screen::Width() - 27.0f, 128);
			mSpriteList->AddArc(org, 10, 22, PI * 3, PI * 3 - r, 32, c);
			mSpriteList->AddArc(org, 21, 24, 0, PI * 2, 32, Color::White);
			mSpriteList->AddCircle(org, 11, 32, Color::White);
		}
	}

	//////////////////////////////////////////////////////////////////////
	
	void Game::PostBoard(bool final)
	{
		if(mGameID != 0)
		{
			if(mBoardID == -1)
			{
				mBoardID = 0;
			}
			mPostedBoardAttempt = mBoard.GetAsString();
			string post = Format("board_id=%llu&board=%s&facebook_id=%llu&game_id=%d", mBoardID, mBoard.GetAsString().c_str(), mFacebookID, mGameID);
			if(final)
			{
				post += "&final=1";
			}

			mURLLoader.Load(WebServiceURL("post").c_str(), post, [this] (bool success, ValueMap values)
			{
				mLastPostTime = g_Time;
			
				if(!success)
				{
					int errorCode = (int)values["error"];
					if(errorCode == 4 || errorCode == 5)
					{
						// Game has ended, and we posted after getting 'final' notification
						DumpValueMap(values);
						//mState = kGameOver;
					}
				}
				else
				{
					mPostedBoard = mPostedBoardAttempt;
					if(values.find("final") != values.end())
					{
						mState = kGameOver;
						mGameOver = true;
					}
					else
					{
						//DumpValueMap(values);
						mPostedScore = values["score"];
						mLeaderboardPosition = values["leaderboard_position"];
						mLeaderboardSize = values["leaderboard_size"];
						mBoardID = values["board_id"];
						time_t serverTime = values["current_time"].mDateTime;
						time_t endTime = values["end_time"].mDateTime;
						mGameTimeRemaining = (double)(endTime - serverTime);
					}
				}
			});
		}
	}

	//////////////////////////////////////////////////////////////////////
	
	void Game::Idle()
	{
		if(mBoard.mAtRest)
		{
#if defined(DEBUG)

			if(mFirstClickedTile != null)
			{
				if(Keyboard::Pressed[27])
				{
					mFirstClickedTile->mLayer = 0;
					mFirstClickedTile->mState = Tile::kLerp;
					mFirstClickedTile->mSelected = false;
					mFirstClickedTile = null;
				}
				else
				{
					for(int i='A'; i<='Z'; ++i)
					{
						if(Keyboard::Pressed[i])
						{
							mFirstClickedTile->mLetter = i;
							mBoard.MarkAllWords();

							int y = mFirstClickedTile->mIndex.y;
							int x = mFirstClickedTile->mIndex.x;

							mFirstClickedTile->mLayer = 0;
							mFirstClickedTile->mState = Tile::kLerp;
							mFirstClickedTile->mSelected = false;
							mFirstClickedTile = null;

							++x;

							if(x < mBoard.mWidth)
							{
								mFirstClickedTile = mBoard.GetTile(x, y);
								mFirstClickedTile->mLayer = 1;
								mFirstClickedTile->mSelected = true;
								mFirstClickedTile->mState = Tile::kSelected;
							}
							// select the next one if it's not in the right hand column?
							break;
						}
					}
				}
			}

#endif

			if(Touch(0).Pressed())
			{
				Tile *t = mBoard.GetTileAtScreenPosition(Touch(0).mPosition);
				if(t != null)
				{
					mPick = Touch(0).mPosition;
					mBoard.ResetTileIndices();
					mActiveTile = t;
					mActiveTile->mLayer = 1;
					mTile = Point2D(mActiveTile->mOrigin);
					mTilePos = Point2D(Touch(0).mPosition / mBoard.mTileSize);
					mState = kPickDirection;
					mDragDirection = kNone;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	Game::Direction Game::GetDirection()
	{
		Vec2 d = Vec2((Touch(0).mPosition - mPick) * 2);

		return	(d.x < -mBoard.mTileSize.x)	?	Direction::kLeft :
				(d.x >  mBoard.mTileSize.x)	?	Direction::kRight :
				(d.y < -mBoard.mTileSize.y)	?	Direction::kUp :
				(d.y >  mBoard.mTileSize.y)	?	Direction::kDown :
				Direction::kNone;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::PickDirection()
	{
		if(Touch(0).Released())
		{
			Tile *t = mBoard.GetTileAtScreenPosition(Touch(0).mPosition);

			if(t == mActiveTile)
			{
				if(mFirstClickedTile == null)
				{
					mFirstClickedTile = mActiveTile;
					if(mActiveTile != null)
					{
						mActiveTile->mState = Tile::kSelected;
						mActiveTile->mHorizontalWordPosition = Tile::WordPosition::None;
						mActiveTile->mVerticalWordPosition = Tile::WordPosition::None;
						for(int i=0; i<mBoard.mNumTiles; ++i)
						{
							mBoard.mTiles[i].mLayer = 0;
						}
						mActiveTile->mLayer = 1;
						mFirstClickedTile->mLayer = 2;
					}
				}
				else if(mFirstClickedTile != mActiveTile)
				{
					mFirstClickedTile->mTarget = mActiveTile->mOrigin;
					mActiveTile->mTarget = mFirstClickedTile->mOrigin;		// swap targets so they swoop past eachother

					mFirstClickedTile->mState = Tile::kLerp;
					mActiveTile->mState = Tile::kLerp;

					mFirstClickedTile->mLayer = 2;
					mActiveTile->mLayer = 1;

					mActiveTile->mHorizontalWordPosition = Tile::WordPosition::None;
					mActiveTile->mVerticalWordPosition = Tile::WordPosition::None;

					mActiveTile->mState = Tile::kLerp;
					mFirstClickedTile->mState = Tile::kLerp;
					mState = kSwapTiles;
					return;
				}
				else
				{
					mFirstClickedTile->mState = Tile::kLerp;
					mFirstClickedTile->mLayer = 0;
					mFirstClickedTile = null;
					mState = kIdle;
					MarkAllWords(true);
				}
			}
			mActiveTile = null;
			mState = kIdle;
			return;
		}
		
		if(mStateTime > 0.5f)
		{
			Word *horiz = mActiveTile->mHorizontalWord;
			Word *vert = mActiveTile->mVerticalWord;

			if(horiz != null || vert != null)
			{
				mActiveTile->mLayer = 0;
				mActiveTile = null;
				mState = kShowDefinition;
				string addr;
				if(horiz != null)
				{
					addr += mDictionary->GetWord(horiz->mDictionaryWord);
				}
				if(vert != null)
				{
					if(!addr.empty())
					{
						addr += ",";
					}
					addr += mDictionary->GetWord(vert->mDictionaryWord);
				}
				mDefinitionScreen->Reset();
				mDefinitionScreen->AddDefinition(addr);
				mDefinitionScreen->PushStack(addr);
				mDefinitionScreen->mActive = true;
				mDefinitionScreen->mVisible = true;
				return;
			}
		}

		if(!mSimpleMode)
		{
			mDragDirection = GetDirection();
			if(mDragDirection != kNone)
			{
				if(mFirstClickedTile != null)
				{
					mFirstClickedTile->mLayer = 0;
					mFirstClickedTile = null;
				}

				mBoard.SetAllTileStates(Tile::kLerp);
				assert(mActiveTile != null);
				mActiveTile->mLayer = 1;
				mState = kDragTile;
			}
		}
		else
		{
			Point2D p = Touch(0).mPosition - mPick;
			if(abs(p.x) > 4 || abs(p.y) > 4)
			{
				if(mFirstClickedTile != null)
				{
					mFirstClickedTile->mLayer = 0;
					mFirstClickedTile = null;
				}
				mBoard.SetAllTileStates(Tile::kLerp);
				assert(mActiveTile != null);
				mActiveTile->mHorizontalWordPosition = Tile::None;
				mActiveTile->mVerticalWordPosition = Tile::None;
				mActiveTile->mLayer = 1;
				mState = kWaveTileAround;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Game::WaveTileAround()
	{
		int bw = (mBoard.mWidth - 1) * mBoard.mTileSize.x;
		int bh = (mBoard.mHeight - 1) * mBoard.mTileSize.y;
		Point2D bs(bw,bh);
		Point2D d = mPick - mTile;
		d = Touch(0).mPosition - d;
		d = Constrain(d, Point2D(0,0), bs);
		mActiveTile->mTarget = (Vec2)d;
		mActiveTile->mPosition = (Vec2)d;
		mActiveTile->mState = Tile::kBeingDragged;

		Point2D mid = d + Point2D(mBoard.mTileSize.x / 2, mBoard.mTileSize.y / 2);

		Tile *ht = mBoard.GetTileAtScreenPosition(mid);
		if(mHoverTile != null)
		{
			mHoverTile->mFlags.clear(Tile::kHoveredOver);
		}
		if(ht != mActiveTile)
		{
			mHoverTile = ht;
			mHoverTile->mFlags.set(Tile::kHoveredOver);
		}
			
		if(!Touch(0).Held())
		{
			Vec2 c = (mActiveTile->mPosition + (Vec2)(mBoard.mTileSize / 2)) / (Vec2)mBoard.mTileSize;
			mFirstClickedTile = mBoard.GetTile(int(c.x), int(c.y));
			mFirstClickedTile->mTarget = mActiveTile->mOrigin;
			mActiveTile->mTarget = mFirstClickedTile->mOrigin;
			mFirstClickedTile->mState = Tile::kLerp;
			mActiveTile->mState = Tile::kLerp;
			mFirstClickedTile->mLayer = 1;
			mActiveTile->mLayer = 2;
			mActiveTile->ResetWords();
			mState = kSwapTiles;
			if(mHoverTile != null)
			{
				mHoverTile->mFlags.clear(Tile::kHoveredOver);
				mHoverTile = null;
			}
			return;
		}
	}

	//////////////////////////////////////////////////////////////////////

	inline bool IsHorizontal(Game::Direction d)
	{
		return d == Game::kLeft || d==Game::kRight;
	}

	//////////////////////////////////////////////////////////////////////

	inline bool IsVertical(Game::Direction d)
	{
		return d == Game::kUp || d==Game::kDown;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::ShuntTiles(Direction dragDirection, Point2D const &tilePos, Point2D const &whichTile, Point2D const &direction)
	{
		char l = mActiveTile->mLetter;
		Index li = mActiveTile->mIndex;

		if(IsHorizontal(dragDirection))
		{
			for (int x = tilePos.x; x != whichTile.x; x -= direction.x)
			{
				Tile *t1 = mBoard.GetTile(x, whichTile.y);
				Tile *t2 = mBoard.GetTile(x - direction.x, whichTile.y);
				t1->mLetter = t2->mLetter;
				t1->mIndex = t2->mIndex;
				t1->ResetPosition();
			}
		}
		else
		{
			for (int y = tilePos.y; y != whichTile.y; y -= direction.y)
			{
				Tile *t1 = mBoard.GetTile(whichTile.x, y);
				Tile *t2 = mBoard.GetTile(whichTile.x, y - direction.y);
				t1->mLetter = t2->mLetter;
				t1->mIndex = t2->mIndex;
				t1->ResetPosition();
			}
		}

		Tile *td = mBoard.GetTile(whichTile);
		td->mLetter = l;
		td->mIndex = li;
		td->ResetPosition();

		mActiveTile->ResetPosition();
		mActiveTile->mLayer = 0;
		mActiveTile = null;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::DragTile()
	{
		Point2D inputMouse = Touch(0).mPosition;
		Point2D d = mPick - mTile;
		Point2D p = Constrain(inputMouse - d, Point2D(0,0), mBoard.mBoardSize - mBoard.mTileSize);
		Point2D dir (Point2D(mActiveTile->mPosition) - p);

		if(IsHorizontal(mDragDirection))
		{
			mActiveTile->mPosition.x = (float)p.x;
		}
		else
		{
			mActiveTile->mPosition.y = (float)p.y;
		}

		Point2D mid = Point2D(mActiveTile->mPosition) + mBoard.mTileSize / 2;
		Point2D whichTile = Constrain(mid / mBoard.mTileSize, Point2D(0,0), mBoard.mSize - Point2D(1,1));
		Point2D diff(mTilePos - whichTile);

		Point2D direction = Sgn(diff);

		Point2D mn = Min(whichTile, mTilePos);
		Point2D mx = Max(whichTile, mTilePos);

		/*
		TRACE("mid:%s, whichTile:%s, diff:%s, direction:%s, mn:%s, mx:%s\n",
				mid.ToString().c_str(),
				whichTile.ToString().c_str(),
				diff.ToString().c_str(),
				direction.ToString().c_str(),
				mn.ToString().c_str(),
				mx.ToString().c_str()
				);
		*/
		Direction newDir = kNone;
		if(IsHorizontal(mDragDirection))
		{
			if(dir.y > (mBoard.mTileSize.y / 2))
			{
				newDir = kDown;
			}
			else if(dir.y < -(mBoard.mTileSize.y / 2))
			{
				newDir = kUp;
			}

			if(newDir != kNone)
			{
				mPick.x = inputMouse.x;
				mPick.y = (int)(mActiveTile->mPosition.y + mBoard.mTileSize.y / 2);
			}
		}
		else
		{
			if(dir.x > (mBoard.mTileSize.x / 2))
			{
				newDir = kRight;
			}
			else if(dir.x < -(mBoard.mTileSize.x / 2))
			{
				newDir = kLeft;
			}

			if(newDir != kNone)
			{
				mPick.x = (int)(mActiveTile->mPosition.x + mBoard.mTileSize.x / 2);
				mPick.y = inputMouse.y;
			}
		}

		if(newDir != kNone)
		{
			ShuntTiles(mDragDirection, mTilePos, whichTile, direction);
			mDragDirection = newDir;
			mTilePos = Constrain(mPick / mBoard.mTileSize, Point2D(0,0), mBoard.mSize - Point2D(1,1));
			mActiveTile = mBoard.GetTile(mTilePos);
			mActiveTile->ResetPosition();
			mActiveTile->mHorizontalWordPosition = Tile::None;
			mActiveTile->mVerticalWordPosition = Tile::None;
			mTile = Point2D(mActiveTile->mOrigin);
			mActiveTile->mLayer = 1;
		}

		if(Touch(0).Released())
		{
			ShuntTiles(mDragDirection, mTilePos, whichTile, direction);
			mBoard.ResetTilePositions();
			PushUndo();
			MarkAllWords(true);
			mState = kIdle;
		}
		else
		{
			mActiveTile->mHorizontalWordPosition = Tile::WordPosition::None;
			mActiveTile->mVerticalWordPosition = Tile::WordPosition::None;

			if(IsHorizontal(mDragDirection))
			{
				for (int x = 0; x < mn.x; ++x)
				{
					Tile *t = mBoard.GetTile(x, mTilePos.y);	t->mTarget = t->mOrigin;
				}
				for (int x = mx.x + 1; x < mBoard.mWidth; ++x)
				{
					Tile *t = mBoard.GetTile(x, mTilePos.y);	t->mTarget = t->mOrigin;
				}
				for (int x = whichTile.x; x != mTilePos.x; x += direction.x)
				{
					Tile *t = mBoard.GetTile(x, mTilePos.y);	t->mTarget.x = t->mOrigin.x + mBoard.mTileSize.x * direction.x;
				}
			}
			else
			{
				for (int y = 0; y < mn.y; ++y)
				{
					Tile *t = mBoard.GetTile(mTilePos.x, y);	t->mTarget = t->mOrigin;
				}
				for (int y = mx.y + 1; y < mBoard.mHeight; ++y)
				{
					Tile *t = mBoard.GetTile(mTilePos.x, y);	t->mTarget = t->mOrigin;
				}
				for (int y = whichTile.y; y != mTilePos.y; y += direction.y)
				{
					Tile *t = mBoard.GetTile(mTilePos.x, y);	t->mTarget.y = t->mOrigin.y + mBoard.mTileSize.y * direction.y;
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Game::SwapTiles()
	{
		if (mBoard.mAtRest)
		{
			char letter = mFirstClickedTile->mLetter;
			Index index = mFirstClickedTile->mIndex;
			mFirstClickedTile->mLetter = mActiveTile->mLetter;
			mFirstClickedTile->mIndex = mActiveTile->mIndex;
			mActiveTile->mLetter = letter;
			mActiveTile->mIndex = index;
			mActiveTile->mState = Tile::kLerp;
			mFirstClickedTile->mState = Tile::kLerp;
			mActiveTile->mLayer = 0;
			mFirstClickedTile->mLayer = 0;
			mActiveTile->ResetPosition();
			mFirstClickedTile->ResetPosition();
			mActiveTile = null;
			mFirstClickedTile = null;
			MarkAllWords(true);
			PushUndo();
			mState = kIdle;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Game::MarkAllWords(bool effects)
	{
		mCurrentScore = mBoard.MarkAllWords();

		if(effects)
		{
			for(auto &w: mBoard.mValidWords)
			{
				if(w.mNew)
				{
					w.mNew = false;
					Vec2 start = Vec2(w.mLocation * mBoard.mTileSize + mBoard.mTileSize / 2);
					switch (w.mOrientation)
					{
					case Word::horizontal:
						mParticles->LaunchHorizontal(start, (float)((w.mLength - 1) * mBoard.mTileSize.x));
						break;

					case Word::vertical:
						mParticles->LaunchVertical(start, (float)((w.mLength - 1) * mBoard.mTileSize.y));
						break;

					default:
						break;
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Game::ResetUndo()
	{
		mUndoPointer = 0;
		mUndoLength = 0;
	}

	//////////////////////////////////////////////////////////////////////

	void Game::InitUndo()
	{
		mUndoTiles = new Tile[mBoard.mNumTiles];
		mUndoStack = new Index[kMaxUndo * mBoard.mNumTiles];
		ResetUndo();
	}

	//////////////////////////////////////////////////////////////////////

	void Game::PushUndo()
	{
		for (int i = 0; i < mBoard.mNumTiles; ++i)
		{
			Index idx(i, mBoard.mWidth);
			if (mBoard.mTiles[i].mIndex != idx)
			{
				for (int t = 0; t < mBoard.mNumTiles; ++t)
				{
					UndoStack(mUndoPointer, t) = mBoard.mTiles[t].mIndex;
				}
				mUndoPointer = ++mUndoPointer % kMaxUndo;
				mUndoLength = Min(kMaxUndo, ++mUndoLength);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Game::PopUndo()
	{
		if (mUndoLength > 0)
		{
			--mUndoLength;
			--mUndoPointer;

			if (mUndoPointer < 0)
			{
				mUndoPointer = kMaxUndo - 1;
			}

			//InGameButton.GetButton(InGameButton.Command.Undo).enabled = mUndoLength > 0;

			if(mFirstClickedTile != null)
			{
				mFirstClickedTile->mSelected = false;
				mFirstClickedTile->mLayer = 0;
				mFirstClickedTile->mState = Tile::kLerp;
			}

			mActiveTile = null;

			for (int i = 0; i < mBoard.mNumTiles; ++i)
			{
				mUndoTiles[i] = mBoard.mTiles[i];
			}

			for (int i = 0; i < mBoard.mNumTiles; ++i)
			{
				mUndoTiles[i].mIndex = Index(i, mBoard.mWidth);
				mBoard.mTiles[i].mIndex = UndoStack(mUndoPointer, i);
				mUndoTiles[i].mOrigin = 
					mBoard.mTiles[i].mOrigin = Vec2(Point2D(i % mBoard.mWidth * mBoard.mTileSize.x, i / mBoard.mWidth * mBoard.mTileSize.y));
				mBoard.mTiles[i].ResetPosition();
			}

			for (int i = 0; i < mBoard.mNumTiles; ++i)
			{
				Index sourceIndex = mUndoTiles[i].mIndex;
				Index targetIndex = mBoard.mTiles[i].mIndex;

				if (sourceIndex != targetIndex)
				{
					Tile &target = mBoard.mTiles[targetIndex.Offset(mBoard.mWidth)];
					Tile &source = mUndoTiles[sourceIndex.Offset(mBoard.mWidth)];
					target.mPosition = source.mOrigin;
					target.mTarget = target.mOrigin;
					target.mLayer = 1;
					source.mLayer = 1;
					target.mLetter = source.mLetter;
					target.ResetWords();
				}
			}

			mBoard.ResetTileIndices();
			mState = kIdle;
			mReScoreRequired = true;
		}
	}

	//////////////////////////////////////////////////////////////////////

} // ::Game