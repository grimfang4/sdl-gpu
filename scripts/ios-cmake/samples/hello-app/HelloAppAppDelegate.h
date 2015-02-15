#import <UIKit/UIKit.h>

@class HelloIOS;

@interface HelloAppAppDelegate : NSObject <UIApplicationDelegate> {
	UIWindow *window;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end
