//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	struct Word : list_node<Word>
	{
		static const int kMaxWords = 209;	// ha!

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

		Point2D			mLocation;
		int				mLength;
		Orientation		mOrientation;
		int				mScore;
		int				mGroup;
		int				mDictionaryWord;
		bool			mNew;
	};

} // ::Game