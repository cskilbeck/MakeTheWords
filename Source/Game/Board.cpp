//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	Board::Board() : mTiles(null)
	{
		mSortVector.reserve(Word::kMaxWords);
	}

	//////////////////////////////////////////////////////////////////////

	Board::~Board()
	{
		SafeDeleteArray(mSortedTiles);
		SafeDeleteArray(mTiles);
		mNumTiles = 0;
	}

	//////////////////////////////////////////////////////////////////////

	void Board::Copy(Board const &other)
	{
		if(other.mSize == mSize)
		{
			for (int i = 0; i < mNumTiles; ++i)
			{
				mTiles[i].mLetter = other.mTiles[i].mLetter;
			}
		}
	}
	
	//  480 x  320
	//  960 x  640
	// 1136 x  640
	//  768 x 1024
	// 1536 x 2048

	//////////////////////////////////////////////////////////////////////

	void Board::Reset(uint32 seed)
	{
		ResetTilePositions();
		InitializeTileLetters(seed);
		ResetTileIndices();

		Random r;
		for(int i=0; i<mNumTiles; ++i)
		{
			mTiles[i].mDropVelocity = -r.NextFloat() * 200;
			mTiles[i].mPosition.y -= Screen::Height();
			mTiles[i].mOldState = Tile::kInvalidState;
			mTiles[i].mState = Tile::kDrop;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Board::Init(int boardWidth, int boardHeight, Dictionary *dictionary, SpriteList *spriteList)
	{
		mSpriteList = spriteList;
		mDictionary = dictionary;
		mWidth = boardWidth;
		mHeight = boardHeight;
		mSize = Point2D(mWidth, mHeight);
		mNumTiles = mWidth * mHeight;
		mTiles = new Tile[mNumTiles];
		mSortedTiles = new Tile *[mNumTiles];

		// Ho hum - decide this based on screen resolution
		mTileSize = Point2D(60, 64);
		mFontScale = 0.725f;
		
		mTileSourceSize = Point2D(96, 96);
		mBoardSize = Point2D(mTileSize.x * mWidth, mTileSize.y * mHeight);
		mWordPool.Resize(Word::kMaxWords);
		Reset();
	}

	//////////////////////////////////////////////////////////////////////

	void Board::Flip()
	{
		Tile *newTiles = new Tile[mNumTiles];

		int i = 0;
		for(int y=0; y<mHeight; ++y)
		{
			for(int x=0; x<mWidth; ++x)
			{
				Tile *src = GetTile(x, y);
				Tile *dst = &newTiles[x * mHeight + y];

				dst->mIndex = Index(i, mWidth);
				dst->mLetter = src->mLetter;
				dst->mSelected = false;
				dst->mLayer = 0;
				++i;
			}
		}

		std::swap(mWidth, mHeight);
		SafeDeleteArray(mTiles);
		mTiles = newTiles;
		ResetTilePositions();
		MarkAllWords();
		mBoardSize = Point2D(mTileSize.x * mWidth, mTileSize.y * mHeight);
		mSize = Point2D(mWidth, mHeight);
	}

	//////////////////////////////////////////////////////////////////////

	void Board::ResetTileIndices()
	{
		for (int i = 0; i < mNumTiles; ++i)
		{
			mTiles[i].mIndex = Index(i, mWidth);
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Board::ResetTilePositions()
	{
		for(int y=0; y<mHeight; ++y)
		{
			for(int x=0; x<mWidth; ++x)
			{
				Tile *t = GetTile(x, y);
				t->mOrigin = Vec2((float)x * mTileSize.x, (float)y * mTileSize.y);
				t->mTarget = t->mOrigin;
				t->mPosition = t->mOrigin;
				t->mAtRest = true;
				t->mSelected = false;
				t->mLayer = 0;
				t->mOldState = Tile::kInvalidState;
				t->mState = Tile::kLerp;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Board::SetAllTileStates(Tile::State state)
	{
		for(int i=0; i<mNumTiles; ++i)
		{
			mTiles[i].mOldState = Tile::kInvalidState;
			mTiles[i].mState = state;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Board::InitializeTileLetters(uint32 seed)
	{
		Reference::Letter::SetRandomSeed(seed);

		for(int i=0; i<mNumTiles; ++i)
		{
			mTiles[i].mLetter = Reference::Letter::GetRandomLetter();
		}

		Random r(0);

		while(MarkAllWords() > 0)
		{
			for(auto &w: mValidWords)
			{
				int l = r.Next() % w.mLength;
				Tile &t = GetWordTile(&w, l);
				t.mLetter = Reference::Letter::GetRandomLetter();
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	bool Board::Update(float deltaTime)
	{
		bool oar = mAtRest;
		mAtRest = true;
		for(int i=0; i<mNumTiles; ++i)
		{
			mAtRest &= mTiles[i].Update(deltaTime);
		}
		return !oar && mAtRest;
	}

	//////////////////////////////////////////////////////////////////////

	void Board::ClearWordLists()
	{
		while(!mFoundWords.empty())
		{
			mWordPool.Free(mFoundWords.pop_back());
		}
		while(!mPreviousWords.empty())
		{
			mWordPool.Free(mPreviousWords.pop_back());
		}
		while(!mValidWords.empty())
		{
			mPreviousWords.push_back(mValidWords.pop_front());
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Board::MarkWordPass(Word::Orientation orientation, int offsetVector, int limit, int xMul, int yMul)
	{
		char checkString[kMaxSize + 1];

		int xLim = mWidth - 2 * xMul;
		int yLim = mHeight - 2 * yMul;

		for (int y = 0; y < yLim; ++y)
		{
			for (int x = 0; x < xLim; ++x)
			{
				int n = x + y * mWidth;
				int t = x * xMul + y * yMul;

				for (int e = 3; e + t <= limit; ++e)
				{
					int m = n;
					int i = 0;
					for (; i < e; ++i)
					{
						checkString[i] = (char)(mTiles[m].mLetter + 32);
						m += offsetVector;
					}
					checkString[i] = '\0';
					int w = mDictionary->WordIndex(checkString);
					if(w != -1)
					{
						Word *word = mWordPool.Alloc();
						if(word != null)
						{
							word->mLocation.x = x;
							word->mLocation.y = y;
							word->mOrientation = orientation;
							word->mScore = Reference::Letter::GetWordScore(checkString);
							word->mLength = e;
							word->mDictionaryWord = w;
							mFoundWords.push_back(word);
						}
					}
				}
			}
		}
	}

	int Board::MarkAllWords()
	{
		ClearWordLists();

		for(int i=0; i<mNumTiles; ++i)
		{
			mTiles[i].ResetWords();
		}

		// find ALL horizontal and vertical words
		MarkWordPass(Word::Orientation::horizontal, 1, mWidth, 1, 0);
		MarkWordPass(Word::Orientation::vertical, mWidth, mHeight, 0, 1);

		// sort them by score,length
		mSortVector.resize(0);
		for(auto &w: mFoundWords)
		{
			mSortVector.push_back(&w);
		}

		sort(mSortVector.begin(), mSortVector.end(), [] (Word *a, Word *b)
		{
			if(a->mScore != b->mScore)
			{
				return a->mScore > b->mScore;
			}
			else if(a->mLength != b->mLength)
			{
				return a->mLength > b->mLength;
			}
			else
			{
				return a->mDictionaryWord > b->mDictionaryWord;
			}
		});

		// delete overlapped words and set up tile flags
		int totalScore = 0;

		for(auto &word: mSortVector)
		{
			bool valid = true;

			for(int i=0; i<word->mLength; ++i)
			{
				Tile &t = GetWordTile(word, i);

				if(t.mVerticalWord != null && word->mOrientation == Word::Orientation::vertical)
				{
					valid = false;
					break;
				}
				if(t.mHorizontalWord != null && word->mOrientation == Word::Orientation::horizontal)
				{
					valid = false;
					break;
				}
			}
			mFoundWords.remove(word);
			if (valid)
			{
				mValidWords.push_back(word);
				Word &w = *word;
				word->mNew = chs::find_first_of(mPreviousWords, w) == mPreviousWords.end();
				for (int i = 0; i < word->mLength; ++i)
				{
					Tile &t = GetWordTile(word, i);
					t.SetWord(word, i);
				}
				totalScore += word->mScore;
			}
			else
			{
				mWordPool.Free(word);
			}
		}

		return totalScore;
	}

	//////////////////////////////////////////////////////////////////////

	void Board::Draw()
	{
		for(int i=0; i<mNumTiles; ++i)
		{
			mSortedTiles[i] = &mTiles[i];
		}

		sort(mSortedTiles, mSortedTiles + mNumTiles, [&] (Tile *a, Tile *b)
		{
			return a->mLayer < b->mLayer;
		});

		auto it = mSortedTiles;
		auto end = mSortedTiles + mNumTiles;
		auto runStart = it;
		int currentLayer = (*it)->mLayer;

		for(; it != end; ++it)
		{
			if((*it)->mLayer != currentLayer)
			{
				currentLayer = (*it)->mLayer;
				for(; runStart != it; ++runStart)
				{
					(*runStart)->DrawLetter(mTileSize, mFontScale);
				}
			}
			(*it)->DrawTile(mTileSize, mTileSourceSize);
		}
		for(; runStart != it; ++runStart)
		{
			(*runStart)->DrawLetter(mTileSize, mFontScale);
		}

#if 0
		for(int i=0; i<mNumTiles; ++i)
		{
			Point2D p(mTiles[i].mPosition + Vec2(40, 70));
			DBG(p.x, p.y, L"%d,%d", mTiles[i].mIndex.x, mTiles[i].mIndex.y);
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////

} // ::Game