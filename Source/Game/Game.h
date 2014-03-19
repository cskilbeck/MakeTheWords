//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	struct Game : Component
	{
		Game(SpriteList *spriteList, Dictionary *dictionary);
		~Game();

		void Init(uint32 seed, uint64 boardID, uint64 facebookID, uint64 gameID, double gameTimeRemaining, double gameDuration);
		void Start();
		eComponentReturnCode Update();
		void Draw();
		void Flip();	// change orientation

		// private:

		enum State
		{
			kIdle,
			kPickDirection,
			kDragTile,
			kSwapTiles,
			kShowDefinition,
			kGameOver,
			kWaveTileAround
		};

		void Quit();

		void Idle();
		void PickDirection();
		void DragTile();
		void SwapTiles();
		void ShowDefinition();
		void WaveTileAround();

		void MarkAllWords(bool effects = false);

		enum Direction
		{
			kNone,
			kUp,
			kRight,
			kDown,
			kLeft
		};

		Dictionary *		mDictionary;

		Board				mBoard;
		Tile *				mUndoTiles;

		int					mCurrentScore;

		bool				mReScoreRequired;
		
		bool				mGameOver;

		double				mStateTime;

		int					mFrames;

		uint32				mSeed;

		State				mState;
		State				mOldState;
		int					mStateFrames;

		Direction			mDragDirection;
		Point2D				mTile;
		Point2D				mPick;
		Point2D				mTilePos;

		SpriteList *		mSpriteList;
		Tile *				mActiveTile;

		Tile *				mFirstClickedTile;
		Tile *				mHoverTile;

		DefinitionScreen *	mDefinitionScreen;

		Font *				mScoreFont;
		Font *				mTextFont;

		static const int	kMaxUndo = 100;

		Index *				mUndoStack;
		int					mUndoPointer;
		int					mUndoLength;

		InGameUI *			mInGameUI;
		Particles *			mParticles;

		string				mPostedBoard;
		string				mPostedBoardAttempt;

		uint32				mPostedScore;
		uint32				mLeaderboardSize;
		uint32				mLeaderboardPosition;
		uint64				mFacebookID;
		uint64				mGameID;
		uint64				mBoardID;
		URLLoader			mURLLoader;
		double				mGameTimeRemaining;
		double				mGameDuration;
		double				mLastPostTime;

		bool				mSimpleMode;

		function<void()>	OnQuit;

		void InitUndo();
		void ResetUndo();
		void PushUndo();
		void PopUndo();

		void ShuntTiles(Direction dragDirection, Point2D const &tilePos, Point2D const &whichTile, Point2D const &direction);
		
		void PostBoard(bool final);

		Direction GetDirection();

		Index &UndoStack(int undo, int tile)
		{
			return mUndoStack[undo * mBoard.mNumTiles + tile];
		}
	};

} // ::Game