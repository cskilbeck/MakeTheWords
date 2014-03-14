//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace Reference
{

	//////////////////////////////////////////////////////////////////////

	void StringTable::Read(node *root, wchar const *rootName, wchar const *nodeName)
	{
		node *n = root->first_node(rootName);
		if(n != null)
		{
			int count = GetInt(n, L"Count");
			if(count > 0)
			{
				int size = GetInt(n, L"BufferSize");

				if(size > 0)
				{
					mBuffer = new wchar[size];
					mStrings = new wchar *[count];

					wchar *p = mBuffer;

					int cnt = 0;
					for(node *child = n->first_node(nodeName); child != null; child = child->next_sibling())
					{
						assert(GetInt(child, L"ID") == cnt);
				
						mStrings[cnt++] = p;

						node *data = child->first_node();
						if(data != null && data->type() == node_type::node_data)
						{
							wcscpy(p, data->value());
							p += data->value_size() + 1;
						}
					}
					assert(cnt == count);
				}
			}
		}
	}

} // ::Dictionary