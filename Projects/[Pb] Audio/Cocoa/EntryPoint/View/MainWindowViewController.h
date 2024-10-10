//
//  ViewController.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define CocoaView                NSView
#define CocoaViewController      NSViewController
#define CocoaScreenSize          NSScreen.mainScreen.frame.size
#define CocoaWindow              NSWindow
#else
#import <UIKit/UIKit.h>
#define CocoaView                UIView
#define CocoaViewController      UIViewController
#define CocoaScreenSize          UIScreen.mainScreen.bounds.size
#define CocoaWindow              UIWindow
#endif

@interface MainWindowViewController : CocoaViewController

-(id)initWithView:(CocoaView*)view;
-(id)initWithWindow:(CocoaWindow*)window;
//-(void)removeSubviews;

@end

