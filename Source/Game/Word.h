//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	static const int kMaxWords = 209;	// ha!

	struct Word : list_node<Word>, Pooled<Word, kMaxWords>	// huh, can't get this from inside the struct...
	{
		static pool_t pool;

		enum Orientation
		{
			horizontal = 0,
			vertical = 1
		};

		bool operator == (Word const &other)
		{
			return mLocation == other.mLocation && mDictionaryWord == other.mDictionaryWord;
		}

		bool operator < (Word const &other)
		{
			if(mScore != other.mScore)
			{
				return mScore > other.mScore;
			}
			else if(mLength != other.mLength)
			{
				return mLength > other.mLength;
			}
			else
			{
				return mDictionaryWord > other.mDictionaryWord;
			}
		}

		Word(int x, int y, Word::Orientation orientation, int score, int length, int dictionaryWord);

		Point2D			mLocation;
		int				mLength;
		Orientation		mOrientation;
		int				mScore;
		int				mGroup;
		int				mDictionaryWord;
		bool			mNew;
	};

} // ::Game