//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

TouchPoint TouchInput::sTouch[TouchInput::kMaxTouch];
TouchPoint TouchInput::sPausedTouch;
int TouchInput::sNumTouch = 0;
bool TouchInput::sPaused = false;

//////////////////////////////////////////////////////////////////////

void TouchInput::Update()
{
	for(int i=0; i<kMaxTouch; ++i)
	{
		sTouch[i].mPreviousPosition = sTouch[i].mPosition;
		sTouch[i].mPreviousState = sTouch[i].mState;
	}
}

//////////////////////////////////////////////////////////////////////

void TouchInput::Pause(bool pause)
{
	sPaused = pause;
}
