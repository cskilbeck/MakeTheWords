//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

linked_list<Component, &Component::mListNode> gComponents;

//////////////////////////////////////////////////////////////////////

Component::Component(SpriteList *spriteList)
	: RefCount()
	, mActive(true)
	, mVisible(true)
	, mSpriteList(spriteList)
	, mZIndex(0)
	, mClipEnabled(false)
{
	spriteList->AddRef();
	gComponents.push_back(this);
}

//////////////////////////////////////////////////////////////////////

Component::~Component()
{
	SafeRelease(mSpriteList);
	gComponents.remove(this);
}

//////////////////////////////////////////////////////////////////////

void Component::UpdateAll()
{
	TouchInput::Pause(false);
	for(auto &c : chs::reverse(gComponents))
	{
		if(c.mActive)
		{
			eComponentReturnCode rc = c.Update();
			if(rc == kBlock)
			{
				break;
			}
			else if(rc == kAllowNoInput)
			{
				// tell the touch system to report nothing until further notice (it's modal)
				TouchInput::Pause(true);
			}
		}
	}
	gComponents.sort();
}

//////////////////////////////////////////////////////////////////////

void Component::DrawAll()
{
	for(auto &c : gComponents)
	{
		if(c.mVisible)
		{
			c.Draw();
		}
	}
}

