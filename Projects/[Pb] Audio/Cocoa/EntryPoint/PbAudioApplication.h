//
//  PbAudioApplication.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
//#import <AppKit/AppKit.h>
#define CocoaApplication NSApplication
#else
#import <UIKit/UIKit.h>
#define CocoaApplication UIApplication
#endif

NS_ASSUME_NONNULL_BEGIN
@interface PbAudioApplication : CocoaApplication

+(PbAudioApplication*)sharedInstance;

#if !TARGET_OS_OSX
-(void)replyToApplicationShouldTerminate:(BOOL)shouldTerminate;
#endif

@end
NS_ASSUME_NONNULL_END
