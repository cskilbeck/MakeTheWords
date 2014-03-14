//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Game
{
	struct InGameUI : UI::Screen
	{
		Texture *			mCloseTexture;
		Texture *			mUndoTexture;
		UI::ImageButton *	mUndoButton;
		UI::ImageButton *	mQuitButton;

		InGameUI(SpriteList *spriteList)
			: Screen(spriteList)
		{
			mCloseTexture = Texture::Load("icon_close.png");
			mUndoTexture = Texture::Load("undo.png");
			mUndoButton = new UI::ImageButton(mUndoTexture, Vec2((float)(::Screen::Width() - 50), 0), Vec2(50, 50));
			mQuitButton = new UI::ImageButton(mCloseTexture, Vec2((float)(::Screen::Width() - 50), (float)(::Screen::Height() - 50)), Vec2(50, 50));
			AddUIItem(mQuitButton);
			AddUIItem(mUndoButton);
		}

		~InGameUI()
		{
			RemoveUIItem(mUndoButton);
			RemoveUIItem(mQuitButton);
			SafeDelete(mUndoButton);
			SafeDelete(mQuitButton);
			SafeRelease(mUndoTexture);
			SafeRelease(mCloseTexture);
		}
	};

} // ::Game