//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Graphics.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	struct ScreenImpl
	{
		static ScreenImpl *g_ScreenImpl;

		//////////////////////////////////////////////////////////////////////

		ScreenImpl()
			: mHWND(null)
			, mHINST(null)
			, OnResize(null)
			, mInResizingLoop(false)
			, mMouseDown(false)
		{
			g_ScreenImpl = this;
		}

		//////////////////////////////////////////////////////////////////////

		static bool IsScreenImpl(ScreenImpl *i)
		{
			return i == g_ScreenImpl;
		}

		//////////////////////////////////////////////////////////////////////

		int						mWidth;
		int						mHeight;
		HWND					mHWND;
		HINSTANCE				mHINST;
		bool					mInResizingLoop;
		bool					mMouseDown;

		std::function<void ()>	OnResize;

		//////////////////////////////////////////////////////////////////////

		void EnableScissoring(bool enable)
		{
			gGraphics.EnableScissoring(enable);
		}

		//////////////////////////////////////////////////////////////////////

		void SetScissorRectangle(Rect2D const &rect)
		{
			gGraphics.SetScissorRectangle(rect);
		}

		//////////////////////////////////////////////////////////////////////

		void ClearBackBuffer(Color color)
		{
			gGraphics.ClearBackBuffer(color);
		}

		//////////////////////////////////////////////////////////////////////

		void Present()
		{
			gGraphics.Present();
		}

		//////////////////////////////////////////////////////////////////////

		void CentreRectInMonitorWithMouseInIt(RECT &rc)
		{
			POINT ptCursorPos;
			GetCursorPos(&ptCursorPos);
			HMONITOR hMonitor = MonitorFromPoint(ptCursorPos, MONITOR_DEFAULTTOPRIMARY);
			if(hMonitor != INVALID_HANDLE_VALUE)
			{
				MONITORINFO monitorInfo = {0};
				monitorInfo.cbSize = sizeof(monitorInfo);
				if(GetMonitorInfo(hMonitor, &monitorInfo))
				{
					int iMidX = (monitorInfo.rcWork.left + monitorInfo.rcWork.right) / 2;
					int iMidY = (monitorInfo.rcWork.top + monitorInfo.rcWork.bottom) / 2;
					int iRectWidth = rc.right - rc.left;
					int iRectHeight = rc.bottom - rc.top;
					rc.left = iMidX - iRectWidth / 2;
					rc.top = iMidY - iRectHeight / 2;
					rc.right = rc.left + iRectWidth;
					rc.bottom = rc.top + iRectHeight;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		void InitWindow(int width, int height)
		{
			mHINST = GetModuleHandle(null);

			WNDCLASSEX wcex;
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = WndProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = sizeof(ScreenImpl *);
			wcex.hInstance = 0;
			wcex.hIcon = null;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = null;
			wcex.lpszClassName = L"WindowClass";
			wcex.hIconSm = null;
			if(!RegisterClassEx(&wcex))
			{
				return;
			}

			mWidth = width;
			mHeight = height;

			RECT rc = { 0, 0, width, height };

			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

			CentreRectInMonitorWithMouseInIt(rc);

			mHWND = CreateWindowEx(	0,
									L"WindowClass",
									L"Screen",
									WS_OVERLAPPEDWINDOW,
									rc.left,
									rc.top,
									rc.right - rc.left, rc.bottom - rc.top,
									NULL,
									NULL,
									mHINST,
									this);
			if(!mHWND)
			{
				TRACE("Window Create Failed: %08x\n", GetLastError());
				return;
			}

			SetWindowLongPtr(mHWND, GWLP_USERDATA, (LONG_PTR)this);
			ShowWindow(mHWND, SW_SHOW);

			GetClientRect(mHWND, &rc);
		}
	
		//////////////////////////////////////////////////////////////////////

		void ChangeSize(int newWidth, int newHeight)
		{
			mWidth = newWidth;
			mHeight = newHeight;
			RECT rc = { 0, 0, mWidth, mHeight };
			AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
			SetWindowPos(mHWND, null, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE|SWP_NOZORDER);
		}

		//////////////////////////////////////////////////////////////////////

		bool Update()
		{
			MSG msg;
			TouchInput::Pause(false);
			TouchInput::Update();
			Keyboard::Update();

			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return msg.message != WM_QUIT;
		}

		//////////////////////////////////////////////////////////////////////

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch(message)
			{
			case WM_NCCREATE:
				SetWindowLongPtr(hWnd, 0, (LONG_PTR)(((CREATESTRUCT *)lParam)->lpCreateParams));

			case WM_GETMINMAXINFO:
				return DefWindowProc(hWnd, message, wParam, lParam);

			default:
				{
					ScreenImpl *ths = (ScreenImpl *)GetWindowLongPtr(hWnd, 0);
					if(ScreenImpl::IsScreenImpl(ths))
					{
						return ths->HandleMessage(hWnd, message, wParam, lParam);
					}
					else
					{
						return DefWindowProc(hWnd, message, wParam, lParam);
					}
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			PAINTSTRUCT ps;
			HDC hdc;

			switch(message)
			{
				case WM_PAINT:
					hdc = BeginPaint(hWnd, &ps);
					EndPaint(hWnd, &ps);
					break;

				case WM_DESTROY:
					PostQuitMessage(0);
					break;

				case WM_SIZE:
					switch(wParam)
					{
					case SIZE_MINIMIZED:	// pause it
						break;

					case SIZE_MAXIMIZED:
					case SIZE_RESTORED:
						if(!mInResizingLoop)
						{
							DoResize();
						}
						break;
					default:
						break;
					}
					break;

				case WM_ENTERSIZEMOVE:
					mInResizingLoop = true;
					break;

				case WM_EXITSIZEMOVE:
					mInResizingLoop = false;
					DoResize();
					break;

				case WM_MOUSEMOVE:
					if(mMouseDown)
					{
						POINTS p = MAKEPOINTS(lParam);
						Touch(0).mPosition = Point2D(p.x, p.y);
					}
					break;

				case WM_LBUTTONDOWN:
					{
						POINTS p = MAKEPOINTS(lParam);
						Touch(0).mState = true;
						Touch(0).mPosition = Point2D(p.x, p.y);
						if(!mMouseDown)
						{
							Touch(0).mPreviousPosition = Touch(0).mPosition;
						}
						mMouseDown = true;
						SetCapture(hWnd);
						TouchInput::sNumTouch = 1;
					}
					break;

				case WM_LBUTTONUP:
					mMouseDown = false;
					TouchInput::sNumTouch = 0;
					Touch(0).mState = false;
					ReleaseCapture();
					break;

				case WM_RBUTTONDOWN:
					break;

				case WM_RBUTTONUP:
					break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}

			return 0;
		}

		//////////////////////////////////////////////////////////////////////

		void DoResize()
		{
			RECT rc;
			GetClientRect(mHWND, &rc);
			if(rc.right != mWidth || rc.bottom != mHeight)
			{
				mWidth = rc.right;
				mHeight = rc.bottom;

				gGraphics.Resize(mWidth, mHeight);

				if(OnResize != null)
				{
					OnResize();
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		void Init(int width, int height)
		{
			InitWindow(width, height);
			gGraphics.Init(mHWND);
		}

		//////////////////////////////////////////////////////////////////////

		~ScreenImpl()
		{
			Release();
			g_ScreenImpl = null;
		}

		//////////////////////////////////////////////////////////////////////

		void Release()
		{
			gGraphics.Release();
		}
	};

	ScreenImpl *ScreenImpl::g_ScreenImpl = null;
	ScreenImpl g_Screen;
}

//////////////////////////////////////////////////////////////////////

namespace Screen
{
	std::function<void ()> OnResize;

	void Release()
	{
		g_Screen.Release();
	}

	//////////////////////////////////////////////////////////////////////

	int Width()
	{
		return g_Screen.mWidth;
	}

	//////////////////////////////////////////////////////////////////////

	int Height()
	{
		return g_Screen.mHeight;
	}

	//////////////////////////////////////////////////////////////////////

	void EnableScissoring(bool enable)
	{
		g_Screen.EnableScissoring(enable);
	}

	//////////////////////////////////////////////////////////////////////

	void SetScissorRectangle(Rect2D const &rect)
	{
		g_Screen.SetScissorRectangle(rect);
	}

	//////////////////////////////////////////////////////////////////////

	Size2D GetSize()
	{
		return Size2D(Width(), Height());
	}

	//////////////////////////////////////////////////////////////////////

	void Init(int width, int height)
	{
		g_Screen.Init(width, height);
//		gGraphics.Init(g_Screen.mHWND);
		g_Screen.OnResize = []
		{
			if(OnResize != null)
			{
				OnResize();
			}
		};
	}

	//////////////////////////////////////////////////////////////////////

	void ChangeSize(int newWidth, int newHeight)
	{
		g_Screen.ChangeSize(newWidth, newHeight);
	}

	//////////////////////////////////////////////////////////////////////

	bool Update()
	{
		return g_Screen.Update();
	}

	//////////////////////////////////////////////////////////////////////

	void Clear(Color color)
	{
		g_Screen.ClearBackBuffer(color);
	}

	//////////////////////////////////////////////////////////////////////

	void Present()
	{
		g_Screen.Present();
	}
}
