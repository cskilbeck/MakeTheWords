
#import "URLLoaderDelegate.h"

@implementation URLLoaderDelegate

//////////////////////////////////////////////////////////////////////

- (bool)loadURL:(char const *)url withPostData:(string const &)postData andCallback:(std::function<void (bool, ValueMap)>)callback
{
	mDataReceivedCallback = callback;
	NSString *s = [[NSString alloc] initWithCString:url encoding:NSASCIIStringEncoding];
	NSURL *myURL = [NSURL URLWithString:s];
	NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:myURL cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:60];
	uint pdlen = postData.length();
	[request setHTTPMethod:@"POST"];
	[request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
	[request setValue:[NSString stringWithFormat:@"%d", pdlen] forHTTPHeaderField:@"Content-Length"];
	[request setHTTPBody:[[NSData alloc] initWithBytes:postData.c_str() length:pdlen]];
	NSURLConnection *connection = [[NSURLConnection alloc] initWithRequest:request delegate:self];
	if(connection)
	{
		mURLResponseData = [[NSMutableData alloc] init];
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////

-(void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
	[mURLResponseData setLength:0];
}

//////////////////////////////////////////////////////////////////////

-(void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
	[mURLResponseData appendData:data];
}

//////////////////////////////////////////////////////////////////////

-(void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	[mURLResponseData release];
	[connection release];
	ValueMap valueMap;
	Value errorCode;
	Value errorString;
	errorCode.mType = Value::eType::kInt;
	errorCode.mInt = 1;
	errorString.mType = Value::eType::kString;
	errorString.mString = string([[error localizedDescription] UTF8String]);
	valueMap["error_code"] = errorCode;
	valueMap["error_string"] = errorString;
	mDataReceivedCallback(false, valueMap);
}

//////////////////////////////////////////////////////////////////////

-(void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	NSString *txt = [[[NSString alloc] initWithData:mURLResponseData encoding:NSASCIIStringEncoding] autorelease];
	
	NSArray * results = [[NSArray alloc] init];
	results = [txt componentsSeparatedByString:@"&"];
	
	map<string, Value> values;
	
	for(NSString *p in results)
	{
		NSArray * s = [p componentsSeparatedByString:@"="];
		
		if([s count] == 2)
		{
			NSString *value = (NSString *)[s objectAtIndex:1];
			NSArray *nameType = [(NSString *)[s objectAtIndex:0] componentsSeparatedByString:@"-"];
			if([nameType count] == 2)
			{
				value = [value stringByReplacingOccurrencesOfString:@"+" withString:@" "];
				value = [value stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
				NSString *name = (NSString *)[nameType objectAtIndex:0];
				NSString *type = (NSString *)[nameType objectAtIndex:1];
				
				Value v;
				if([type isEqualToString:@"int"] || [type isEqualToString:@"long"])
				{
					v.mType = Value::kInt;
					v.mInt = atoll([value UTF8String]);
				}
				else if([type isEqualToString:@"str"] || [type isEqualToString:@"unicode"])
				{
					v.mType = Value::kString;
					v.mString = string([value UTF8String]);
				}
				else if([type isEqualToString:@"datetime"])
				{
					v.mType = Value::kDateTime;
					v.mDateTime = ParseTime([value UTF8String]);
				}
				
				if(v.mType != Value::kNone)
				{
					char const *nameStr = [name UTF8String];
					if(values.find(nameStr) == values.end())
					{
						values[nameStr] = v;
					}
				}
			}
		}
	}
	bool success = values.find("error") == values.end();
	mDataReceivedCallback(success, values);
}

@end


