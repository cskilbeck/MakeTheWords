//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct HeaderBar : UI::Screen
{
	//////////////////////////////////////////////////////////////////////

	float									mWidth;
	Font *									mHeaderFont;
	map<UI::TextButton *, UI::Screen *>		mScreens;
	string									mCurrentScreen;

	//////////////////////////////////////////////////////////////////////

	HeaderBar(SpriteList *spriteList, Font *headerFont)
		: Screen(spriteList)
		, mHeaderFont(headerFont)
		, mWidth(12)
	{
	}

	//////////////////////////////////////////////////////////////////////

	~HeaderBar()
	{
		while(!mScreens.empty())
		{
			UI::TextButton *t = mScreens.begin()->first;
			mScreens.erase(t);
			RemoveUIItem(t);
			Delete(t);
		}
	}

	//////////////////////////////////////////////////////////////////////

	void AddButton(char const *text, UI::Screen *screen)
	{
		UI::TextButton *button = new UI::TextButton(mHeaderFont, text, Vec2(mWidth, 8), Color(48, 48, 48));
		AddUIItem(button);
		mWidth += button->GetSize().x + 26;
		string t = text;
		mScreens[button] = screen;
		button->OnPress = [=]
		{
			Activate(t.c_str());
		};
	}

	//////////////////////////////////////////////////////////////////////

	void Activate(char const *buttonText)
	{
		if(mCurrentScreen != buttonText)
		{
			mCurrentScreen.clear();
			for(auto p = mScreens.begin(); p != mScreens.end(); ++p)
			{
				bool me = p->first->mText == buttonText;
				p->second->mVisible = me;
				p->second->mActive = me;
				p->first->mEnabled = !me;
				if(me)
				{
					mCurrentScreen = buttonText;
					p->first->mColor = Color(255, 255, 255);
				}
				else
				{
					p->first->mColor = Color(64, 64, 64);
				}
			}
		}
	}
};
