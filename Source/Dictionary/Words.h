//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace Reference
{

	//////////////////////////////////////////////////////////////////////

	struct Word
	{
		uint16		mLookupStart;
		uint16		mLookupCount;
		uint16		mIndex;
		uint16		mScore;
		wchar *		mDisplayText;
		wchar *		mEnglishText;

		bool operator < (Word const &r) const
		{
			return wcscmp(mEnglishText, r.mEnglishText) < 0;
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct Words
	{
		Word *	mWords;
		int					mWordCount;
		int					mCurrentWordCount;
		int					mBufferSize;
		int					mTranslatedBufferSize;
		int					mBufferUsed;
		int					mTranslatedBufferUsed;
		wchar *				mWordBuffer;
		wchar *				mTranslatedWordBuffer;

		static wchar const *sForeignChars;
		static wchar const *sEnglishChars;

		//////////////////////////////////////////////////////////////////////

		Words()
			: mWords(null)
			, mWordCount(0)
			, mCurrentWordCount(0)
			, mBufferSize(0)
			, mTranslatedBufferSize(0)
			, mBufferUsed(0)
			, mTranslatedBufferUsed(0)
			, mWordBuffer(null)
		{
		}

		//////////////////////////////////////////////////////////////////////

		Word const & operator [] (int index) const
		{
			return mWords[index];
		}

		//////////////////////////////////////////////////////////////////////

		Word & operator [] (int index)
		{
			return mWords[index];
		}

		//////////////////////////////////////////////////////////////////////

		void Init(int wordCount, int bufferSize, int translatedBufferSize)
		{
			mWordCount = wordCount;
			mWords = new Word[mWordCount];

			mWordBuffer = new wchar[bufferSize];
			mTranslatedWordBuffer = new wchar[translatedBufferSize];

			mBufferUsed = 0;
			mTranslatedBufferUsed = 0;

			mBufferSize = bufferSize;
			mTranslatedBufferSize = translatedBufferSize;
		}

		//////////////////////////////////////////////////////////////////////

		void Release()
		{
			SafeDeleteArray(mWords);
			SafeDeleteArray(mWordBuffer);
			SafeDeleteArray(mTranslatedWordBuffer);
		}

		//////////////////////////////////////////////////////////////////////

		bool Translate(wchar const *txt, wchar *translatedTxt)
		{
			bool isDifferent = false;
			for(; *txt; ++txt, ++translatedTxt)
			{
				wchar w = *txt;
				wchar const *p = wcschr(sForeignChars, w);
				if(p != null)
				{
					*translatedTxt = sEnglishChars[p - sForeignChars];
					isDifferent = true;
				}
				else
				{
					*translatedTxt = *txt;
				}
			}
			*translatedTxt = L'\0';
			return isDifferent;
		}

		//////////////////////////////////////////////////////////////////////

		Word &Add(wchar const *text)
		{
			Word &w = mWords[mCurrentWordCount];

			int l = wcslen(text);

			w.mDisplayText = mWordBuffer + mBufferUsed;
			memcpy(mWordBuffer + mBufferUsed, text, sizeof(wchar) * (l + 1));
			mBufferUsed += l + 1;

			w.mEnglishText = w.mDisplayText;

			// see if it needs a 'translated' version stored
			wchar translationBuffer[100];
			if(Translate(text, translationBuffer))
			{
				w.mEnglishText = mTranslatedWordBuffer + mTranslatedBufferUsed;
				memcpy(w.mEnglishText, translationBuffer, sizeof(wchar) * (l + 1));
				mTranslatedBufferUsed += l + 1;
			}

			++mCurrentWordCount;
			return w;
		}

		//////////////////////////////////////////////////////////////////////

		Word *Find(wchar const *candidate)
		{
			int t = mWordCount - 1;
			int b = 0;
			while (b <= t)
			{
				int m = b + (t - b) / 2;
				Word *w = &mWords[m];

				int c = wcscmp(candidate, w->mEnglishText);

				if (c > 0)
				{
					b = m + 1;
				}
				else if (c < 0)
				{
					t = m - 1;
				}
				else
				{
					return w;
				}
			}
			return null;
		}
	};

} // ::Dictionary