#pragma once

namespace Keyboard
{
	extern uint8 Pressed[256];
	extern uint8 Held[256];
	extern uint8 Released[256];

	void Update();
}
