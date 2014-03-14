//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

// Summary
// Score
// # of words
// average word score
// average word length
// highest scoring word
// longest word
// all the words
// #'rarest' word? obscurity.... (requires data connection for now...)
//

struct Summary : UI::Screen
{
	//////////////////////////////////////////////////////////////////////
	
	Font *				mFont;
	UI::Document *		mDocument;
	DefinitionScreen *	mDefinitionScreen;

	Summary(SpriteList *spriteList);
	~Summary();
	void SetBoard(Game::Board *board);

};
