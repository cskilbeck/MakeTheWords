//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Win32.h"
#include "Facebook.h"
#include "D3D.h"

//////////////////////////////////////////////////////////////////////

static Timer sTimer;
static double sOldTime;
bool gDebuggerAttached;

//////////////////////////////////////////////////////////////////////

void ShowCurrentFolder()
{
	WCHAR buffer[16384];
	buffer[0] = 0;
	GetCurrentDirectory(ARRAYSIZE(buffer), buffer);
	OutputDebugString(L"Current folder:");
	OutputDebugString(buffer);
	OutputDebugString(L"\n");
}

//////////////////////////////////////////////////////////////////////

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	gDebuggerAttached = IsDebuggerPresent() == TRUE;

    _CrtSetDbgFlag(
					_CRTDBG_ALLOC_MEM_DF |
//					_CRTDBG_CHECK_ALWAYS_DF |
					_CRTDBG_CHECK_CRT_DF |
					_CRTDBG_DELAY_FREE_MEM_DF |
					_CRTDBG_LEAK_CHECK_DF
					);

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_CRT_DF);
	
	//_CrtSetBreakAlloc(2675);
	assert(g_App != null);

	sTimer.Reset();

	CD(L"..\\data");

	ShowCurrentFolder();

	Screen::Init(480, 320);

	g_DebugSpriteList = SpriteList::Create();
	g_DebugFont = FontManager::Load("debug");

	g_App->Init();

	g_Time = sTimer.GetElapsed();
	sOldTime = g_Time;

	//Facebook::Login(false);

	while(Screen::Update())
	{
		g_Time = sTimer.GetElapsed();
		g_DeltaTime = g_Time - sOldTime;
		if(g_DeltaTime > 0.1)
		{
			g_DeltaTime = 0.1;
		}
		sOldTime = g_Time;

		g_App->Update();
		g_App->Draw();

		SpriteList::SubmitAll();
		Screen::Present();
	}

	//URLLoader::TerminateAll();
	//Facebook::Logout();

	Release(g_DebugSpriteList);
	Release(g_DebugFont);

	g_App->Release();

	return 0;
}
