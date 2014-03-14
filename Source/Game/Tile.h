//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	struct Word;

	//////////////////////////////////////////////////////////////////////

	struct Tile
	{
		enum WordPosition : uint8
		{
			None = 0,
			Beginning = 1,
			Middle = 2,
			End = 3
		};

		enum State
		{
			kInvalidState,
			kLerp,					// just lerp from origin to target
			kSelected,				// wibble in place
			kBeingDragged,
			kDrop					// Fall down the screen
		};

		Word *				mHorizontalWord;
		Word *				mVerticalWord;
		int					mHorizontalIndex;
		int					mVerticalIndex;
		WordPosition		mHorizontalWordPosition;
		WordPosition		mVerticalWordPosition;

		Vec2				mPosition;		// current draw position
		Vec2				mTarget;		// where its headed
		Vec2				mOrigin;		// where its come from

		float				mDropVelocity;
		int					mBounced;

		bool				mAtRest;
		bool				mSelected;

		char				mLetter;
		int					mLayer;
		Index				mIndex;

		State				mState;
		State				mOldState;

		float				mStateTime;

		static Font *		sTileFont;
		static Texture *	sTileTexture;
		static SpriteList *	sSpriteList;
		static Timer		sTimer;

		float				mRandomFloat;

		bool Update(float deltaTime);
		void ResetPosition();
		void SetupTransform(Point2D const &tileSize, float scaleFactor);
		void DrawTile(Point2D const &tileSize, Point2D const &tileSourceSize);
		void DrawLetter(Point2D const &tileSize, float letterScale);
		void GetUVs(Point2D &tl, Point2D &br);

		bool operator < (Tile const &other)
		{
			return mLayer < other.mLayer;
		}

		void ResetWords()
		{
			mHorizontalWord = mVerticalWord = null;
			mHorizontalIndex = mVerticalIndex = 0;
			mHorizontalWordPosition = mVerticalWordPosition = WordPosition::None;
		}

		void SetWord(Word *w, int index);

		static void Open(SpriteList *spriteList);
		static void Close();
	};

} // ::Game