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
#define CocoaViewController      AUViewController
#define CocoaScreenSize          NSScreen.mainScreen.frame.size
#define CocoaWindow              NSWindow
#else
#import <UIKit/UIKit.h>
#define CocoaView                UIView
#define CocoaViewController      AUViewController
#define CocoaScreenSize          UIScreen.mainScreen.bounds.size
#define CocoaWindow              UIWindow
#endif

#import <CoreAudioKit/AUViewController.h>

//@class SampleAudioUnit;

@interface AudioUnitViewController : CocoaViewController

-(id)initWithView:(CocoaView*)view;         //App Extension Init Path
-(id)initWithWindow:(CocoaWindow*)window;   //Application   Init Path
//-(void)removeSubviews;

//Any [Pb] Audio Application can also be used to target an Audio Unit Extension (for packaging as an AUv3  plugin)
@property (nonatomic) AUAudioUnit * audioUnit;
//var hostingController: HostingController<SampleInstrumentMainView>?

@end

//Extend AudioUnitViewController to adhere to the AUAudioUnitFactory protocol when building an AUv3 plugin app extension
@interface AudioUnitViewController (AUAudioUnitFactory) <AUAudioUnitFactory>
@end
