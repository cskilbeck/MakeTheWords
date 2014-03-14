#include "pch.h"

namespace Keyboard
{
	uint8 Pressed[256];
	uint8 Held[256];
	uint8 Released[256];

	void Update()
	{
		uint8 old[256];

		memcpy(old, Held, 256);

		GetKeyboardState(Held);

		for(int i=0; i<256; ++i)
		{
			Held[i] >>= 7;
			Pressed[i] = (old[i] != Held[i]) && Held[i];
			Released[i] = (old[i] != Held[i]) && !Held[i];
		}
	}
}