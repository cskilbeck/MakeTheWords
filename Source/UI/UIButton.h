//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	struct Button : virtual Item
	{
		//////////////////////////////////////////////////////////////////////

		Button()
			: OnPress(null)
			, OnPush(null)
			, OnRelease(null)
			, mEnabled(true)
		{
			mPressed = false;
		}

		//////////////////////////////////////////////////////////////////////

		void Call(std::function<void ()> func)
		{
			if(func != null)
			{
				func();
			}
		}

		//////////////////////////////////////////////////////////////////////

		bool Update() override
		{
			bool rc = true;
			if(mEnabled)
			{
				Point2D p(Touch(0).mPosition);
				if(!mPressed)
				{
					if(Contains(p) && Touch(0).Pressed())
					{
						mPressed = true;
						mScale = Vec2(0.85f, 0.85f);
						Call(OnPush);
						rc = false;
					}
				}
				else if(!Contains(p))
				{
					mPressed = false;
					mScale = Vec2::one;
					Call(OnRelease);
				}
				else if(Touch(0).Released())
				{
					mScale = Vec2::one;
					mPressed = false;
					Call(OnRelease);
					Call(OnPress);
				}
				mColor |= 0xff000000;
			}
			else
			{
				mColor = (mColor & 0x00ffffff) | 0x60000000;
			}
			return rc;
		}

		//////////////////////////////////////////////////////////////////////

		bool mPressed;
		bool mEnabled;

		std::function<void ()> OnPush;
		std::function<void ()> OnRelease;
		std::function<void ()> OnPress;
	};

}