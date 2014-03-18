//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

Font *		DefinitionScreen::sBodyFont = null;
Font *		DefinitionScreen::sHeaderFont = null;
Texture *	DefinitionScreen::sCloseButtonTexture = null;
Texture *	DefinitionScreen::sLeftArrowButtonTexture = null;
Texture *	DefinitionScreen::sRightArrowButtonTexture = null;

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::Open()
{
	sHeaderFont = FontManager::Load("definitionHeader");
	sBodyFont = FontManager::Load("definitionBody");
	sCloseButtonTexture = Texture::Load("icon_close.png");
	sLeftArrowButtonTexture = Texture::Load("LeftArrow.png");
	sRightArrowButtonTexture = Texture::Load("RightArrow.png");
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::Close()
{
	::Release(sLeftArrowButtonTexture);
	::Release(sRightArrowButtonTexture);
	::Release(sCloseButtonTexture);
	::Release(sHeaderFont);
	::Release(sBodyFont);
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::Reset()
{
	Clear();
	mStackPos = 0;
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::Clear()
{
	mDocument->Clear();
	mNumDefinitions = 0;
}

//////////////////////////////////////////////////////////////////////

DefinitionScreen::DefinitionScreen(SpriteList *spriteList, int width, int height)
	: Screen(spriteList)
	, mCursorPos(0,0)
	, mStackPos(0)
	, mStackSize(0)
	, mNumDefinitions(0)
{
	mBackground = new UI::Rectangle(Vec2::zero, Vec2((float)::Screen::Width(), (float)::Screen::Height()), Color(0xF0, 0, 0, 0));
	AddUIItem(mBackground);

	mDocument = new UI::Document(width, height);
	mDocument->mLinkClickCallback = [this] (Link *l)
	{
		string s = AsciiStringFromWide(l->mText.c_str());
		if(gDictionary->WordIndex(s.c_str()) != -1)
		{
			PushStack(s);
			AddDefinition(s);
		}
	};
	AddUIItem(mDocument);

	float w = (float)(::Screen::Width() - sCloseButtonTexture->Width());
	mCloseButton = new UI::ImageButton(sCloseButtonTexture, Vec2(w, 0));
	mCloseButton->OnPress = [this] ()
	{
		mVisible = false;
		mActive = false;
	};
	AddUIItem(mCloseButton);

	float lw = 52;
	mLeftButton = new UI::ImageButton(sLeftArrowButtonTexture, Vec2::zero, Vec2(48, 48));
	mRightButton = new UI::ImageButton(sRightArrowButtonTexture, Vec2(lw, 0), Vec2(48, 48));

	mLeftButton->OnPress = [this] ()
	{
		PopStack();
	};

	mRightButton->OnPress = [this] ()
	{
		UnpopStack();
	};

	AddUIItem(mLeftButton);
	AddUIItem(mRightButton);
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::DumpStack()
{
	for(uint i=0; i<mStack.size(); ++i)
	{
		TRACE("%s %2d %s\n", (i == mStackPos) ? ">" : " ", i, mStack[i].c_str());
	}
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::EnableNavigationButtons()
{
	mLeftButton->mEnabled = mStackPos > 0;
	mRightButton->mEnabled = mStackPos < mStackSize - 1;
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::PushStack(string s)
{
	// leak > 256 history entries
	if(mStackSize == mStack.size())
	{
		for(uint i=0; i<mStack.size(); ++i)
		{
			mStack[i] = mStack[i + 1];
		}
		--mStackSize;
	}
	mStackPos = mStackSize;
	mStack[mStackSize++] = s;
	EnableNavigationButtons();
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::PopStack()
{
	if(mStackPos > 0)
	{
		AddDefinition(mStack[--mStackPos].c_str());
	}
	EnableNavigationButtons();
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::UnpopStack()
{
	if(mStackPos < mStackSize - 1)
	{
		AddDefinition(mStack[++mStackPos].c_str());
	}
	EnableNavigationButtons();
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::AddWord(string s)
{
	int score = Reference::Letter::GetWordScore(s.c_str());
	int index = gDictionary->WordIndex(s.c_str());
	string def = gDictionary->GetDefinition(index);
	def = sBodyFont->WrapText(def, mDocument->mSize.w - 8, "\n   ");

	string header = Format("#ff00ffff#%s (%d)", s.c_str(), score);
	mDocument->mCursor.x = 4;
	mDocument->Add(sHeaderFont, header);
	mDocument->mCursor.y += sBodyFont->GetHeight() + 8;
	mDocument->Add(sBodyFont, def);
	++mNumDefinitions;
}

//////////////////////////////////////////////////////////////////////

void DefinitionScreen::AddDefinition(string word)
{
	Clear();
	if(mDocument->mCursor.y == 0)
	{
		mDocument->mCursor.y = 50;
	}

	auto i = word.find(',');
	if(i != -1)
	{
		string a = word.substr(0, i);
		string b = word.substr(i + 1);
		AddWord(a);
		AddWord(b);
	}
	else
	{
		AddWord(word);
	}
}

//////////////////////////////////////////////////////////////////////

Component::eComponentReturnCode DefinitionScreen::Update()
{
	Screen::Update();
	return kAllowNoInput;		// don't allow components below this one to see any input
}

//////////////////////////////////////////////////////////////////////

DefinitionScreen::~DefinitionScreen()
{
	RemoveUIItem(mCloseButton);
	RemoveUIItem(mLeftButton);
	RemoveUIItem(mRightButton);
	RemoveUIItem(mDocument);
	Delete(mCloseButton);
	Delete(mLeftButton);
	Delete(mRightButton);
	Delete(mDocument);
}
