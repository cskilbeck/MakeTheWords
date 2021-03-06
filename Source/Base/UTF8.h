//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

void AsciiToWide(char const *input, wchar *output);
bool UTF8ToWide(uint8 *input, size_t inputSize, size_t *decodedLength, wchar *decodeBuffer);
wstring WideStringFromUTF8(char const *input);
string AsciiStringFromWide(wchar const *input);

//////////////////////////////////////////////////////////////////////

extern const uint8 utf8d[];

struct UTF8Decoder
{
	wchar			unic;
	char const *	ptr;
	char const *	currentPtr;
	size_t			inputSize;
	uint8			stat;

	UTF8Decoder(char const *input, size_t inputBytesLength = -1)
	{
		stat = 9;
		unic = 0;
		ptr = input;
		inputSize = inputBytesLength;
	}

	// returns: 0 = end of stream, 0xffffffff = decode error, else decoded unicode character
	// calling Next() after it has returned 0 or -1 is undefined (crash)
	wchar Next()
	{
		char b;
		while(true)
		{
			if(inputSize == 0)
			{
				return 0;
			}

			--inputSize;
			currentPtr = ptr;
			b = *ptr++;
			if(b == 0)
			{
				return 0;
			}

			uint8 data = utf8d[(uint8)b];
			stat = utf8d[256 + (stat << 4) + (data >> 4)];
			b = (b ^ (uint8)(data << 4));
			unic = (unic << 6) | b;
			if (stat == 0)
			{
				wchar temp = unic;
				unic = 0;
				return temp;
			}
			if(stat == 1)
			{
				return -1;
			}
		}
	}
};


