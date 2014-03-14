//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace Reference
{

	//////////////////////////////////////////////////////////////////////

	struct Dictionary;

	//////////////////////////////////////////////////////////////////////

	struct Definition
	{
		uint16		mWordIndex;
		int16		mWordClass;
		uint16		mIndex;
		wchar *		mText;

		void Set(int index, int wordIndex, int wordClass, wchar *text)
		{
			mIndex = (uint16)index;
			mWordIndex = (uint16)wordIndex;
			mWordClass = (int16)wordClass;
			mText = text;
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct Definitions
	{
		wchar *			mBuffer;
		wchar **		mDefinitionText;
		int				mCount;
		int				mCurrentCount;
		int				mBufferSize;
		wchar *			mBufferPtr;
		Definition *	mDefinitions;
		Dictionary *	mDictionary;

		Definitions(Dictionary *dictionary)
			: mDictionary(dictionary)
			, mBufferPtr(null)
			, mBuffer(null)
			, mCount(0)
			, mCurrentCount(0)
		{
		}

		void Init(int count, int bufferSize)
		{
			mBufferSize = bufferSize;
			mBufferPtr = mBuffer = new wchar[bufferSize];
			mDefinitionText = new wchar * [count];
			mCount = count;
			mCurrentCount = 0;
			mDefinitions = new Definition[mCount];
		}

		void Release()
		{
			SafeDeleteArray(mDefinitions);
			SafeDeleteArray(mBuffer);
			SafeDeleteArray(mDefinitionText);
		}

		Definition const &operator [] (int index) const
		{
			return mDefinitions[index];
		}

		Definition &operator [] (int index)
		{
			return mDefinitions[index];
		}

		void Add(int index, int wordIndex, int wordClass, wchar *text, int textLen)
		{
			assert(mBufferPtr + textLen + 1 < mBuffer + mBufferSize);
			mDefinitionText[mCurrentCount] = mBufferPtr;
			memcpy(mBufferPtr, text, sizeof(wchar) * textLen);	
			mBufferPtr[textLen] = L'\0';
			mBufferPtr += textLen + 1;
			mDefinitions[mCurrentCount].Set(index, wordIndex, wordClass, mDefinitionText[mCurrentCount]);
			++mCurrentCount;
		}

		int SpareSpace()
		{
			return mBuffer + mBufferSize - mBufferPtr;
		}
	};

} // ::Dictionary