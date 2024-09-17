//
//  NSCheckboxView.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/31/22.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

#ifndef NSTextAlignmentVertical
#define NSTextAlignmentVertical 5
#endif

@protocol NSCheckboxViewDelegate <NSObject>
-(void)buttonToggledAtIndex:(int)index sender:(id)sender;
@end

@interface NSCheckboxView : NSView

-(id)initWithIdentifiers:(NSArray*)identifiers Title:( NSString* _Nullable )title Justification:(NSTextAlignment)alignment;

@property (nullable, weak) id<NSCheckboxViewDelegate> delegate;
@property (nonatomic, readonly) NSMutableArray * buttons;

@end

NS_ASSUME_NONNULL_END
