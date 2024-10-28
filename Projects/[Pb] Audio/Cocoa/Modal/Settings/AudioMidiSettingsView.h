//
//  AudioMidiSettingsView.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define CocoaTextField         NSTextField
#define CocoaTextFieldDelegate NSTextFieldDelegate
#define CocoaButton            NSButton
#define CocoaVisualEffectView  NSVisualEffectView
#define CocoaScrollView        NSScrollView
#define CocoaPopUpButton       NSPopUpButton
#define CocoaFont              UIFont
#else
#import <UIKit/UIKit.h>
#define CocoaTextField         UILabel
#define CocoaTextFieldDelegate UITextFieldDelegate
#define CocoaButton            UIButton
#define CocoaVisualEffectView  UIVisualEffectView
#define CocoaScrollView        UIScrollView
#define CocoaPopUpButton       UIButton
#define CocoaFont              UIFont
#endif

#import "NSCheckboxView.h"

NS_ASSUME_NONNULL_BEGIN

@interface AudioMidiSettingsView : CocoaVisualEffectView <CocoaTextFieldDelegate, NSCheckboxViewDelegate>

//-(void)updateDocumentView;

//-(id)initWithFrame:(NSRect)frame andProxy:(NSString* _Nullable)thruID;
//@property (nonatomic, readonly) CMProxyConnection * dataModel;

//-(id)initWithFrame:(NSRect)frame andActiveDevice:(CMHardwareDevice* _Nullable)device;


@property (nonatomic, readonly) CocoaButton* cancelButton;

@end

NS_ASSUME_NONNULL_END
