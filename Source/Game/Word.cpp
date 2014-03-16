//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace Game
{
	Word::pool_t Word::pool;

	Word::Word(int x, int y, Word::Orientation orientation, int score, int length, int dictionaryWord)
		: mLocation(x, y)
		, mOrientation(orientation)
		, mScore(score)
		, mLength(length)
		, mDictionaryWord(dictionaryWord)
	{
	}
}