//////////////////////////////////////////////////////////////////////

#include "pch.h"

#pragma comment(lib, "Ws2_32.lib")

//////////////////////////////////////////////////////////////////////

static bool wasStartedUp = false;

Socket::Socket() : mSocket(INVALID_SOCKET)
{
	if(!wasStartedUp)
	{
		WSADATA wsaData;   // if this doesn't work
		//WSAData wsaData; // then try this instead

		// MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:

		if (WSAStartup(MAKEWORD(2,2), &wsaData) == 0)
		{
			wasStartedUp = true;
		}
		else
		{
			TRACE("WSAStartup failed.\n");
		}
	}
}

//////////////////////////////////////////////////////////////////////

Socket::~Socket()
{
	Close();
}

//////////////////////////////////////////////////////////////////////

bool Socket::Connect(char const *ip, uint32 port)
{
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo  hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	#define DEFAULT_PORT "32000"

	// Resolve the server address and port
	int iResult = getaddrinfo("107.21.245.151", "32000", &hints, &result);
	if (iResult != 0)
	{
		return false;
	}

	ptr = result;
	mSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

	if(mSocket == INVALID_SOCKET)
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////

void Socket::Close()
{
	if(mSocket != INVALID_SOCKET)
	{
		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}
}

//////////////////////////////////////////////////////////////////////

long Socket::Send(char const *data, uint32 len)
{
	return -1;
}

//////////////////////////////////////////////////////////////////////

long Socket::Receive(char const *buffer, uint32 bufferLen)
{
	return -1;
}


