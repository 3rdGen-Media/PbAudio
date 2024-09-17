//
//  DVIView.m
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#import "DVIView.h"

@implementation DVIView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

    if (self.selected) {
       [[NSColor blueColor] set];
       NSRectFill([self bounds]);
    }
}

@end
