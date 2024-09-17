//
//  HighlightImageView.m
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#import "HighlightImageView.h"

@implementation HighlightImageView

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

    if (self.selected) 
    {
        NSColor * highlightColor = [[NSColor selectedContentBackgroundColor] colorWithAlphaComponent:0.5];
        //NSColor * highlightColorAlpha = [NSColor colorWithRed:highlightColor.redComponent * 255.99999 green:highlightColor.greenComponent * 255.99999 blue:highlightColor.blueComponent * 255.99999 alpha:0.5];
        [highlightColor set];

       //NSRectFill([self bounds]);
       NSRectFillUsingOperation([self bounds], NSCompositingOperationSourceOver);

    }
}
@end
