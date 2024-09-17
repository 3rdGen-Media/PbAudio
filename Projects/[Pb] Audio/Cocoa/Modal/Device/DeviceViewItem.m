//
//  DeviceViewItem.m
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#import "DeviceViewItem.h"
#import "DVIView.h"

#import "PbAudioAppDelegate.h"

@interface DeviceViewItem ()
{

}

@property (nonatomic, retain) NSView * labelContainer;

@end



@implementation DeviceViewItem


-(void)createDeviceImageView
{
    
    
    //self.imageView = [[NSImageView alloc] initWithFrame:CGRectMake(0,0,150, 150)];
    //self.imageView.image = [NSImage imageNamed:NSImageNameNetwork];
    self.deviceImageView = [[HighlightImageView alloc] initWithFrame:CGRectMake(0,0,150, 150)];
    
    self.deviceImageView.wantsLayer = YES;
    self.deviceImageView.layer.backgroundColor = [NSColor clearColor].CGColor;
    self.deviceImageView.layer.cornerRadius = 8.0;//self.imageView.frame.size.width/2.0;
    //self.deviceImageView.layer.borderWidth = 1.5;
    //self.deviceImageView.layer.borderColor = [NSColor whiteColor].CGColor;

    [self.view addSubview:self.deviceImageView];
    
    
    self.deviceImageView.translatesAutoresizingMaskIntoConstraints = NO;
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem:self.deviceImageView
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:0];

    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem:self.deviceImageView
                                                                attribute: NSLayoutAttributeLeft
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeLeft
                                                                 multiplier:1.0f constant:0];

    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem:self.deviceImageView
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1 constant:0.0f];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem:self.deviceImageView
                                                                attribute: NSLayoutAttributeHeight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeWidth
                                                                 multiplier:1 constant:0];
    [self.view addConstraints:@[ Top, Left, Right, Height]];
}

//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createDeviceLabel
{
    self.labelContainer = [[NSView alloc] initWithFrame:CGRectZero];
    self.labelContainer.wantsLayer = YES;
    self.labelContainer.layer.backgroundColor = [NSColor clearColor].CGColor;
    [self.view addSubview:self.labelContainer];

    self.labelContainer.translatesAutoresizingMaskIntoConstraints = NO;
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem:self.labelContainer
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.deviceImageView
                                                                attribute:NSLayoutAttributeBottom
                                                                multiplier:1.0f constant:0];

    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem:self.labelContainer
                                                                attribute: NSLayoutAttributeLeft
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeLeft
                                                                 multiplier:1.0f constant:0];

    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem:self.labelContainer
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1 constant:0.0f];
    
    NSLayoutConstraint * Bottom = [NSLayoutConstraint constraintWithItem:self.labelContainer
                                                                attribute: NSLayoutAttributeBottom
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.view
                                                                attribute:NSLayoutAttributeBottom
                                                                 multiplier:1 constant:0];
    [self.view addConstraints:@[ Top, Left, Right, Bottom]];
    
    
    self.deviceLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    //[textField setCell: [[CMTextFieldCell alloc] init]];
    self.deviceLabel.focusRingType = NSFocusRingTypeDefault;
    self.deviceLabel.stringValue  = @"New Device";
    //_descriptionLabel.font = [NSFont systemFontOfSize:36.0];
    self.deviceLabel.wantsLayer = YES;
    //self.descriptionLabel.bordered = YES;
    
    //self.descriptionLabel.enabled = YES;
    self.deviceLabel.editable = NO;
    self.deviceLabel.drawsBackground = YES;
    self.deviceLabel.layer.masksToBounds = NO;
    
    self.deviceLabel.usesSingleLineMode = NO;
    self.deviceLabel.maximumNumberOfLines = 2;
    self.deviceLabel.textColor = NSColor.whiteColor;
    self.deviceLabel.backgroundColor = [NSColor clearColor];
    self.deviceLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.deviceLabel.alignment = NSTextAlignmentCenter;
    
    self.deviceLabel.bezeled         = NO;
    self.deviceLabel.editable        = NO;
    self.deviceLabel.drawsBackground = NO;
    
    [self.labelContainer addSubview:self.deviceLabel];
    

    self.deviceLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem: self.deviceLabel
                                                                attribute: NSLayoutAttributeCenterX
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.labelContainer attribute:NSLayoutAttributeCenterX
                                                               multiplier:1 constant:0.0];
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.deviceLabel
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.labelContainer attribute:NSLayoutAttributeCenterY
                                                               multiplier:1 constant:0.0];
    /*
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem: self.deviceLabel
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.labelContainer attribute:NSLayoutAttributeHeight
                                                               multiplier:0.5 constant:0.0f];
    */
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.deviceLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.labelContainer attribute:NSLayoutAttributeWidth
                                                             multiplier:1. constant:0];

    
    [self.labelContainer addConstraints:@[ CenterX, CenterY, Width ]];
     
     
}

/*
- (instancetype)init
{
    self = [super init];
    if (self) {
        //self.identifier = @"CollectionViewItemReuseIdentifier";
    }
    return self;
}
*/
 
- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Do view setup here.
    self.view.wantsLayer = YES;
    //self.view.layer.cornerRadius = 8.0;
    
    self.view.layer.backgroundColor = [NSColor clearColor].CGColor;
    [self createDeviceImageView];
    [self createDeviceLabel];
    
    [self createGestureRecognizer];
    
}

- (void)setSelected:(BOOL)flag
{
    [super setSelected:flag];
    //[(DVIView*)[self view] setSelected:flag];
    //[(DVIView*)[self view] setNeedsDisplay:YES];
    
    [self.deviceImageView setSelected:flag];
    [self.deviceImageView setNeedsDisplay:YES];
}


- (void)loadView
{
    [self setView:[[DVIView alloc] initWithFrame:NSZeroRect]];
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];
    //[[[self view] button] setTitle:representedObject];
}


- (void)createGestureRecognizer
{
    NSClickGestureRecognizer *clickGesture = [[NSClickGestureRecognizer alloc] initWithTarget:self action:@selector(onClick:)];
    [clickGesture setNumberOfClicksRequired:1];
   // this should be the default, but without setting it, single clicks were delayed until the double click timed-out
   [clickGesture setDelaysPrimaryMouseButtonEvents:FALSE];
   [self.view addGestureRecognizer:clickGesture];
}

- (void)onClick:(NSGestureRecognizer *)sender
{
    // by sending the action to nil, it is passed through the first responder chain
    // to the first object that implements collectionItemViewDoubleClick:
    //[NSApp sendAction:@selector(collectionItemViewDoubleClick:) to:nil from:self];

    if( self.delegate && [self.delegate respondsToSelector:@selector(itemClicked:)])
    {
        [self.delegate itemClicked:self];
    }
    
}

@end
