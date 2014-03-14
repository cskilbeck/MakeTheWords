//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Reference
{

	//////////////////////////////////////////////////////////////////////

	struct StringTable
	{
		wchar *		mBuffer;
		wchar **	mStrings;

		wchar const *operator [] (int index) const
		{
			return mStrings[index];
		}

		void Release()
		{
			SafeDeleteArray(mBuffer);
			SafeDeleteArray(mStrings);
		}

		void Read(rapidxml::xml_node<wchar> *root, wchar const *rootName, wchar const *nodeName);
	};

} // ::Dictionary