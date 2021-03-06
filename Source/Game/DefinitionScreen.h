//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct DefinitionScreen : UI::Screen
{
	//////////////////////////////////////////////////////////////////////

	UI::ImageButton *	mCloseButton;
	UI::ImageButton *	mLeftButton;
	UI::ImageButton *	mRightButton;
	UI::Rectangle *		mBackground;
	UI::Document *		mDocument;
	Vec2				mCursorPos;
	array<string, 256>	mStack;
	int					mStackSize;
	int					mStackPos;
	int					mNumDefinitions;

	static Font *		sHeaderFont;
	static Font *		sBodyFont;
	static Texture *	sCloseButtonTexture;
	static Texture *	sLeftArrowButtonTexture;
	static Texture *	sRightArrowButtonTexture;

	//////////////////////////////////////////////////////////////////////

	static void Open();
	static void Close();

	//////////////////////////////////////////////////////////////////////

	DefinitionScreen(SpriteList *spriteList, int width, int height);
	~DefinitionScreen();

	//////////////////////////////////////////////////////////////////////

	eComponentReturnCode Update() override;

	//////////////////////////////////////////////////////////////////////

	void PushStack(string s);
	void PopStack();
	void UnpopStack();
	void DumpStack();

	//////////////////////////////////////////////////////////////////////

	void Clear();
	void Reset();
	void AddDefinition(string word);

	void EnableNavigationButtons();
	void AddWord(string word);

	//////////////////////////////////////////////////////////////////////

};
