//
//  URLLoaderDelegate.h
//  Make The Words
//
//  Created by Charlie Skilbeck on 20/12/2012.
//  Copyright (c) 2012 Ice Cream Software. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface URLLoaderDelegate : NSObject
{
	NSMutableData *							mURLResponseData;
	std::function<void (bool, ValueMap)>	mDataReceivedCallback;
}

- (bool)loadURL:(char const *)url withPostData:(string const &)postData andCallback:(std::function<void (bool, ValueMap)>)callback;

@end
