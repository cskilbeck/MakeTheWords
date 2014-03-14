//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct URLLoader
{
	enum State
	{
		New,
		Busy,
		Idle,
		Error
	};
	
	URLLoader();
	~URLLoader();

	static void TerminateAll();

	void Close();
	void Load(char const *url, string const &postData, std::function<void(bool, ValueMap)> callback);
	State GetState() const;
	
private:
	
	struct Impl;
	Impl *impl;
};

