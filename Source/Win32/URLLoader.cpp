//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Winhttp.h"

#pragma comment(lib,"winhttp.lib")

//////////////////////////////////////////////////////////////////////

struct URLLoader::Impl : list_node<URLLoader::Impl>
{
	URLLoader *								mParent;
	HINTERNET								mConnection;
	HINTERNET								mRequest;
	string									mData;
	DWORD									mBytesReceived;
	URLLoader::State						mState;
	std::function<void (bool, ValueMap)>	mCallback;
	string									mPostData;

	//////////////////////////////////////////////////////////////////////

	static linked_list<Impl> &sImpls()
	{
		static linked_list<Impl> impls;
		return impls;
	}

	//////////////////////////////////////////////////////////////////////

	static HINTERNET Session(bool close = false)
	{
		static HINTERNET _session = null;

		if(!close)
		{
			if(_session == null)
			{
				_session = WinHttpOpen(L"WinHTTP/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
			}
		}
		else
		{
			if(_session != null)
			{
				WinHttpCloseHandle(_session);
				_session = null;
			}
		}
		return _session;
	}

	//////////////////////////////////////////////////////////////////////

	Impl(URLLoader *parent) : mParent(parent), mState(New), mConnection(null)
	{
		sImpls().push_back(this);
		TRACE("+IMPLS: %u\n", sImpls().size());
	}

	//////////////////////////////////////////////////////////////////////

	~Impl()
	{
		Close();
		sImpls().remove(this);
		TRACE("IMPLS-: %u\n", sImpls().size());
		if(sImpls().empty())
		{
			Session(true);
		}
	}

	//////////////////////////////////////////////////////////////////////

	static void TerminateAll()
	{
		WinHttpSetStatusCallback(Session(), NULL, 0, 0 );
		for(auto &i: sImpls())
		{
			i.Close();
		}
	}

	//////////////////////////////////////////////////////////////////////

	vector<string> &split(const string &s, char delim, vector<string> &elems)
	{
		using namespace std;
		stringstream ss(s);
		string item;
		while(getline(ss, item, delim))
		{
			elems.push_back(item);
		}
		return elems;
	}

	//////////////////////////////////////////////////////////////////////

	static string urlDecode(string const &src)
	{
		string ret;
		for(uint i = 0; i < src.length(); ++i)
		{
			if(src[i] == '+')
			{
				ret += ' ';
			}
			else if (int(src[i]) == '%')
			{
				int ii;
				sscanf(src.substr(i+1, 2).c_str(), "%x", &ii);
				char ch = static_cast<char>(ii);
				ret += ch;
				i = i + 2;
			}
			else
			{
				ret += src[i];
			}
		}
		return (ret);
	}

	//////////////////////////////////////////////////////////////////////

	void Callback(HINTERNET handle, DWORD status, void *info, DWORD length)
	{
		switch(status)
		{
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			{
				WINHTTP_ASYNC_RESULT *result = (WINHTTP_ASYNC_RESULT *)info;
				TRACE("ERR: %d,%d\n", result->dwResult, result->dwError);
			}
			break;

		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			{
				WinHttpReceiveResponse(handle, null);
			}
			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				wchar *lpOutBuffer = null;
				BOOL bResults = false;
				DWORD dwSize;
				WinHttpQueryHeaders(handle, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);

				if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					lpOutBuffer = new WCHAR[dwSize/sizeof(WCHAR)];
					bResults = WinHttpQueryHeaders(handle, WINHTTP_QUERY_CONTENT_LENGTH, WINHTTP_HEADER_NAME_BY_INDEX, lpOutBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX);
				}

				if (bResults)
				{
					uint contentLength = (uint)asciiToInt64(lpOutBuffer);
					uint offset = mData.size();
					mData.resize(mData.size() + contentLength);
					WinHttpReadData(handle, (void *)(mData.c_str() + offset), contentLength, &mBytesReceived);
				}

				Delete(lpOutBuffer);
			}
			break;

		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			{
				// Crack it into a ValueMap and call the callback
				ValueMap values;
				using namespace std;
				vector<string> strings;
				split(mData, '&', strings);
				for(auto s = strings.begin(); s != strings.end(); ++s)
				{
					vector<string> nameValue;
					split(*s, '=', nameValue);
					if(nameValue.size() == 2)
					{
						vector<string> nameType;
						split(urlDecode(nameValue[0]), '-', nameType);

						string type = urlDecode(nameType[1]);
						string name = urlDecode(nameType[0]);
						string value = urlDecode(nameValue[1]);

						//TRACE("%s %s = %s\n", type.c_str(), name.c_str(), value.c_str());

						Value v;

						if(type == "int" || type == "long")
						{
							v.mType = Value::Int;
							v.mInt = asciiToInt64(value.c_str());
						}
						else if(type == "str" || type == "unicode")
						{
							v.mType = Value::String;
							v.mString = value;
						}
						else if(type == "datetime")
						{
							v.mType = Value::DateTime;
							v.mDateTime = ParseTime(value.c_str());
						}

						if(v.mType != Value::None)
						{
							char const *nameStr = name.c_str();
							if(values.find(nameStr) == values.end())
							{
								values[nameStr] = v;
							}
						}
					}
				}
				mPostData.clear();
				mState = Idle;
				mCallback(values.find("error") == values.end(), values);
			}
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////

	static void CALLBACK StatusCallback(HINTERNET handle, DWORD_PTR context, DWORD status, void *info, DWORD length)
	{
		((Impl *)context)->Callback(handle, status, info, length);
	}


	//////////////////////////////////////////////////////////////////////

	void Close()
	{
		if(mRequest != null)
		{
			WinHttpCloseHandle(mRequest);
			mRequest = null;
		}
		if(mConnection != null)
		{
			WinHttpCloseHandle(mConnection);
			mConnection = null;
		}
		mState = Idle;
	}

	//////////////////////////////////////////////////////////////////////

	void Load(char const *url, string const &postData, std::function<void(bool, ValueMap)> callback)
	{
		if(mState != Busy)
		{
			mCallback = callback;

			URL_COMPONENTS urlComponents = { 0 };
			wchar host[256];
			wchar urlPath[256];
			wchar extraInfo[256];
			urlComponents.dwStructSize = sizeof(urlComponents);
			urlComponents.lpszHostName = host;
			urlComponents.lpszExtraInfo = extraInfo;
			urlComponents.lpszUrlPath = urlPath;
			urlComponents.dwSchemeLength    = (DWORD)-1;
			urlComponents.dwHostNameLength  = (DWORD)256;
			urlComponents.dwUrlPathLength   = (DWORD)256;
			urlComponents.dwExtraInfoLength = (DWORD)256;
			wstring wideURLString = WideStringFromString(string(url));

			mData.clear();

			mConnection = null;
			mRequest = null;

			WinHttpSetStatusCallback(Session(), StatusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);

			mState = Busy;

			if(WinHttpCrackUrl(wideURLString.c_str(), wideURLString.size(), 0, &urlComponents))
			{
				mConnection = WinHttpConnect(Session(), urlComponents.lpszHostName, urlComponents.nPort, 0);

				if(mConnection != null)
				{
					mPostData = postData;
					wchar const *method = mPostData.empty() ? L"GET" : L"POST";

					wstring wholeReq = wstring(urlComponents.lpszUrlPath) + wstring(urlComponents.lpszExtraInfo);

					mRequest = WinHttpOpenRequest(mConnection, method, wholeReq.c_str(), null, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

					if(mRequest != null)
					{
						if(!WinHttpSendRequest(mRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (void *)mPostData.c_str(), mPostData.size(), mPostData.size(), (DWORD_PTR)this))
						{
							// error WinHttpSendRequest
							mState = Error;
						}
					}
					else
					{
						// error WinHttpOpenRequest
						mState = Error;
					}
				}
				else
				{
					// error WinHttpConnect
					mState = Error;
				}
			}
			else
			{
				// error WinHttpCrackUrl
				mState = Error;
			}

			if(mState != Busy)
			{
				Close();
			}
		}
	}

//////////////////////////////////////////////////////////////////////

	URLLoader::State GetState()
	{
		return mState;
	}
};

//////////////////////////////////////////////////////////////////////

URLLoader::URLLoader()
	: impl(new Impl(this))
{
}

//////////////////////////////////////////////////////////////////////

URLLoader::~URLLoader()
{
	Delete(impl);
}

//////////////////////////////////////////////////////////////////////

void URLLoader::Close()
{
	impl->Close();
}

//////////////////////////////////////////////////////////////////////

void URLLoader::Load(char const *url, string const &postData, std::function<void(bool, ValueMap)> callback)
{
	impl->Load(url, postData, callback);
}

//////////////////////////////////////////////////////////////////////

URLLoader::State URLLoader::GetState() const
{
	return impl->GetState();
}

//////////////////////////////////////////////////////////////////////

void URLLoader::TerminateAll()
{
	Impl::TerminateAll();
}