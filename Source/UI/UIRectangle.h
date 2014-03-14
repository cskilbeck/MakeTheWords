// Background color
// Border color
// Border thickness
// Corner radius

//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	struct Rectangle : Item
	{
		Color	mBorderColor;
		int		mBorderThickness;
		int		mBorderRadius;

		Rectangle(Vec2 topleft, Vec2 bottomRight, Color backgroundColor, Color borderColor = Color::Transparent, int borderWidth = 0, int cornerRadius = 0);
		~Rectangle();

		void Draw(SpriteList *spriteList);
	};
}
