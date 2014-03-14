//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Facebook.h"
#import <FacebookSDK/FacebookSDK.h>

//////////////////////////////////////////////////////////////////////

namespace
{
	//////////////////////////////////////////////////////////////////////
	
	NSString *const FBSessionStateChangedNotification = @"ics.Make-The-Words.Login:FBSessionStateChangedNotification";
	
	string sUserName;
	uint64 sUserID;

	//////////////////////////////////////////////////////////////////////
	
	void SessionStateChanged(FBSession *session, FBSessionState state, NSError *error)
	{
		switch(state)
		{
			case FBSessionStateOpen:
				if (!error)
				{
					NSLog(@"User session found");
				}
				break;
				
			case FBSessionStateClosed:
			case FBSessionStateClosedLoginFailed:
				Facebook::Logout();
				break;
				
			default:
				break;
		}

		[[NSNotificationCenter defaultCenter] postNotificationName:FBSessionStateChangedNotification
															object:session];
		
		if (error)
		{
			[[[UIAlertView alloc] initWithTitle:@"Error"
										message:error.localizedDescription
									   delegate:nil
							  cancelButtonTitle:@"OK"
							  otherButtonTitles:nil] show];
		}
	}
}

//////////////////////////////////////////////////////////////////////

namespace Facebook
{
	//////////////////////////////////////////////////////////////////////
	
	void Login(bool allowUI)
	{
		NSArray *permissions = [[NSArray alloc] initWithObjects: nil];
		
//		[FBRequest requestWithGraphPath:<#(NSString *)#> parameters:<#(NSDictionary *)#> HTTPMethod:<#(NSString *)#>]
		
		[FBSession openActiveSessionWithReadPermissions:permissions
										   allowLoginUI:allowUI
									  completionHandler:^(FBSession *session, FBSessionState state, NSError *error)
		{
			SessionStateChanged(session, state, error);
			
			if(IsLoggedIn())
			{
				[[FBRequest requestForMe] startWithCompletionHandler:^(FBRequestConnection *connection, NSDictionary<FBGraphUser> *user, NSError *error)
				{
					if(!error)
					{
						sUserName = [user.name UTF8String];
						sUserID = [user.id longLongValue];
					}
					else
					{
						sUserName.clear();
						sUserID = 0;
					}
				}];
			}
		}];
	}
	
	//////////////////////////////////////////////////////////////////////
	
	void Logout()
	{
		sUserName.clear();
		sUserID = 0;
		[FBSession.activeSession closeAndClearTokenInformation];
	}
	
	//////////////////////////////////////////////////////////////////////
	
	void OnAppActivate()
	{
		[FBSession.activeSession handleDidBecomeActive];
	}
	
	//////////////////////////////////////////////////////////////////////
	
	bool IsLoggedIn()
	{
		return FBSession.activeSession.isOpen;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	bool HandleOpenURL(NSURL *url)
	{
		return [FBSession.activeSession handleOpenURL:url];
	}

	//////////////////////////////////////////////////////////////////////
	
	void RequestPostPermissions(function<void()> callback)
	{
		if(IsLoggedIn())
		{
			// Ask for publish_actions permissions in context
			if ([[[FBSession activeSession] permissions] indexOfObject:@"publish_actions"] == NSNotFound)
			{
				// No permissions found in session, ask for it
				[[FBSession activeSession] reauthorizeWithPublishPermissions:[NSArray arrayWithObject:@"publish_actions"]
															 defaultAudience:FBSessionDefaultAudienceEveryone
														   completionHandler:^(FBSession *session, NSError *error)
				 {
					callback();
				 }];
			}
		}
		else
		{
			callback();
		}
	}

	//////////////////////////////////////////////////////////////////////
	
	void PostStory()
	{
		NSMutableDictionary *postParams;
		postParams = [[NSMutableDictionary alloc] initWithObjectsAndKeys:
					  @"http://www.ice-cream-software.com", @"link",
					  @"https://developers.facebook.com/attachment/iossdk_logo.png", @"picture",
					  @"The First Post from Make The Words", @"name",
					  @"Make The Words wakes up...", @"caption",
					  @"This post came from some code running in an iPhone simulator...", @"description",
					  nil];
		
		[FBRequestConnection startWithGraphPath:@"me/feed"
									 parameters:postParams
									 HTTPMethod:@"POST"
							  completionHandler:^(FBRequestConnection *connection, id result, NSError *error)
		{
			NSString *alertText;
			if (error)
			{
				alertText = [NSString stringWithFormat:@"error: domain = %@, code = %d", error.domain, error.code];
			}
			else
			{
				alertText = [NSString stringWithFormat:@"Posted action, id: %@", [result objectForKey:@"id"]];
			}
		 
			[[[UIAlertView alloc] initWithTitle:@"Result"
										message:alertText
									   delegate:nil
							  cancelButtonTitle:@"OK!"
							  otherButtonTitles:nil]
			 show];
		}];
	}
	
	//////////////////////////////////////////////////////////////////////
	
	string const &Username()
	{
		return sUserName;
	}
	
	//////////////////////////////////////////////////////////////////////
	
	uint64 UserID()
	{
		return sUserID;
	}

	//////////////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////////

