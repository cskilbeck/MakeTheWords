//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

list<Component *> Component::sComponents;

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

void Component::Reorder()
{
	sComponents.sort([&] (Component *&a, Component *&b)
	{
		return a->mZIndex < b->mZIndex;
	});
}

//////////////////////////////////////////////////////////////////////

void Component::UpdateAll()
{
	TouchInput::Pause(false);
	for(auto c = sComponents.rbegin(); c != sComponents.rend(); ++c)
	{
		if((*c)->mActive)
		{
			eComponentReturnCode rc = (*c)->Update();
			if(rc == kBlock)
			{
				break;
			}
			else if(rc == kAllowNoInput)
			{
				// tell the touch system to report nothing until further notice
				TouchInput::Pause(true);
			}
		}
	}
	Reorder();
}

//////////////////////////////////////////////////////////////////////

void Component::DrawAll()
{
	for(auto i = sComponents.begin(); i != sComponents.end(); ++i)
	{
		Component *c = *i;
		if(c->mVisible)
		{
			(*i)->Draw();
		}
	}
}

