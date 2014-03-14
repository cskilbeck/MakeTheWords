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

	//virtual void OnReceive(char const *buffer, uint32 len);
	//virtual void OnSent(char const *buffer, uint32 sentLen);
	//virtual void OnError(uint32 error);

	CFReadStreamRef		mReadStream;
	CFWriteStreamRef	mWriteStream;
	string				mReceiveBuffer;

	void HandleEvent(CFStreamEventType eventType);

};


