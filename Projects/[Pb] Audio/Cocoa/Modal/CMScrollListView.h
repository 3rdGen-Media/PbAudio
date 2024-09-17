//
//  CMScrollListView.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/28/22.
//

#import <Cocoa/Cocoa.h>
#import "CMProxyConnection.h"

NS_ASSUME_NONNULL_BEGIN

@interface CMScrollListView : NSScrollView <NSTextFieldDelegate, NSTableViewDelegate, NSTableViewDataSource>

-(id)initWithFrame:(NSRect)frameRect andDOM:(CMDOM*)dataModel;

@property (nonatomic, retain) NSColor * fillColor;
-(void)redraw;

@end

NS_ASSUME_NONNULL_END
