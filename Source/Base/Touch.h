//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct TouchPoint
{
	bool mState;
	bool mPreviousState;
	Point2D mPosition;
	Point2D mPreviousPosition;

	TouchPoint() : mPosition(-1,-1), mPreviousPosition(-1,-1), mState(false), mPreviousState(false)
	{
	}

	bool Held()
	{
		return mState == true;
	}

	bool Pressed()
	{
		return mState == true && mPreviousState == false;
	}

	bool Released()
	{
		return mState == false && mPreviousState == true;
	}
};

//////////////////////////////////////////////////////////////////////

struct TouchInput
{
	static const int kMaxTouch = 10;
	static int sNumTouch;
	static void Pause(bool pause);
	static void Update();

	static bool sPaused;

private:

	static TouchPoint sTouch[kMaxTouch];
	static TouchPoint sPausedTouch;

	friend inline TouchPoint &Touch(int index);
};

//////////////////////////////////////////////////////////////////////

inline TouchPoint &Touch(int index)
{
	return TouchInput::sPaused ? TouchInput::sPausedTouch : TouchInput::sTouch[index];
}

