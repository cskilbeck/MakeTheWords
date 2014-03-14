//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Win32.h"
#include "Facebook.h"

//////////////////////////////////////////////////////////////////////

static Timer sTimer;
static double sOldTime;
bool gDebuggerAttached;

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	gDebuggerAttached = IsDebuggerPresent() == TRUE;

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF);
	//_CrtSetBreakAlloc(6612);
	assert(g_App != null);

	sTimer.Reset();

	CD(L"..\\..\\data");
	Screen::Init(480, 320);

	g_DebugSpriteList = SpriteList::Create();
	g_DebugFont = FontManager::Load("debug");

	g_App->Init();

	g_Time = sTimer.GetElapsed();
	sOldTime = g_Time;

	Facebook::Login(false);

	while(Screen::Update())
	{
		g_Time = sTimer.GetElapsed();
		g_DeltaTime = g_Time - sOldTime;
		sOldTime = g_Time;

		g_App->Update();
		g_App->Draw();

		SpriteList::SubmitAll();
		Screen::Present();
	}

	URLLoader::TerminateAll();
	Facebook::Logout();

	g_App->Release();

	SafeRelease(g_DebugSpriteList);
	SafeRelease(g_DebugFont);

	return 0;
}
