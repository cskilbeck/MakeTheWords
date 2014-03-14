//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	struct Board
	{
		int					mWidth;
		int					mHeight;

		Point2D				mSize;			// size in tiles
		Point2D				mTileSize;		// how big a tile is in pixels
		Point2D				mTileSourceSize;
		Point2D				mBoardSize;		// size of board in pixels
		float				mFontScale;

		Tile *				mTiles;
		Tile **				mSortedTiles;
		int					mNumTiles;
		bool				mAtRest;

		SpriteList *		mSpriteList;
		Dictionary *		mDictionary;

		vector<Word *>		mSortVector;

		Pool<Word>			mWordPool;
		linked_list<Word>	mFoundWords;
		linked_list<Word>	mValidWords;
		linked_list<Word>	mPreviousWords;

		static const int kMaxSize = 7;

		Board();
		~Board();

		Tile *GetTile(int x, int y)
		{
			return mTiles + x + y * mWidth;
		}

		Tile *GetTile(Point2D const &p)
		{
			return mTiles + p.x + p.y * mWidth;
		}

		Tile *GetTileAtScreenPosition(Point2D const &point)	// screen coords
		{
			Point2D p(point / mTileSize);
			return (p.x >= 0 && p.x < mWidth && p.y >= 0 && p.y < mHeight) ? GetTile(p) : null;
		}

		Tile &GetWordTile(Word *w, int index)
		{
			int yo = (int)w->mOrientation;	// YoinK! don't touch that enum...
			int xo = 1 - yo;
			return mTiles[(w->mLocation.x + xo * index) + (w->mLocation.y + yo * index) * mWidth];
		}

		wstring GetWideWordText(Word *w)
		{
			wstring s;
			s.reserve(w->mLength);
			for(int i=0; i<w->mLength; ++i)
			{
				s += (wchar)tolower(GetWordTile(w, i).mLetter);
			}
			return s;
		}
		
		string GetAsString() const
		{
			string s;
			s.reserve(mNumTiles);
			for(int i=0; i<mNumTiles; ++i)
			{
				s += mTiles[i].mLetter;
			}
			return s;
		}

		string GetWordText(Word *w)
		{
			string s;
			s.reserve(w->mLength);
			for(int i=0; i<w->mLength; ++i)
			{
				s += (char)tolower(GetWordTile(w, i).mLetter);
			}
			return s;
		}

		void Init(int boardWidth, int boardHeight, Dictionary *dictionary, SpriteList *spriteList);
		void Flip();
		void Reset(uint32 seed = 0);
		void Copy(Board const &other);
		void ResetTileIndices();
		void ResetTilePositions();
		void SetAllTileStates(Tile::State state);
		void InitializeTileLetters(uint32 seed);
		void ClearWordLists();
		bool Update(float deltaTime);
		int MarkAllWords();
		void Draw();

	private:

		void MarkWordPass(Word::Orientation orientation, int offsetVector, int limit, int xMul, int yMul);
	};

	//////////////////////////////////////////////////////////////////////

} //::Game

//////////////////////////////////////////////////////////////////////

