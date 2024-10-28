//
//  NSCheckboxView.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/31/22.
//

#include <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define CocoaView       NSView
#define CocoaTextField  NSTextField
#define CocoaColor      NSColor
#define ToggleButton    NSButton
#else
#import <UIKit/UIKit.h>
#define CocoaView       UIView
#define CocoaTextField  UILabel
#define CocoaColor      UIColor
#define ToggleButton    UISwitch
#endif


NS_ASSUME_NONNULL_BEGIN

#ifndef NSTextAlignmentVertical
#define NSTextAlignmentVertical 5
#endif

@protocol NSCheckboxViewDelegate <NSObject>
-(void)buttonToggledAtIndex:(int)index sender:(id)sender;
@end

@interface NSCheckboxView : CocoaView

-(id)initWithIdentifiers:(NSArray*)identifiers Title:( NSString* _Nullable )title Justification:(NSTextAlignment)alignment;

@property (nullable, weak) id<NSCheckboxViewDelegate> delegate;
@property (nonatomic, readonly) NSMutableArray * buttons;

@end

NS_ASSUME_NONNULL_END
