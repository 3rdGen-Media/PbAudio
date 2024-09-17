//
//  SettingsModalViewController.m
//  C4Commander
//
//  Created by Joe Moulton on 11/24/23.
//

#import "SettingsModalViewController.h"

@interface SettingsModalViewController ()

@end

@implementation SettingsModalViewController


-(id)initWithView:(NSView*)view
{
    self = [super init];
    if( self)
    {
        self.view = view;
    }
    return self;
}


- (void)viewDidAppear
{
    [super viewDidAppear];

    //[self.view.window makeFirstResponder: self];
}


-(void)viewWillAppear
{
    self.view.layer.backgroundColor = [NSColor orangeColor].CGColor;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    
    // Do view setup here.
    //[self.view.window makeKeyAndOrderFront:self];
    //[self.view.window makeFirstResponder:self.view];
    
    //CMThruModalView* modalView = (CMThruModalView*)self.view;
    //self.preferredContentSize = NSMakeSize(self.view.frame.size.width, modalView.cancelButton.frame.size.height + modalView.cancelButton.frame.origin.y + 20);
    
    [self.view setNeedsLayout:YES];
    [self.view display];
}

-(void)updateViewConstraints
{
    [super updateViewConstraints];
}


@end
