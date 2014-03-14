//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	struct Index
	{
		union
		{
			struct
			{
				uint8	x,y;
			};
			uint16 index;
		};

		Index()
		{
		}

		Index(int i, int w) : x(i % w), y(i / w)
		{
		}

		int Offset(int boardWidth)
		{
			return x + y * boardWidth;
		}

		Index &Flip()
		{
			uint8 t = x;
			x = y;
			y = t;
			return *this;
		}

		bool operator != (Index const &o)
		{
			return index != o.index;
		}
	};

} // ::Game