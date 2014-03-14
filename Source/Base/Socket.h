//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Socket
{
	Socket();
	~Socket();

	bool Connect(char const *ip, uint32 port);
	void Close();

	long Send(char const *data, uint32 len = 0);
	long Receive(char const *buffer, uint32 bufferLen);

#if defined(IOS)
	CFReadStreamRef		mReadStream;
	CFWriteStreamRef	mWriteStream;
	void HandleEvent(CFStreamEventType eventType);
#else if defined(WIN32)
	SOCKET				mSocket;
#endif

	string				mReceiveBuffer;
};


