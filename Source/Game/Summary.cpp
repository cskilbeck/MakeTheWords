//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

Summary::Summary(SpriteList *spriteList) : Screen(spriteList)
{
	mFont = FontManager::Load("definitionBody");
	mDefinitionScreen = new DefinitionScreen(spriteList, ::Screen::Width(), ::Screen::Height());
	mDefinitionScreen->mVisible = false;
	mDefinitionScreen->mActive = false;
	mDefinitionScreen->mZIndex = 4;
	mDocument = new UI::Document(::Screen::Width(), ::Screen::Height());
	mDocument->SetLocation(Vec2(0, 0));
	mDocument->CalculateHitRect();
	mDocument->mLinkClickCallback = [this] (Link *l)
	{
		string s = AsciiStringFromWide(l->mText.c_str());
		if(gDictionary->WordIndex(s.c_str()) != -1)
		{
			mDefinitionScreen->Reset();
			mDefinitionScreen->AddDefinition(s);
			mDefinitionScreen->PushStack(s);
			mDefinitionScreen->mActive = true;
			mDefinitionScreen->mVisible = true;
		}
	};
	AddUIItem(mDocument);
	mClipEnabled = true;
	mClipRect = Rect2D(Point2D(10, 10), Size2D(200, 200));
}

//////////////////////////////////////////////////////////////////////

Summary::~Summary()
{
	SafeDelete(mDefinitionScreen);
	RemoveUIItem(mDocument);
	SafeDelete(mDocument);
	SafeRelease(mFont);
}

//////////////////////////////////////////////////////////////////////

void Summary::SetBoard(Game::Board *board)
{
	int score = board->MarkAllWords();
	mDocument->Clear();
	mDocument->AddNewLine(mFont);
	mDocument->AddNewLine(mFont);

	Game::Word *longestWord = null;
	Game::Word *highestScoringWord = null;
	int wordCount = 0;
	int totalWordLength = 0;
	for(auto &w : board->mValidWords)
	{
		if(longestWord == null || w.mLength > longestWord->mLength || (w.mLength == longestWord->mLength && w.mScore > longestWord->mScore))
		{
			longestWord = &w;
		}
		if(highestScoringWord == null || w.mScore > highestScoringWord->mScore || (w.mScore == highestScoringWord->mScore && w.mLength > highestScoringWord->mLength))
		{
			highestScoringWord = &w;
		}
		wordCount += 1;
		totalWordLength += w.mLength;
	}

	if(longestWord != null)
	{
		mDocument->Add(mFont, Format("Longest word: @%s@ (%d points)\n", board->GetWordText(longestWord).c_str(), longestWord->mScore));
		mDocument->mCursor.y -= 8;
	}

	if(highestScoringWord != null)
	{
		mDocument->Add(mFont, Format("Highest scoring word: @%s@ (%d points)\n", board->GetWordText(highestScoringWord).c_str(), highestScoringWord->mScore));
		mDocument->mCursor.y -= 8;
	}

	float averageWordLength = (float)totalWordLength / wordCount;
	float averageWordScore = (float)score / wordCount;

	mDocument->Add(mFont, Format("Average word length: %4.1f letters\n", averageWordLength).c_str());
	mDocument->mCursor.y -= 8;
	mDocument->Add(mFont, Format("Average word score: %4.1f points\n", averageWordScore).c_str());
	mDocument->mCursor.y -= 8;

	mDocument->Add(mFont, Format("Words: %d\n", board->mValidWords.size()));
	mDocument->mCursor.y -= 8;

	float y = mDocument->mCursor.y;
	for(auto &w: board->mValidWords)
	{
		string score = Format("%3d", w.mScore);
		Vec2 offset;
		Vec2 ts = mFont->MeasureString(score.c_str(), offset);
		mDocument->mCursor.x = 80 - ts.x;
		mDocument->mCursor.y = y;
		mDocument->Add(mFont, Format("  %3d", w.mScore));

		mDocument->mCursor.x = 110;
		mDocument->mCursor.y = y;
		mDocument->Add(mFont, Format("@%s@", board->GetWordText(&w).c_str()));

		y += mFont->GetHeight();
	}
	mDocument->AddNewLine(mFont);
}

//////////////////////////////////////////////////////////////////////

