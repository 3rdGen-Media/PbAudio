//
//  SizeToFitView.m
//  MCUCommander
//
//  Created by Joe Moulton on 12/6/23.
//

#import "SizeToFitView.h"

@implementation SizeToFitView


/*
-(void)layout
{
    
    CGRect frame = self.frame;
    if(self.topAnchor && self.bottomAnchor)
    {
        frame.size.height = self.bottomAnchor.frame.origin.y - (self.topAnchor.frame.origin.y + self.topAnchor.frame.size.height);
        
    }
    self.frame = frame;
    
    [super layout];

}
*/

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

@end
