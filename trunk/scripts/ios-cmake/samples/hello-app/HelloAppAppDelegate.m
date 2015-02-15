#import "HelloAppAppDelegate.h"

#import "HelloIOS.h"

@implementation HelloAppAppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
	[HelloIOS class];
	[window makeKeyAndVisible];
}

- (void)dealloc{
	[window release];
	[super dealloc];
}

@end
