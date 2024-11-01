//
//  AppDelegate.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define CocoaDelegateSuperclass NSObject
#define CocoaAppDelegate        NSApplicationDelegate
#define CocoaWindow             NSWindow
#define CocoaToolbarDelelegate  NSToolbarDelegate
#define CocoaToolbar            NSToolbar
#define CocoaImage              NSImage

//#import "MCUCommanderMenu.h"
//#import "MCUCommanderMainMenu.h"
#else
#define CocoaDelegateSuperclass UIResponder
#define CocoaAppDelegate        UIApplicationDelegate, UIWindowSceneDelegate
#define CocoaWindow             UIWindow
#define CocoaToolbarDelelegate  UIToolbarDelegate
#define CocoaToolbar            UIToolbar
#define CocoaImage              UIImage
#endif


#import "PbAudioApplication.h"
#import "CustomButtonView.h"
//#import "MCUDeviceView.h"

@interface PbAudioAppDelegate : CocoaDelegateSuperclass <CocoaAppDelegate, CocoaToolbarDelelegate, CustomButtonViewDelegate>

+ (PbAudioAppDelegate*)sharedInstance;

@property (strong, nonatomic) CocoaWindow *window;
@property (strong, nonatomic) CocoaWindow *modalWindow;

//-(void)openActiveDeviceWindow;
-(void)openSettingsManagerModalWindow;
//-(void)openDeviceManagerModalWindow:(NSString*)thruID;
//-(void)openProxyManagerModalWindow:(NSString*)thruID;
//-(void)openProxyConnectionModalWindow:(NSString*)thruID;

@end
