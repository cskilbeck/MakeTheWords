//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#define DBG_NEW new ( _NORMAL_BLOCK, __FILE__ , __LINE__ )
#define new DBG_NEW
#define new(x) new(x)	// std::nothrow and placement new not tracked
#endif

//////////////////////////////////////////////////////////////////////

#if defined(WIN32)

	#pragma warning(disable:4996)	// deprecated function use
	#pragma warning(disable:4250)	// inheritance via dominance

	#include <winsock2.h>
	#include <WS2tcpip.h>
	#include <Windows.h>

	#if !defined(OPENGL)
		#include <d3d11.h>
		#include <d3dx11.h>
		#include <d3dcompiler.h>
		#include <d3dcommon.h>
		#include <xnamath.h>
	#endif

	#include <atlbase.h>
	#include <crtdbg.h>

#endif

//////////////////////////////////////////////////////////////////////

#if defined(IOS)

	#include "mach/mach_time.h"
	#include "CoreFoundation/CoreFoundation.h"

	#ifdef DEBUG
		#define DLog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
	#else
		#define DLog(...)
	#endif

	// ALog always displays output regardless of the DEBUG setting
	#define ALog(fmt, ...) NSLog((@"%s [Line %d] " fmt), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__);
#endif

//////////////////////////////////////////////////////////////////////

#include <memory.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include <list>
#include <unordered_map>
#include <map>
#include <functional>
#include <algorithm>

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

//////////////////////////////////////////////////////////////////////

#include "rapidxml.hpp"

//////////////////////////////////////////////////////////////////////

#include "Box2D/Box2D.h"

//////////////////////////////////////////////////////////////////////

#include "Types.h"
#include "Util.h"
#include "Value.h"
#include "Vec2.h"
#include "Point.h"
#include "Size.h"
#include "Rect.h"
//#include "LinkedList.h"
#include "linked_list.h"
using chs::linked_list;
using chs::list_node;
#include "Pool.h"
#include "Refcount.h"
#include "UTF8.h"
#include "xml_util.h"
#include "Color.h"
#include "Timer.h"
#include "CodeTimer.h"
#include "Random.h"
#include "Touch.h"
#include "Texture.h"
#include "SpriteList.h"
#include "Screen.h"
#include "Touch.h"
#include "Font.h"
#include "App.h"
#include "Component.h"
#include "Keyboard.h"
#include "Particles.h"

#include "URLLoader.h"

//////////////////////////////////////////////////////////////////////

//#include "Position.h"
//#include "FlagMap.h"
//#include "Constants.h"
//#include "Particle.h"
//#include "Grid.h"
//#include "Neighbour.h"
//#include "Spring.h"
//#include "World.h"

//////////////////////////////////////////////////////////////////////

#include "Letter.h"
#include "Dictionary.h"

//////////////////////////////////////////////////////////////////////

#include "Index.h"
#include "Tile.h"
#include "Word.h"
#include "Board.h"

//////////////////////////////////////////////////////////////////////

#include "UIItem.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIImageButton.h"
#include "UITextLabel.h"
#include "UIScreen.h"
#include "UIDocument.h"
#include "UIRectangle.h"
#include "UITextButton.h"

//////////////////////////////////////////////////////////////////////

#include "DefinitionScreen.h"
#include "InGameUI.h"
#include "Leaderboard.h"
#include "Summary.h"
#include "HeaderBar.h"
#include "ResultsUI.h"
#include "Game.h"
#include "MainUI.h"
#include "MakeTheWords.h"

#undef PI_2
#define PI_2     1.57079632679489661923f

extern Font *g_DebugFont;
extern SpriteList *g_DebugSpriteList;
extern bool gDebuggerAttached;
