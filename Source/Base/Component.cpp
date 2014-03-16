//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

static linked_list<Component, &Component::mListNode> sComponents;

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
	sComponents.push_back(this);
}

//////////////////////////////////////////////////////////////////////

Component::~Component()
{
	SafeRelease(mSpriteList);
	sComponents.remove(this);
}

//////////////////////////////////////////////////////////////////////

void Component::UpdateAll()
{
	TouchInput::Pause(false);
	for(auto &c : chs::reverse(sComponents))
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
	sComponents.sort();
}

//////////////////////////////////////////////////////////////////////

void Component::DrawAll()
{
	for(auto &c : sComponents)
	{
		if(c.mVisible)
		{
			c.Draw();
		}
	}
}

