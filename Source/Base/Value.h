#pragma once

//////////////////////////////////////////////////////////////////////

struct Value
{
	enum Type
	{
		None,
		Int,
		String,
		DateTime
	};

	Value() : mType(None)
	{
	}
	
	char const *TypeName() const
	{
		switch(mType)
		{
		case None:		return "None";
		case Int:		return "int";
		case String:	return "str";
		case DateTime:	return "datetime";
		default:		return "Unknown";
		}
	}
	
	string ToString() const
	{
		switch(mType)
		{
		case None:		return string();
		case Int:		return Format("%lld", mInt);
		case String:	return mString;
		case DateTime:	return Format("%s", FormatTime(mDateTime).c_str());
		default:		return string();
		}
	}
	
	operator string() const
	{
		return ToString();
	}
	
	operator int32() const
	{
		switch(mType)
		{
		case None:		return 0;
		case Int:		return (int32)mInt;
		case String:	return atoi(mString.c_str());
		case DateTime:	return (int32)mDateTime;
		default:		return 0;
		}
	}

	operator uint32() const
	{
		switch(mType)
		{
		case None:		return 0;
		case Int:		return (uint32)mInt;
		case String:	return (uint32)atoi(mString.c_str());
		case DateTime:	return (uint32)mDateTime;
		default:		return 0;
		}
	}
	
	operator int64() const
	{
		switch(mType)
		{
		case None:		return 0;
		case Int:		return mInt;
		case String:	return asciiToInt64(mString.c_str());
		case DateTime:	return (int64)mDateTime;
		default:		return 0;
		}
	}
	
	operator uint64() const
	{
		switch(mType)
		{
		case None:		return 0;
		case Int:		return (uint64)mInt;
		case String:	return (uint64)asciiToInt64(mString.c_str());
		case DateTime:	return (uint64)mDateTime;
		default:		return 0;
		}
	}
	
	Type		mType;
	int64		mInt;
	string		mString;
	time_t		mDateTime;
};

typedef map<string, Value> ValueMap;

inline void DumpValueMap(ValueMap v)
{
	TRACE("%d values:\n", v.size());
	for(auto i = v.begin(); i != v.end(); ++i)
	{
		TRACE("%s %s = %s\n", i->second.TypeName(), i->first.c_str(), i->second.ToString().c_str());
	}
}


