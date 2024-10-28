//
//  SettingsModalViewController.h
//  C4Commander
//
//  Created by Joe Moulton on 11/24/23.
//


#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define CocoaDelegateSuperclass NSObject
#define CocoaAppDelegate        NSApplicationDelegate
#define CocoaViewController     NSViewController
#define CocoaView               NSView
//#import "MCUCommanderMenu.h"
//#import "MCUCommanderMainMenu.h"
#else
#import  <UIKit/UIKit.h>
#define CocoaDelegateSuperclass UIResponder
#define CocoaAppDelegate        UIApplicationDelegate, UIWindowSceneDelegate
#define CocoaViewController     UIViewController
#define CocoaView               UIView
#endif


NS_ASSUME_NONNULL_BEGIN

@interface SettingsModalViewController : CocoaViewController

-(id)initWithView:(CocoaView*)view;

@end

NS_ASSUME_NONNULL_END
