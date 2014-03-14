//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

struct CodeTimer
{
	Timer t;
	char *txt;

	CodeTimer(char *txt) : txt(txt)
	{
		t.Reset();
	}

	~CodeTimer()
	{
		TRACE("%s:%fs\n", txt, t.GetElapsed());
	}
};