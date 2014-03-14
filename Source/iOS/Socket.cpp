//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

Socket::Socket() : mReadStream(null), mWriteStream(null)
{
	mReceiveBuffer.resize(4096);
}

//////////////////////////////////////////////////////////////////////

Socket::~Socket()
{
	Close();
}

//////////////////////////////////////////////////////////////////////

void Socket::Close()
{
	CFReadStreamClose(mReadStream);
	CFRelease(mReadStream);
	mReadStream = null;

	CFWriteStreamClose(mWriteStream);
	CFRelease(mWriteStream);
	mWriteStream = null;
}

//////////////////////////////////////////////////////////////////////

static void MyReadStreamEventCallback(CFReadStreamRef stream, CFStreamEventType eventType, void *clientCallBackInfo)
{
	Socket *s = (Socket *)clientCallBackInfo;
	s->HandleEvent(eventType);
}

//////////////////////////////////////////////////////////////////////

static void MyWriteStreamEventCallback(CFWriteStreamRef stream, CFStreamEventType eventType, void *clientCallBackInfo)
{
	Socket *s = (Socket *)clientCallBackInfo;
	s->HandleEvent(eventType);
}

//////////////////////////////////////////////////////////////////////

void Socket::HandleEvent(CFStreamEventType eventType)
{
	static bool sent = false;
	
	switch (eventType)
	{
		case kCFStreamEventOpenCompleted:
			NSLog(@"kCFStreamEventOpenCompleted");
			break;
		
		case kCFStreamEventHasBytesAvailable:
			Receive(mReceiveBuffer.c_str(), mReceiveBuffer.size());
			NSLog(@"kCFStreamEventHasBytesAvailable: %s", mReceiveBuffer.c_str());
			break;
		
		case kCFStreamEventCanAcceptBytes:
			NSLog(@"kCFStreamEventCanAcceptBytes");
			if(!sent)
			{
				Send("{\"action\":\"ping\"}");
				sent = true;
			}
			break;
		
		case kCFStreamEventErrorOccurred:
			NSLog(@"kCFStreamEventErrorOccurred");
			break;
		
		case kCFStreamEventEndEncountered:
			NSLog(@"kCFStreamEventEndEncountered");
			break;

		default:
			break;
	}
}

//////////////////////////////////////////////////////////////////////
// 107.21.245.151

bool Socket::Connect(char const *ip, uint32 port)
{
	bool rc = false;
	CFStringRef ipStr = CFStringCreateWithCString(null, ip, kCFStringEncodingUTF7);
	CFStreamCreatePairWithSocketToHost(null, ipStr, port, &mReadStream, &mWriteStream);
	CFRelease(ipStr);

	CFStreamClientContext context = { 0, this, null, null, null };

	if(CFReadStreamSetClient(mReadStream,
								kCFStreamEventOpenCompleted +
								kCFStreamEventHasBytesAvailable +
								kCFStreamEventCanAcceptBytes +
								kCFStreamEventErrorOccurred +
								kCFStreamEventEndEncountered,
							 MyReadStreamEventCallback,
							 &context))
	{
		CFReadStreamScheduleWithRunLoop(mReadStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

		if(CFWriteStreamSetClient(mWriteStream,
								  kCFStreamEventOpenCompleted +
								  kCFStreamEventHasBytesAvailable +
								  kCFStreamEventCanAcceptBytes +
								  kCFStreamEventErrorOccurred +
								  kCFStreamEventEndEncountered,
								  MyWriteStreamEventCallback,
								  &context))
		{
			CFWriteStreamScheduleWithRunLoop(mWriteStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
			
			if(CFReadStreamOpen(mReadStream))
			{
				if(CFWriteStreamOpen(mWriteStream))
				{
					rc = true;
				}
				else
				{
					NSLog(@"Error of CFWriteStreamOpen");
				}
			}
			else
			{
				NSLog(@"Error of CFReadStreamOpen");
			}
		}
		else
		{
			NSLog(@"Error of CFWriteStreamSetClient");
		}
	}
	else
	{
		NSLog(@"Error on CFReadStreamSetClient");
	}
	if(!rc)
	{
		Close();
	}
	return rc;
}

//////////////////////////////////////////////////////////////////////

long Socket::Send(char const *data, uint32 len)
{
	if(len == 0)
	{
		len = strlen(data);
	}
	return CFWriteStreamWrite(mWriteStream, (uint8 const *)data, (CFIndex)len);
}

//////////////////////////////////////////////////////////////////////

long Socket::Receive(char const *buffer, uint32 bufferLen)
{
	return CFReadStreamRead(mReadStream, (uint8 *)buffer, (CFIndex)bufferLen);
}

//////////////////////////////////////////////////////////////////////

