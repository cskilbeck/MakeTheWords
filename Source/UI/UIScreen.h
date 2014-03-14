//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	struct Screen : Component
	{
		//////////////////////////////////////////////////////////////////////

		Screen(SpriteList *spriteList)
			: Component(spriteList)
		{
		}

		//////////////////////////////////////////////////////////////////////

		virtual ~Screen()
		{
			while(!mItems.empty())
			{
				Item *i = mItems.head();
				RemoveUIItem(i);
				delete i;
			}
		}

		//////////////////////////////////////////////////////////////////////

		void AddUIItem(Item *i)
		{
			mItems.push_back(i);
			i->mParent = this;
		}

		//////////////////////////////////////////////////////////////////////

		void MoveToFront(Item *i)
		{
			mItems.remove(i);
			mItems.push_front(i);
		}

		//////////////////////////////////////////////////////////////////////

		void MoveToBack(Item *i)
		{
			mItems.remove(i);
			mItems.push_back(i);
		}

		//////////////////////////////////////////////////////////////////////

		void RemoveUIItem(Item *i)
		{
			i->mParent = null;
			mItems.remove(i);
		}

		//////////////////////////////////////////////////////////////////////

		virtual eComponentReturnCode Update()
		{
			for(auto &i : chs::reverse(mItems))
			{
				if(i.Update() == false)
				{
					break;
				}
			}
			return kAllow;
		}

		//////////////////////////////////////////////////////////////////////

		virtual void Draw()
		{
			for(auto &i : mItems)
			{
				i.Draw(mSpriteList);
				//mSpriteList->ResetTransform();
				//DrawRectangle(*mSpriteList, (Vec2)(i->mHitRect.mTopLeft), (Vec2)(i->mHitRect.mSize), Color::Magenta);
			}
		}

		linked_list<Item, &Item::mListNode> mItems;
	};

	//////////////////////////////////////////////////////////////////////

} //::UI
