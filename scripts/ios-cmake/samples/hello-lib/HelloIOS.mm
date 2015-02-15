#import "HelloIOS.h"
#include "HelloWorld.h"

@implementation HelloIOS

- (void)viewDidLoad{
	UIView *view=[[UIView alloc] initWithFrame:[UIScreen mainScreen].applicationFrame];
	self.view=view;

	UILabel *label=[[UILabel alloc] initWithFrame:[UIScreen mainScreen].applicationFrame];
	[self.view addSubview:label];
	label.textAlignment=UITextAlignmentCenter;
	label.text=[self getHello];
	[label release];
}

- (NSString*)getHello{
	HelloWorld h;
	NSString *text=[NSString stringWithUTF8String: h.helloWorld().c_str()];
	return text;
}

- (void)dealloc{
	[super dealloc];
}

@end
