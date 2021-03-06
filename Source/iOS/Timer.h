//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Timer
{
	//////////////////////////////////////////////////////////////////////
	
	Timer()
	{
		Reset();
	}
	
	//////////////////////////////////////////////////////////////////////
	
	uint64 GetCounter()
	{
		return mach_absolute_time();
	}
	
	//////////////////////////////////////////////////////////////////////
	
	void Reset()
	{
		mOldTime = mStartTime = GetCounter();
	}
	
	//////////////////////////////////////////////////////////////////////
	
	mach_timebase_info_data_t &TimebaseInfo()
	{
		static mach_timebase_info_data_t sTimebaseInfo;
		
		if(sTimebaseInfo.denom == 0)
		{
			(void)mach_timebase_info(&sTimebaseInfo);
		}
		return sTimebaseInfo;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	uint64 AbsoluteToNanoseconds(uint64 ticks)
	{
		mach_timebase_info_data_t tbi = TimebaseInfo();
		return ticks * tbi.numer / tbi.denom;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	double GetElapsed()
	{
		return AbsoluteToNanoseconds(GetCounter() - mStartTime) / 1000000000.0;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	double GetDelta()
	{
		uint64 t = GetCounter();
		double delta = AbsoluteToNanoseconds(t - mOldTime) / 1000000000.0;
		mOldTime = t;
		return delta;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	uint64 mStartTime;
	uint64 mOldTime;
};

//////////////////////////////////////////////////////////////////////

extern double g_Time;
extern double g_DeltaTime;
