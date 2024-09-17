//
//  ControlManagerView.m
//  MCUCommander
//
//  Created by Joe Moulton on 12/6/23.
//

#import "ControlManagerView.h"
#import "PbAudioAppInterface.h"

#import <Foundation/NSObjcRuntime.h>
#import <objc/runtime.h>

#import "PbAudioAppDelegate.h"
//#import "CMThruScrollListView.h"

#import "CustomIndicatorView.h"
#import "NSTextFieldButton.h"

//#import "NSCheckboxView.h"

#import "ControlMappingView.h"
#import "DisplayMappingView.h"

@interface ControlManagerView()
{
    
    //CMThruScrollListView * _scrollListView;
    NSSegmentedControl * _radioControl;

    NSTextField *_descriptionLabel;
    NSTextField *_manufacturerLabel;
    NSTextField *_outputDeviceLabel;

    NSMutableDictionary * _inputFields;
    NSMutableDictionary * _fieldButtons;
    
    CocoaTextField *_csvField;
    CocoaTextField *_dirField;
    
    NSButton * _submitButton;
    //NSButton * _deleteButton;
    NSButton * _cancelButton;
    
    volatile bool _userInteractionEnabled;
    
    CMControl * _control;
}


//@property (nonatomic, retain) NSImageView * imageView;
//@property (nonatomic, retain) CMLogoView * imageView;

@property (nonatomic, retain) NSSegmentedControl * radioControl;

//Containers
//@property (nonatomic, retain) NSView * ParameterPanel;
//@property (nonatomic, retain) NSView * ControlPanel;


//Labels
@property (nonatomic, retain) NSTextField * descriptionLabel;

@property (nonatomic, retain) NSTextField * manufacturerLabel;
@property (nonatomic, retain) NSTextField * modelLabel;
@property (nonatomic, retain) NSTextField * outputDeviceLabel;
@property (nonatomic, retain) NSTextField * outputChannelLabel;

@property (nonatomic, retain) NSTextField * nameLabel;
@property (nonatomic, retain) NSTextField * messageLabel;

@property (nonatomic, retain) NSTextField * incrementLabel;

//TextFields
@property (nonatomic, retain) NSTextField      * manufacturerField;
@property (nonatomic, retain) NSTextField      * modelField;
@property (nonatomic, retain) NSPopUpButton    * outputPopupButton;
@property (nonatomic, retain) NSPopUpButton    * outputChannelPopupButton;
@property (nonatomic, retain) NSTextField      * nameField;
@property (nonatomic, retain) NSTextField      * messageField;


//@property (nonatomic, retain) NSCheckboxView * filterView;
@property (nonatomic, retain) NSTextField      * incrementField;

//Toolbar
@property (nonatomic, retain) NSView * bottomToolbar;


@property (nonatomic, retain) NSScrollView * scrollView;
@property (nonatomic, retain) NSView * scrollDocumentView;

@property (nonatomic) int modalViewMode;
@property (nonatomic) int type;

@property (nonatomic, retain) CMSoftThruConnection * dataModel;

@property (atomic) bool userInteractionEnabled;


@property (nonatomic, retain) NSMutableArray * inputFieldKeys;
@property (nonatomic, retain) NSMutableDictionary *inputFields;
@property (nonatomic, retain) NSMutableDictionary *fieldButtons;

@property (nonatomic, retain) NSMutableArray * modalButtons;

@property (nonatomic, retain) NSButton * submitButton;
@property (nonatomic, retain) NSButton * helpButton;

@property (nonatomic, retain) NSButton * cancelButton;

@property (nonatomic, retain) CustomIndicatorView * indicatorView;

@property (nonatomic, retain) NSArray* radioControlViews;
@property (nonatomic, retain) ControlMappingView* controlMappingView;
@property (nonatomic, retain) DisplayMappingView* displayMappingView;


@end

@implementation ControlManagerView

@synthesize descriptionLabel = _descriptionLabel;
@synthesize manufacturerLabel = _manufacturerLabel;
@synthesize outputDeviceLabel = _outputDeviceLabel;

@synthesize inputFields = _inputFields;

@synthesize cancelButton = _cancelButton;

-(void)segmentedControlSelected
{    
    for( int i = 0; i < self.radioControlViews.count; i++) ((NSView*)[self.radioControlViews objectAtIndex:i]).hidden = YES;
    ((NSView*)[self.radioControlViews objectAtIndex:self.radioControl.selectedSegment]).hidden = NO;
}

-(void)createRadioControl
{
    CGFloat controlHeight = 36.;
    
    //self.imageView = [[NSImageView alloc] initWithFrame:CGRectMake(0,0,150, 150)];
    //self.imageView.image = [NSImage imageNamed:NSImageNameNetwork];
    
    
    NSArray *itemArray = [NSArray arrayWithObjects: @"Assignments", @"Display", nil];
    self.radioControl = [NSSegmentedControl segmentedControlWithLabels:itemArray trackingMode:NSSegmentSwitchTrackingSelectOne target:self action:@selector(segmentedControlSelected)];
    //self.radioControl.segmentStyle = NSSegmentedControlStyle
    self.radioControl.selectedSegment = 0;
    //self.radioControl = [[NSSegmentedControl alloc] initWithFrame:CGRectMake(0,0,self.frame.size.width/2., controlHeight)];1
    
    self.radioControl.wantsLayer = YES;
    self.radioControl.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.imageView.layer.cornerRadius = self.imageView.frame.size.width/2.0;
    
    
    [self addSubview:self.radioControl];
    
    
    self.radioControl.translatesAutoresizingMaskIntoConstraints = NO;
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:14];

    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeCenterX
                                                                 multiplier:1.0 constant:0.0f];

    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeWidth
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeWidth
                                                                 multiplier:0.5 constant:0.0f];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeHeight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeHeight
                                                              multiplier:0 constant:controlHeight];
    [self addConstraints:@[ Top, CenterX, Width, Height]];
    
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createDescriptionLabel
{
    self.descriptionLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.descriptionLabel.focusRingType = NSFocusRingTypeDefault;
    self.descriptionLabel.stringValue  = @"Configure a mapping for the selected control.";
    //_descriptionLabel.font = [NSFont systemFontOfSize:36.0];
    self.descriptionLabel.wantsLayer = YES;
    //self.descriptionLabel.bordered = YES;
    
    //self.descriptionLabel.enabled = YES;
    self.descriptionLabel.editable = NO;
    self.descriptionLabel.drawsBackground = YES;
    self.descriptionLabel.layer.masksToBounds = NO;
    
    //self.descriptionLabel.usesSingleLineMode = NO;
    //self.descriptionLabel.maximumNumberOfLines = 1;
    self.descriptionLabel.textColor = NSColor.blackColor;
    self.descriptionLabel.backgroundColor = [NSColor clearColor];
    self.descriptionLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.descriptionLabel.alignment = NSTextAlignmentLeft;
    
    _descriptionLabel.bezeled         = NO;
    _descriptionLabel.editable        = NO;
    _descriptionLabel.drawsBackground = NO;
    
    [self addSubview:self.descriptionLabel];
    

    self.descriptionLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.descriptionLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:14.f];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.descriptionLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.radioControl attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:14.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.descriptionLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self attribute:NSLayoutAttributeWidth
                                                             multiplier:1. constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}

-(void)createRadioControlViews
{
    
    [self createControlMappingView:_control];
    [self createDisplayMappingView];

    self.radioControlViews = [NSArray arrayWithObjects:self.controlMappingView, self.displayMappingView, nil];
    
    [self segmentedControlSelected];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createControlMappingView:(CMControl*)control
{
    self.controlMappingView = [[ControlMappingView alloc] initWithFrame:CGRectZero andControl:control];
    [self addSubview:self.controlMappingView];
    self.controlMappingView.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.controlMappingView
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.controlMappingView
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.controlMappingView
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeTop
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Bottom = [NSLayoutConstraint constraintWithItem: self.controlMappingView
                                                              attribute: NSLayoutAttributeBottom
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.bottomToolbar attribute:NSLayoutAttributeTop
                                                             multiplier:1. constant:0];

    
    [self addConstraints:@[ Left, Right, Top, Bottom ]];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createDisplayMappingView//:(CMControl*)control
{
    self.displayMappingView = [[DisplayMappingView alloc] initWithFrame:CGRectZero];// andControl:control];
    self.displayMappingView.wantsLayer = YES;
    self.displayMappingView.layer.backgroundColor = NSColor.redColor.CGColor;
    [self addSubview:self.displayMappingView];
    self.displayMappingView.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.displayMappingView
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.displayMappingView
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.displayMappingView
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.radioControl attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Bottom = [NSLayoutConstraint constraintWithItem: self.displayMappingView
                                                              attribute: NSLayoutAttributeBottom
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.bottomToolbar attribute:NSLayoutAttributeTop
                                                             multiplier:1. constant:0];

    
    [self addConstraints:@[ Left, Right, Top, Bottom ]];
    self.displayMappingView.hidden = YES;
}



//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void)createManufacturerLabel
{
    self.manufacturerLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.manufacturerLabel.focusRingType = NSFocusRingTypeDefault;
    self.manufacturerLabel.stringValue  = @"Manufacturer:";
    self.manufacturerLabel.wantsLayer = YES;
    
    self.manufacturerLabel.editable = YES;
    self.manufacturerLabel.drawsBackground = YES;
    self.manufacturerLabel.layer.masksToBounds = NO;

    self.manufacturerLabel.textColor = NSColor.blackColor;
    self.manufacturerLabel.backgroundColor = [NSColor clearColor];
    self.manufacturerLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.manufacturerLabel.alignment = NSTextAlignmentLeft;
    
    _manufacturerLabel.bezeled         = NO;
    _manufacturerLabel.editable        = NO;
    _manufacturerLabel.drawsBackground = NO;
    
    [self addSubview:self.manufacturerLabel];
    [self.manufacturerLabel sizeToFit];

    CGRect frame = self.manufacturerLabel.frame;

    self.manufacturerLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.manufacturerLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.manufacturerLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.manufacturerLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.descriptionLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:0 constant:frame.size.width];

    
    [self addConstraints:@[ Left, Top, Width ]];
    
        
    self.manufacturerLabel.stringValue  = @"Manufacturer:";
    //self.manufacturerLabel.frame =  frame;
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void)createModelLabel
{
    self.modelLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.modelLabel.focusRingType = NSFocusRingTypeDefault;
    self.modelLabel.stringValue  = @"Model:";
    self.modelLabel.wantsLayer = YES;
    
    //self.modelLabel.lineBreakMode = NSLineBreakByClipping;
    //self.modelLabel.usesSingleLineMode = YES;
    //self.modelLabel.maximumNumberOfLines = YES;
    
    self.modelLabel.editable = YES;
    self.modelLabel.drawsBackground = YES;
    self.modelLabel.layer.masksToBounds = NO;
    
    self.modelLabel.textColor = NSColor.blackColor;
    self.modelLabel.backgroundColor = [NSColor clearColor];
    self.modelLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.modelLabel.alignment = NSTextAlignmentLeft;

    _modelLabel.bezeled         = NO;
    _modelLabel.editable        = NO;
    _modelLabel.drawsBackground = NO;
    
    [self addSubview:self.modelLabel];
    [self.modelLabel sizeToFit];
    //CGRect frame = self.modelLabel.frame;


    self.modelLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.modelLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.modelLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.manufacturerLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.modelLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.manufacturerLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createOutputDeviceLabel
{
    self.outputDeviceLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.outputDeviceLabel.focusRingType = NSFocusRingTypeDefault;
    self.outputDeviceLabel.stringValue  = @"Midi Out:";
    self.outputDeviceLabel.wantsLayer = YES;
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.outputDeviceLabel.editable = YES;
    self.outputDeviceLabel.drawsBackground = YES;
    self.outputDeviceLabel.layer.masksToBounds = NO;
    
    self.outputDeviceLabel.textColor = NSColor.blackColor;
    self.outputDeviceLabel.backgroundColor = [NSColor clearColor];
    self.outputDeviceLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.outputDeviceLabel.alignment = NSTextAlignmentLeft;

    _outputDeviceLabel.bezeled         = NO;
    _outputDeviceLabel.editable        = NO;
    _outputDeviceLabel.drawsBackground = NO;
    
    [self addSubview:self.outputDeviceLabel];
    [self.outputDeviceLabel sizeToFit];
    //CGRect frame = self.outputDeviceLabel.frame;


    self.outputDeviceLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.modelLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.manufacturerLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createNameLabel
{
    self.nameLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.nameLabel.focusRingType = NSFocusRingTypeDefault;
    self.nameLabel.stringValue  = @"Name:";
    self.nameLabel.wantsLayer = YES;
    
    //self.nameLabel.lineBreakMode = NSLineBreakByClipping;
    //self.nameLabel.usesSingleLineMode = YES;
    //self.nameLabel.maximumNumberOfLines = YES;
    
    self.nameLabel.editable = YES;
    self.nameLabel.drawsBackground = YES;
    self.nameLabel.layer.masksToBounds = NO;
    
    self.nameLabel.textColor = NSColor.blackColor;
    self.nameLabel.backgroundColor = [NSColor clearColor];
    self.nameLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.nameLabel.alignment = NSTextAlignmentLeft;

    _nameLabel.bezeled         = NO;
    _nameLabel.editable        = NO;
    _nameLabel.drawsBackground = NO;
    
    [self addSubview:self.nameLabel];
    [self.nameLabel sizeToFit];
    //CGRect frame = self.nameLabel.frame;


    self.nameLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.nameLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.nameLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.nameLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.manufacturerLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createMessageLabel
{
    self.messageLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.messageLabel.focusRingType = NSFocusRingTypeDefault;
    self.messageLabel.stringValue  = @"Message:";
    self.messageLabel.wantsLayer = YES;
    
    //self.messageLabel.lineBreakMode = NSLineBreakByClipping;
    //self.messageLabel.usesSingleLineMode = YES;
    //self.messageLabel.maximumNumberOfLines = YES;
    
    self.messageLabel.editable = YES;
    self.messageLabel.drawsBackground = YES;
    self.messageLabel.layer.masksToBounds = NO;
    
    self.messageLabel.textColor = NSColor.blackColor;
    self.messageLabel.backgroundColor = [NSColor clearColor];
    self.messageLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.messageLabel.alignment = NSTextAlignmentLeft;

    _messageLabel.bezeled         = NO;
    _messageLabel.editable        = NO;
    _messageLabel.drawsBackground = NO;
    
    [self addSubview:self.messageLabel];
    [self.messageLabel sizeToFit];
    //CGRect frame = self.messageLabel.frame;


    self.messageLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.messageLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.messageLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.nameLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.messageLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.manufacturerLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}


-(void)createNameField
{
    self.nameField = [self createTextField:nil placeholder:@"[Required] Control Mapping Name"];
    [self addSubview:self.nameField];
    [self.nameField sizeToFit];
    

    self.nameField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.nameField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.nameLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.nameField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.nameField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.nameLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


-(void)createMessageField
{
    self.messageField = [self createTextField:@"" placeholder:@"eg F0 E0 A0 B0"];
    [self addSubview:self.messageField];
    [self.messageField sizeToFit];
    

    self.messageField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.messageField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.messageLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.messageField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.messageField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.messageLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}



-(void)devicePopupButtonClicked:(id)sender
{
    //NSLog(@"inputPopupButton select: %@", sender.)
    
    //if the string associated with the selected index does not equal our existing string
    
    assert( self.modalViewMode == ModalView_ControlAssign );
    
    {
        /*
        if( sender == self.inputPopupButton )
        {
            int selectedIndex = (int)self.inputPopupButton.indexOfSelectedItem;
            NSString * inputDeviceName = [NSString stringWithUTF8String:CMClient.sources[selectedIndex].name];
            //if( [_dataModel.Input localizedCompare:inputDeviceName] != NSOrderedSame ) self.submitButton.enabled = YES;

        }
        else*/ if( sender == self.outputPopupButton )
        {
            //int selectedIndex = (int)self.outputPopupButton.indexOfSelectedItem;
            //NSString * deviceName = [NSString stringWithUTF8String:CMClient.destinations[selectedIndex].name];
            //if( [_dataModel.Output localizedCompare:deviceName] != NSOrderedSame ) self.submitButton.enabled = YES;
        }
        
    }
}


-(void)createManufacturerField
{
    self.manufacturerField = [self createTextField:@"" placeholder:@"[Optional] Target Manufacturer Name"];
    [self addSubview:self.manufacturerField];
    [self.manufacturerField sizeToFit];
        

    self.manufacturerField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.manufacturerField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.manufacturerLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.manufacturerField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.manufacturerField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.manufacturerLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}

/*
-(void)createInputPopupButton
{
    self.inputPopupButton = [[NSPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.inputPopupButton.enabled = YES;
    
    [self.inputPopupButton setAction:@selector(devicePopupButtonClicked:)];
    
    int srcIndex = 0;
    for( int i = 0; i < CMClient.numSources; i++)
    {
        [self.inputPopupButton addItemWithTitle:[NSString stringWithUTF8String:CMClient.sources[i].name]];
        //if( strcmp(_dataModel.connection->source.name, CMClient.sources[i].name) == 0 ) srcIndex = i;
    }
    
    if( self.modalViewMode == ModalView_ControlAssign) [self.inputPopupButton selectItemAtIndex:srcIndex];
    [self addSubview:self.inputPopupButton];
    

    self.inputPopupButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.inputPopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.manufacturerLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.inputPopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.inputPopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.manufacturerLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}

*/


-(void)createModelField
{
    self.modelField = [self createTextField:@"" placeholder:@"[Optional] Target Model Name"];
    [self addSubview:self.modelField];
    [self.modelField sizeToFit];
    

    self.modelField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.modelField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.modelLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.modelField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.modelField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.modelLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


-(void)createOutputChannelPopupButton
{
    self.outputChannelPopupButton = [[NSPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.outputChannelPopupButton.enabled = YES;
    [self.outputChannelPopupButton setAction:@selector(devicePopupButtonClicked:)];

    for( int i = 0; i < 16; i++)
    {
        [self.outputChannelPopupButton addItemWithTitle:[NSString stringWithFormat:@"%d", i+1]];
    }
    
    //if( self.modalViewMode == ModalView_ControlAssign) [self.outputPopupButton selectItemAtIndex:dstIndex];
    //1else if( self.outputPopupButton.numberOfItems > 1 ) [self.outputPopupButton selectItemAtIndex:1];

    [self addSubview:self.outputChannelPopupButton];
    

    self.outputChannelPopupButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    /*
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputChannelPopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    */
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.outputChannelPopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.outputChannelPopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Right ]];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createOutputChannelLabel
{
    self.outputChannelLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.outputChannelLabel.focusRingType = NSFocusRingTypeDefault;
    self.outputChannelLabel.stringValue  = @"Channel:";
    self.outputChannelLabel.wantsLayer = YES;
    
    self.outputChannelLabel.editable = YES;
    self.outputChannelLabel.drawsBackground = YES;
    self.outputChannelLabel.layer.masksToBounds = NO;

    self.outputChannelLabel.textColor = NSColor.blackColor;
    self.outputChannelLabel.backgroundColor = [NSColor clearColor];
    self.outputChannelLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.outputChannelLabel.alignment = NSTextAlignmentLeft;
    
    _outputChannelLabel.bezeled         = NO;
    _outputChannelLabel.editable        = NO;
    _outputChannelLabel.drawsBackground = NO;
    
    [self addSubview:self.outputChannelLabel];
    [self.outputChannelLabel sizeToFit];

    CGRect frame = self.outputChannelLabel.frame;

    self.outputChannelLabel.translatesAutoresizingMaskIntoConstraints = NO;
    

    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.outputChannelLabel
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputChannelPopupButton
                                                              attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.outputChannelLabel
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];

    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.outputChannelLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.descriptionLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:0 constant:frame.size.width];

    
    [self addConstraints:@[ Right, CenterY, Width ]];

    self.manufacturerLabel.stringValue  = @"Channel:";
    
}


-(void)createOutputPopupButton
{
    self.outputPopupButton = [[NSPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.outputPopupButton.enabled = YES;
    [self.outputPopupButton setAction:@selector(devicePopupButtonClicked:)];

    
    int dstIndex=0;
    for( int i = 0; i < CMClient.numDestinations; i++)
    {
        [self.outputPopupButton addItemWithTitle:[NSString stringWithUTF8String:CMClient.destinations[i].name]];
        //if(strcmp(_dataModel.connection->destination.name, CMClient.destinations[i].name) == 0 )dstIndex = i;
    }
    
    if( self.modalViewMode == ModalView_ControlAssign) [self.outputPopupButton selectItemAtIndex:dstIndex];
    else if( self.outputPopupButton.numberOfItems > 1 ) [self.outputPopupButton selectItemAtIndex:1];

    [self addSubview:self.outputPopupButton];
    

    self.outputPopupButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputChannelLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


-(void)createIncrementLabel
{
    self.incrementLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.incrementLabel.focusRingType = NSFocusRingTypeDefault;
    self.incrementLabel.stringValue  = @"Shift Increment: ";
    self.incrementLabel.wantsLayer = YES;
    
    self.incrementLabel.editable = YES;
    self.incrementLabel.drawsBackground = YES;
    self.incrementLabel.layer.masksToBounds = NO;
    
    self.incrementLabel.textColor = NSColor.blackColor;
    self.incrementLabel.backgroundColor = [NSColor clearColor];
    self.incrementLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.incrementLabel.alignment = NSTextAlignmentLeft;

    self.incrementLabel.bezeled         = NO;
    self.incrementLabel.editable        = NO;
    self.incrementLabel.drawsBackground = NO;
    
    [self addSubview:self.incrementLabel];
    [self.incrementLabel sizeToFit];

    self.incrementLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.incrementLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.incrementLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.];
    
    /*
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.incrementLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.outputDeviceLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];
     */
    
    [self addConstraints:@[ Left, Top]];
}

-(NSTextField*)createTextField:(NSString*)string placeholder:(NSString*)placeholder
{
  NSTextField* textField = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
  //[textField setCell: [[CMTextFieldCell alloc] init]];

  textField.focusRingType = NSFocusRingTypeDefault;
  textField.stringValue  = @"";//@"TBD";
  textField.wantsLayer = NO;
  textField.placeholderString = placeholder;//@"Enter Value [1-127]";
  textField.editable = YES;
  textField.drawsBackground = NO;
  textField.layer.masksToBounds = NO;
    
  textField.textColor = NSColor.blackColor;
  textField.backgroundColor = [NSColor whiteColor];
  textField.layer.backgroundColor = [NSColor whiteColor].CGColor;
  textField.delegate = self;
  textField.alignment = NSTextAlignmentLeft;

  textField.bezeled         = YES;
  textField.editable        = YES;
  textField.drawsBackground = NO;
    
   
    [textField sizeToFit];

    return textField;
}

-(void)createIncrementField
{
    self.incrementField = [self createTextField:@"" placeholder:@"Enter Value [1-127]"];//[[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    [self addSubview:self.incrementField];
    [self.incrementField sizeToFit];

    self.incrementField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.incrementField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.incrementLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.incrementField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.incrementField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.incrementLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
    
    self.incrementField.enabled = YES;
}

/*
-(void)buttonToggledAtIndex:(int)index;
{
    NSLog(@"CMThruModalView::buttonToggledAtIndex(%d)", index);
    if( self.modalViewMode == ModalView_Modify ) self.submitButton.enabled = YES;
}

-(void) createFilterView
{
    NSArray * filterIdentifiers = @[@"CC", @"SysEx", @"MTC", @"Beat Clock", @"Tune Request"];
    self.filterView = [[NSCheckboxView alloc] initWithIdentifiers:filterIdentifiers Title:@"Filter:  " Justification:NSTextAlignmentRight];
    self.filterView.delegate = self;
    
    if( self.modalViewMode == ModalView_Modify )
    {
        //for( NSButton * button in self.filterView.buttons ) button.enabled = NO;
        if( _dataModel.connection->params.filterOutAllControls ) ((NSButton*)[self.filterView.buttons objectAtIndex:0]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutSysEx ) ((NSButton*)[self.filterView.buttons objectAtIndex:1]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutMTC ) ((NSButton*)[self.filterView.buttons objectAtIndex:2]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutBeatClock ) ((NSButton*)[self.filterView.buttons objectAtIndex:3]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutTuneRequest ) ((NSButton*)[self.filterView.buttons objectAtIndex:4]).state = NSControlStateValueOn;
    }
    
    [self addSubview:self.filterView];
    self.filterView.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputPopupButton attribute:NSLayoutAttributeHeight
                                                               multiplier:1.0 constant:0.f];

    
    [self addConstraints:@[ Top, Left, Right, Height ]];
    
}
*/

-(void)createBottomToolbar
{
    
    self.bottomToolbar = [[NSView alloc] initWithFrame:CGRectMake(0,0, self.frame.size.width, 50)];
    
    self.bottomToolbar.wantsLayer = YES;
    self.bottomToolbar.layer.backgroundColor = [NSColor colorWithWhite:1.0 alpha:0.2].CGColor;
    
    //self.bottomToolbar.superview.wantsLayer = true
    self.bottomToolbar.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.bottomToolbar.layer.shadowOpacity = 1.0;
    self.bottomToolbar.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.bottomToolbar.layer.shadowOffset = NSMakeSize(0, 0);
    self.bottomToolbar.layer.shadowRadius = 10;

    self.bottomToolbar.layer.borderWidth = 0.25;
    self.bottomToolbar.layer.borderColor = [NSColor colorWithWhite:0.7 alpha:1.0].CGColor;
    
    [self addSubview:self.bottomToolbar];
    
    //self.bottomToolbar.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Bottom = [NSLayoutConstraint constraintWithItem: self.bottomToolbar
                                                                attribute: NSLayoutAttributeBottom
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeBottom
                                                               multiplier:1 constant:0];
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.bottomToolbar
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];

    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.bottomToolbar
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0];

  

    
    [self addConstraints:@[ Bottom, Left, Right ]];
}


-(void)createSubmitButton
{
    self.submitButton = [[NSButton alloc] initWithFrame:CGRectMake(0,0,80,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    self.submitButton.enabled = NO;
    
    [self.submitButton setTitle:@"Apply"];
    [self.submitButton setTarget:self];
    [self.submitButton setAction:@selector(submitButtonClicked:)];
    
    self.submitButton.wantsLayer = YES;
    self.submitButton.layer.backgroundColor = NSColor.clearColor.CGColor;
    self.submitButton.layer.borderColor = NSColor.clearColor.CGColor;
    self.submitButton.layer.borderWidth = 0;
    //self.submitButton.layer.cornerRadius = 4.0f;

    self.submitButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.submitButton.layer.shadowOpacity = 1.0;
    self.submitButton.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.submitButton.layer.shadowOffset = NSMakeSize(0, 0.5);
    self.submitButton.layer.shadowRadius = 0.5;

    
    self.submitButton.contentTintColor = [NSColor colorWithWhite:0.9 alpha:1.0];
    self.submitButton.bezelStyle = NSBezelStyleRounded;
    
    //self.layer.backgroundColor =
    self.submitButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.bottomToolbar addSubview:self.submitButton];
    
    self.submitButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.submitButton
                                                           attribute: NSLayoutAttributeRight
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self.bottomToolbar attribute:NSLayoutAttributeRight
                                                          multiplier:1.0f constant:-16.0f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.submitButton
                                                            attribute: NSLayoutAttributeCenterY
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.bottomToolbar attribute:NSLayoutAttributeCenterY
                                                           multiplier:1.0 constant:0];
    
    
    [self addConstraints:@[ Right, CenterY]];
    
}



-(void)createHelpButton
{
    self.helpButton = [[NSButton alloc] initWithFrame:CGRectMake(0,0,30,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    [self.helpButton setTitle:@""];
    [self.helpButton setTarget:self];
    [self.helpButton setAction:@selector(helpButtonClicked:)];
    
    self.helpButton.wantsLayer = YES;
    self.helpButton.layer.backgroundColor = NSColor.clearColor.CGColor;
    self.helpButton.layer.borderColor = NSColor.clearColor.CGColor;
    self.helpButton.layer.borderWidth = 0;
    //self.submitButton.layer.cornerRadius = 4.0f;

    self.helpButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.helpButton.layer.shadowOpacity = 1.0;
    self.helpButton.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.helpButton.layer.shadowOffset = NSMakeSize(0, 0.5);
    self.helpButton.layer.shadowRadius = 0.5;

    
    self.helpButton.contentTintColor = [NSColor colorWithWhite:0.9 alpha:1.0];
    self.helpButton.bezelStyle = NSBezelStyleHelpButton;
    
    //self.layer.backgroundColor =
    self.helpButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.bottomToolbar addSubview:self.helpButton];
    
    self.helpButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.helpButton
                                                           attribute: NSLayoutAttributeLeft
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self.bottomToolbar attribute:NSLayoutAttributeLeft
                                                          multiplier:1.0f constant:16.0f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.helpButton
                                                            attribute: NSLayoutAttributeCenterY
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.bottomToolbar attribute:NSLayoutAttributeCenterY
                                                           multiplier:1.0 constant:0];
    
    [self addConstraints:@[ Left, CenterY]];
    
}

-(id)initWithFrame:(NSRect)frame andControl:(nonnull CMControl *)control
{
    self = [super initWithFrame:frame];
    if(self)
    {
        
        
        _control = control;
        self.wantsLayer = YES;
    
        //[self setWantsLayer:NO];
        //self.layer.borderWidth = 0.0f;
        //self.layer.masksToBounds = YES;
        //[self.layer setBorderWidth:0];
        //self.appearance = [NSAppearance appearanceNamed: NSAppearanceNameDarkAqua];

        //create a temporary data model object that the modal table view can use to populate itself and visa versa
        //so the user can create a thru coonnection
        self.modalViewMode = ModalView_ControlAssign;
        self.dataModel = CMSoftThru;
        /*
        if( thruID && thruID.length > 0 )
        {
            self.dataModel = [CMThru.dictionary objectForKey:thruID];
            self.modalViewMode = ModalView_Modify;
            assert(self.dataModel);
        }
        else
        {
            self.dataModel = [[CMThruConnection alloc] init];
        }
        */
        
        //[self createScrollListView];
        //[self createScrollView];
        
        //create UI
        
        //[self createRadioControl];
        //[self createDisplayMappingView];

        /*
        [self createDescriptionLabel];
        
        [self createManufacturerLabel];
        [self createModelLabel];
        [self createOutputDeviceLabel];
            
        //[self createIncrementLabel];
        
        //[self createInputPopupButton];
        [self createManufacturerField];
        [self createModelField];

        
        [self createOutputChannelPopupButton];
        [self createOutputChannelLabel];

        [self createOutputPopupButton];
        
        [self createNameLabel];
        [self createMessageLabel];
        //[self createIncrementField];
        
        [self createNameField];
        [self createMessageField];
         */
        
        [self createBottomToolbar];
        //[self createSubmitButton];
        //[self createHelpButton];
        
        //[self createRadioControlViews];
        [self createControlMappingView:control];
        
        //Updates to determine whether to activate submit button
        //[self devicePopupButtonClicked:self.inputPopupButton];
        //[self devicePopupButtonClicked:self.outputPopupButton];
        
        
        
        self.userInteractionEnabled = true;

       /*
        NSLayoutConstraint * documentTop = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                        attribute: NSLayoutAttributeTop
                                                                        relatedBy:NSLayoutRelationEqual
                                                                           toItem:self.scrollView
                                                                        attribute:NSLayoutAttributeTop
                                                                       multiplier:1.0f constant:0];
        
        NSLayoutConstraint * documentBottom = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                           attribute: NSLayoutAttributeBottom
                                                                           relatedBy:NSLayoutRelationEqual
                                                                              toItem:self.scrollView
                                                                           attribute:NSLayoutAttributeBottom
                                                                          multiplier:1.0f constant:0];
        
        NSLayoutConstraint * documentLeft = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                         attribute: NSLayoutAttributeLeft
                                                                         relatedBy:NSLayoutRelationEqual
                                                                            toItem:self.scrollView
                                                                         attribute:NSLayoutAttributeLeft
                                                                        multiplier:1.0f constant:0.f];
        
        NSLayoutConstraint * documentRight = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                          attribute: NSLayoutAttributeRight
                                                                          relatedBy:NSLayoutRelationEqual
                                                                             toItem:self.scrollView
                                                                          attribute:NSLayoutAttributeRight
                                                                         multiplier:1.0f constant:0.0f];
        
        
        [self.scrollView addConstraints:@[documentTop, documentLeft, documentBottom, documentRight]];
        
    
        [self updateDocumentView];
        [_descriptionLabel setNeedsLayout:YES];
        [_descriptionLabel setNeedsDisplay:YES];
        */
    }
    
    return self;
}


-(void)scrollToBottom
{
    
    
}


-(void)createScrollView
{
    
    self.scrollView = [[NSScrollView alloc] initWithFrame:CGRectMake(0,0,100,100)];
    self.scrollView.backgroundColor = [NSColor colorWithWhite:233./255. alpha:1.];
    [self addSubview:self.scrollView];
    self.scrollView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeCenterX
                                                                multiplier:1 constant:0];

    NSLayoutConstraint * right = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1.0f constant:0];

    NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:0.0f];

    NSLayoutConstraint * bottom = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeBottom
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0f constant:0.0f];
    
    
    [self addConstraints:@[CenterX, right, top, bottom]];
    
    
    
    self.scrollDocumentView = [[NSView alloc] initWithFrame:CGRectMake(0,0,200,200)];
    self.scrollDocumentView.wantsLayer = true;
    self.scrollDocumentView.layer.backgroundColor = [NSColor redColor].CGColor;
    [self.scrollView setDocumentView:self.scrollDocumentView];
    self.scrollDocumentView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * sCenterX = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeCenterX
                                                                multiplier:1.f constant:0];

    NSLayoutConstraint * sright = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1.0f constant:0];

    NSLayoutConstraint * stop = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:0.0f];

    NSLayoutConstraint * sbottom = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeBottom
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0f constant:0.0f];
    
    
    [self.scrollView addConstraints:@[stop, sCenterX, sbottom, sright]];
}



-(void)updateDocumentView
{
    [self layoutSubtreeIfNeeded];
    
    /*
    CGRect frame = self.scrollDocumentView.frame;
    self.scrollDocumentView.frame = CGRectMake(0,0,frame.size.width, 1000);

    CGFloat documentViewOffset = (self.cancelButton.frame.origin.y - self.frame.origin.y + self.cancelButton.frame.size.height - (self.frame.size.height) );
    if( documentViewOffset < 0 )
    documentViewOffset = 0;
    
    NSLayoutConstraint * documentTop = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.scrollView attribute:NSLayoutAttributeTop
                                                           multiplier:0.5 constant:0.0f];
     
     NSLayoutConstraint * documentLeft = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                              attribute: NSLayoutAttributeLeft
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.scrollView attribute:NSLayoutAttributeLeft
                                                             multiplier:1 constant:0.0f];



    NSLayoutConstraint * documentBottom = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                            attribute: NSLayoutAttributeBottom
                                                            relatedBy:NSLayoutRelationEqual
                                                              toItem: self.cancelButton attribute:NSLayoutAttributeBottom
                                                                multiplier:1 constant:20];

    NSLayoutConstraint * documentRight = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                                                        attribute: NSLayoutAttributeRight
                                                                                        relatedBy:NSLayoutRelationEqual
                                                                                           toItem: self.scrollView attribute:NSLayoutAttributeRight
                                                                                       multiplier:1 constant:0.0f];
                               
    [self.scrollView addConstraints:@[ documentTop, documentLeft, documentBottom, documentRight ]];
     */
}


-(NSTextFieldButton*)createUserInputButtonForField:(NSTextField*)textField withTitle:(NSString*)title
{
    //CGFloat buttonWidth = 200;
    //CGFloat buttonHeight = 100;
    //CGFloat buttonX = self.view.frame.size.width/2 - buttonWidth/2;
    //CGFloat buttonY = self.view.frame.size.height/2 - buttonHeight/2;
    NSTextFieldButton * button = [[NSTextFieldButton alloc] initWithFrame:CGRectMake(0,0,200,100)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    
    textField.enabled = NO;
    
    button.textfield = textField;
    button.chooseDir = NO;
    button.chooseFiles = YES;
    [button setAction:@selector(textFieldButtonClicked:)];

    [button setTitle:title];
    [button setTarget:self];
    //[button setAction:@selector(csvButtonClicked:)];
    
    button.translatesAutoresizingMaskIntoConstraints = NO;
    [self.scrollDocumentView addSubview:button];
    
    //[self addLoginButtonConstraintsToItem:self fromItem:_loginButton];
    
    NSLayoutConstraint * cy = [NSLayoutConstraint constraintWithItem: button
                                                           attribute: NSLayoutAttributeCenterY
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: textField attribute:NSLayoutAttributeCenterY
                                                          multiplier:1.0f constant:0.0f];
    
    NSLayoutConstraint * left = [NSLayoutConstraint constraintWithItem: button
                                                             attribute: NSLayoutAttributeLeft
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem: textField attribute:NSLayoutAttributeRight
                                                            multiplier:1.0f constant:10.0f];
    
    /*
     NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem: _csvButton
     attribute: NSLayoutAttributeTop
     relatedBy:NSLayoutRelationEqual
     toItem: _pwField attribute:NSLayoutAttributeBottom
     multiplier:1.0 constant:20];
     */
    /*
     NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: _csvButton
     attribute: NSLayoutAttributeWidth
     relatedBy:NSLayoutRelationEqual
     toItem: self attribute:NSLayoutAttributeWidth
     multiplier:.015 constant:0];
     
     NSLayoutConstraint * height = [NSLayoutConstraint constraintWithItem: _csvButton
     attribute: NSLayoutAttributeHeight
     relatedBy:NSLayoutRelationEqual
     toItem: _nameField attribute:NSLayoutAttributeHeight
     multiplier:1 constant:0 ];
     */
    [self.scrollDocumentView addConstraints:@[ cy, left ]];
    
    return button;
    
}

/*
- (id)initWithFrame:(NSRect)frame andDataModel:(VRTDOM*)dataModel{
    self = [super initWithFrame:frame];
    if (self) {

        self.modalViewMode = ModalView_Modify;
        self.dataModel = dataModel;
        
        [self createTitleLabel];
        
        [self createUserInputFields];
        [self createModalButtons];
        //[self createLoginButton];
        
        //self.layer.backgroundColor = [[NSColor blueColor] CGColor];
        [self createActivityIndicator];
        
    }
    return self;
}

*/

/*
-(void)createActivityIndicator
{
    
    if( !self.indicatorView )
    {
        self.indicatorView = [[CustomIndicatorView alloc] init];
        _indicatorView.layer.cornerRadius = 6.0;
        
        [self addSubview:_indicatorView];
        
        _indicatorView.translatesAutoresizingMaskIntoConstraints = NO;
        //[self addIndicatorViewConstraintsToItem:_loginButtonContainer fromItem:_indicatorView ];
        
        NSLayoutConstraint * centerX = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                    attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem: self attribute:NSLayoutAttributeCenterX
                                                                   multiplier:1.0f constant:0.0f];
        
        
        NSLayoutConstraint * centerY = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                    attribute: NSLayoutAttributeCenterY
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem: self attribute:NSLayoutAttributeCenterY
                                                                   multiplier:1.0f constant:0.0f];
        
        
        NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                  attribute: NSLayoutAttributeWidth
                                                                  relatedBy:NSLayoutRelationEqual
                                                                     toItem: self attribute:NSLayoutAttributeWidth
                                                                 multiplier:0 constant:150];
        
        NSLayoutConstraint * height = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                   attribute: NSLayoutAttributeHeight
                                                                   relatedBy:NSLayoutRelationEqual
                                                                      toItem: self attribute:NSLayoutAttributeHeight
                                                                  multiplier:0 constant:100];//22.0f * [[UIScreen mainScreen] scale]];
        
        [self addConstraints:@[ centerX, centerY, width, height ]];
        
        
        _indicatorView.label.stringValue = @"Logging In";
        //_indicatorView.label.font = [[[MastryFontManager sharedInstance] buttonThemeBoldFont] fontWithSize:36.0];
        //_indicatorView.label.minimumScaleFactor = 0.1;
        
        //_indicatorView.label.numberOfLines=0;
        //_indicatorView.label.lineBreakMode=NSLineBreakByWordWrapping;
        //_indicatorView.label.adjustsFontSizeToFitWidth=YES;
        _indicatorView.hidden = YES;
        
        //[_indicatorView showAndStartAnimating];
        
    }
    
    
}
 */

-(void)dealloc
{
    NSLog(@"MastryDOMView dealloc");

}



-(void) addTitleLabelConstraintsToItem:(NSView*)toItem fromItem:(NSView*)fromItem
{
    
    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem: fromItem
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: toItem attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0.0f];
    
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: fromItem
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: toItem attribute:NSLayoutAttributeBottom
                                                               multiplier:0.10 constant:0];
    
    NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: fromItem
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: toItem attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [toItem addConstraints:@[ CenterY, width, CenterX ]];
    
}

-(NSTextField*)createTextFieldWithText:(NSString *)text placeholderText:(NSString*)placeholderText prevTextField:(NSView*)prevTextField
{
    
    NSTextField * textField = [[NSTextField alloc] initWithFrame:CGRectMake(0,0,60,100)];
    [textField setWantsLayer:YES];
    textField.layer.cornerRadius = 3.0;
    textField.layer.backgroundColor = [NSColor blueColor].CGColor;

    [textField setBezeled:YES];
    [textField setDrawsBackground:YES];
    [textField setEditable:YES];
    [textField setSelectable:YES];
    
    //textField = UITextBorderStyleNone;
    //textField = [UIFont systemFont];
    //textField = @"user@spur.com";
    
    //textField.stringValue = @"Hello";
    if( placeholderText )
        textField.placeholderString = placeholderText;
    
    if( text )
        textField.stringValue = text;
    textField.textColor = [NSColor darkGrayColor];
    textField.delegate = self;
    
    [self.scrollDocumentView addSubview:textField];
    
    
    textField.translatesAutoresizingMaskIntoConstraints = NO;

    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem: textField
                                                                attribute: NSLayoutAttributeCenterX
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.scrollDocumentView attribute:NSLayoutAttributeCenterX
                                                               multiplier:1.f constant:0.0f];
    
    
    NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem: textField
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: prevTextField attribute:NSLayoutAttributeBottom
                                                           multiplier:1.0 constant:0];
    
    NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: textField
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.scrollDocumentView attribute:NSLayoutAttributeWidth
                                                             multiplier:0.4 constant:0];
    

    [self.scrollDocumentView addConstraints:@[ CenterX, width, top ]];
    
    
    textField.hidden = NO;
    [textField setNeedsDisplay:YES];
    [textField setNeedsLayout:YES];
    
    return textField;
    
}

- (NSView *)hitTest:(NSPoint)aPoint
{
    if( self.userInteractionEnabled )
        return [super hitTest:aPoint];
    else
        return nil;
}

-(void)textFieldButtonClicked:(id)sender
{
    NSTextFieldButton * button = (NSTextFieldButton *)sender;
    
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setLevel:NSPopUpMenuWindowLevel];
    [openDlg setAllowsMultipleSelection:NO];
    
    [openDlg setCanChooseDirectories:button.chooseDir];
    [openDlg setCanChooseFiles:button.chooseFiles];
    //[openDlg setCanCreateDirectories:NO];
    
    [openDlg setPrompt:@"Select"];
    
    //NSArray* imageTypes = [NSImage imageTypes];
    
    if( button.filetypes && button.filetypes.count > 0 )
        [openDlg setAllowedContentTypes:button.filetypes];
    
    
    self.userInteractionEnabled = false;
    
    __weak typeof(self) weakSelf = self;
    
    [openDlg beginWithCompletionHandler:^(NSInteger result){
        
        //if( result == NSModalResponseOK )
        //{
        //get the URL of the selected file
        NSArray* pathURLs = [openDlg URLs];
        if( pathURLs && pathURLs.count > 0 )
        {
            NSURL * pathURL = [pathURLs objectAtIndex:0];
            
            //NSArray* filepaths = [openDlg filenames];
            //get the filepath of the selected file and convert back to url
            NSString * filepath = pathURL.path;//[filepaths objectAtIndex:0];
            //NSURL * url = [[NSURL alloc] initFileURLWithPath:filepath];
        
            if( filepath )
            {
                //NSTextField * textField = (NSTextField*) [self.inputFields objectForKey:@"URL"];
                button.textfield.stringValue = filepath;
            }
            else
            {
                [self showAlertView:@"Error" message:@"Failed to get local URL to selected file." completionHandler:nil];
            }
        }
        
        //return user interaction to this view
        //dispatch_async(dispatch_get_main_queue(), ^{
            weakSelf.userInteractionEnabled = true;
        //});
        
    }];

}

-(void)createUserInputFields
{
    //loop over each property and create and input field
    unsigned int count = 0;
    //NSMutableDictionary *dictionary = [NSMutableDictionary new];
    self.inputFields = [NSMutableDictionary new];
    self.inputFieldKeys = [NSMutableArray new];
    self.fieldButtons = [NSMutableDictionary new];
    
    if( self.modalViewMode == ModalView_ControlAssign  )//|| self.modalViewMode == ModalView_Modify)
    {
        objc_property_t *properties = nil;//class_copyPropertyList([CMThruConnection class], &count);
        
        NSView * prevTextField = self.scrollDocumentView;
        for (int i = 0; i < count; i++) {
            
            NSString *key = [NSString stringWithUTF8String:property_getName(properties[i])];
            //NSObject * value = [self valueForKey:key];
            
            NSLog(@"Input Field Key = %@", key);
            //if( value && [value isKindOfClass:[NSString class]] )
            // {
                //create a text field
                id text = nil;
                //if( self.modalViewMode == ModalView_Modify ) text = [self.dataModel valueForKey:key];
            
                NSTextField * textField = [self createTextFieldWithText:text placeholderText:key prevTextField:prevTextField];
                //textField.delegate = self;
                prevTextField = textField;
                [self.inputFields setObject:textField forKey:key];
                [self.inputFieldKeys addObject:key];
            
                if( [key localizedCompare:@"URL"] == NSOrderedSame)
                {
                    NSTextFieldButton * button = [self createUserInputButtonForField:textField withTitle:@"Choose..."];
                    button.filetypes = @[@"png", @"PNG"];
                }
            //}
            //NSString *value = [NSString stringWithFormat:@"%@",[self valueForKey:key]];
            //[dictionary setObject:value forKey:key];
            //id value = [self valueForKey:key];
            /*
             if (value == nil) {
             // nothing todo
             }
             else if ([value isKindOfClass:[NSNumber class]]
             || [value isKindOfClass:[NSString class]]
             || [value isKindOfClass:[NSDictionary class]] || [value isKindOfClass:[NSMutableArray class]]) {
             // TODO: extend to other types
             [dictionary setObject:value forKey:key];
             }
             else if ([value isKindOfClass:[NSObject class]]) {
             [dictionary setObject:[value dictionary] forKey:key];
             }
             else {
             NSLog(@"Invalid type for %@ (%@)", NSStringFromClass([self class]), key);
             }
             */
        }
        free(properties);
        
    }
    /*
    else if( self.modalViewMode == ModalView_Create_MANY)
    {
        
        //NSArray * fieldTitleStrings = [[NSArray alloc] initWithObjects:@"Path ]
        //NSArray * fieldKeys = [[NSArray alloc] initWithObjects:@"CSV", "DIR", nil];
        NSArray * fieldKeys = [[NSArray alloc] initWithObjects:@"Path to CSV", @"Path to Image Directory", nil];

        //NSArray *
        
        NSView * prevTextField = _descriptionLabel;
        for (int i = 0; i < fieldKeys.count; i++) {
            
            NSString * key = [fieldKeys objectAtIndex:i];
            NSTextField * textField = [self createTextFieldWithText:nil placeholderText:key prevTextField:prevTextField];
            NSTextFieldButton * button = [self createUserInputButtonForField:textField withTitle:@"Choose..."];
            
            if( i == 0 )
                
            {
                _csvField = textField;
                button.chooseDir = NO;
                button.chooseFiles = YES;
                button.filetypes = @[@"csv", @"CSV"];
            }
            else
            {
                _dirField = textField;
                button.chooseDir = YES;
                button.chooseFiles = NO;
            }
            
            //textField.delegate = self;
            prevTextField = textField;
            [self.inputFields setObject:textField forKey:key];
            [self.inputFieldKeys addObject:key];
            
        }


    }
     */
    //return dictionary;
    
}

-(NSButton *)createModalButton:(NSString *)title topView:(NSView*)topView selector:(SEL)selector
{
    //CGFloat buttonWidth = 200;
    //CGFloat buttonHeight = 100;
    //CGFloat buttonX = self.view.frame.size.width/2 - buttonWidth/2;
    //CGFloat buttonY = self.view.frame.size.height/2 - buttonHeight/2;
    NSButton * button = [[NSButton alloc] initWithFrame:CGRectMake(0,0,0,100)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    [button setTitle:title];
    [button setTarget:self];
    [button setAction:selector];
    
    button.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:button];
    
    //[self addLoginButtonConstraintsToItem:self fromItem:_loginButton];
    
    NSLayoutConstraint * cx = [NSLayoutConstraint constraintWithItem: button
                                                           attribute: NSLayoutAttributeCenterX
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self attribute:NSLayoutAttributeCenterX
                                                          multiplier:1.0f constant:0.0f];
    
    NSLayoutConstraint * bottom = [NSLayoutConstraint constraintWithItem: button
                                                            attribute: NSLayoutAttributeBottom
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self attribute:NSLayoutAttributeBottom
                                                           multiplier:1.0 constant:20];
    
    NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: button
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self attribute:NSLayoutAttributeWidth
                                                             multiplier:0.25 constant:0];
    
    
    [self addConstraints:@[ cx, width, bottom ]];
    
    return button;
    
}

/*
-(const char *)getCollectionTitle
{
    const char * collectionTitle = nil;
    if( self.dataModel.type > NUM_DOM_COLLECTIONS)
        collectionTitle = VRT_EXT_DOM_COLLECTION_TITLES[self.dataModel.type - NUM_DOM_COLLECTIONS];
    else
        collectionTitle = VRT_DOM_COLLECTION_TITLES[self.dataModel.type];
    return collectionTitle;
}

-(const char *)getDOMTitle
{
    const char * collectionTitle = nil;
    if( self.dataModel.type > NUM_DOM_COLLECTIONS)
        collectionTitle = VRT_EXT_DOM_TITLES[self.dataModel.type - NUM_DOM_COLLECTIONS];
    else
        collectionTitle = VRT_DOM_TITLES[self.dataModel.type];
    return collectionTitle;
}
*/

- (void)controlTextDidChange:(NSNotification *)notification {
    NSTextField *textField = [notification object];
    if( textField.stringValue.length > 0 )
        _submitButton.enabled = YES;
    else
        _submitButton.enabled = NO;
}

- (void)controlTextDidEndEditing:(NSNotification *)notification {
    //NSTextField *textField = [notification object];
    //if ([textField resignFirstResponder]) {
    //    textField.textColor = [NSColor blackColor];
    //}
}

-(void)helpButtonClicked:(id)sender
{
    
}

-(void)submitButtonClicked:(id)sender
{
    //[self.indicatorView showAndStartAnimating];
    
    if( self.modalViewMode == ModalView_ControlAssign  )
    {
        //The submit button should not have been activated unless fields have been populated
        //TO DO: check ThruID for name collisions
        
        //TO DO:  make these asserts user alerts
        //assert(self.thruIDField.stringValue.length > 0);
        //assert(self.inputPopupButton.indexOfSelectedItem != self.outputPopupButton.indexOfSelectedItem);
        
        //TO DO:  check the field value for sanity
        //int inputIDs[1] =  {(int)self.inputPopupButton.indexOfSelectedItem};
        //int outputIDs[1] = {(int)self.outputPopupButton.indexOfSelectedItem};
        
        //[[MCUApplicationDelegate sharedInstance] dismissModalWindow];
        
    }
    else if( self.modalViewMode == ModalView_ControlAssign )
    {
        //TO DO:  make these asserts user alerts
        //assert(self.thruIDField.stringValue.length > 0);
       // assert(self.inputPopupButton.indexOfSelectedItem != self.outputPopupButton.indexOfSelectedItem);
        
        /*
        //TO DO:  check the field value for sanity
        int inputIDs[1] =  {(int)self.inputPopupButton.indexOfSelectedItem};
        int outputIDs[1] = {(int)self.outputPopupButton.indexOfSelectedItem};
        
        //Populate thruParam Inputs/Outputs
        MIDIThruConnectionParams* thruParams = &(_dataModel.connection->params);
        CMInitThruParamEndpoints(thruParams, inputIDs, 1, outputIDs, 1);

        //NSArray * filterIdentifiers = @[@"CC", @"SysEx", @"MTC", @"Beat Clock", @"Tune Request"];

        //Populate thruParam Filter Section
        for( int i = 0; i<self.filterView.buttons.count; i++)
        {
            NSButton * button = [self.filterView.buttons objectAtIndex:i];
        
            if( button.state == NSControlStateValueOn)
            {
                if( [button.title localizedCompare:@"CC"] == NSOrderedSame )
                    thruParams->filterOutAllControls = 1;
                else if( [button.title localizedCompare:@"SysEx"] == NSOrderedSame )
                    thruParams->filterOutSysEx = 1;
                else if( [button.title localizedCompare:@"MTC"] == NSOrderedSame )
                    thruParams->filterOutMTC = 1;
                else if( [button.title localizedCompare:@"Beat Clock"] == NSOrderedSame )
                    thruParams->filterOutBeatClock = 1;
                else if( [button.title localizedCompare:@"Tune Request"] == NSOrderedSame )
                    thruParams->filterOutTuneRequest = 1;
            }
            else if( button.state == NSControlStateValueOff)
            {
                if( [button.title localizedCompare:@"CC"] == NSOrderedSame )
                    thruParams->filterOutAllControls = 0;
                else if( [button.title localizedCompare:@"SysEx"] == NSOrderedSame )
                    thruParams->filterOutSysEx = 0;
                else if( [button.title localizedCompare:@"MTC"] == NSOrderedSame )
                    thruParams->filterOutMTC = 0;
                else if( [button.title localizedCompare:@"Beat Clock"] == NSOrderedSame )
                    thruParams->filterOutBeatClock = 0;
                else if( [button.title localizedCompare:@"Tune Request"] == NSOrderedSame )
                    thruParams->filterOutTuneRequest = 0;
            }
        }
        
        //if the ThruID changed we need to delete the old connection then create a new one
        if( [_dataModel.ThruID localizedCompare:self.thruIDField.stringValue] != NSOrderedSame )
        {
            //TO DO: present an alert to the user
            if([CMThru.dictionary objectForKey:self.thruIDField.stringValue]) assert(1==0);

            [_dataModel replaceThruConnection:self.thruIDField.stringValue atIndex:_dataModel.row];
        }
        else //but otherwise we just need to update the params on the CoreMIDI thru connection
        {
            [_dataModel saveThruConnection];
        }
        */
        //[[PbAudioAppDelegate sharedInstance] dismissModalWindow];
        
    }
        

    /*
    else if( self.modalViewMode == ModalView_Create_MANY )
    {
        //TO DO:  verify csv and dir field
        self.indicatorView.label.stringValue = @"Parsing CSV";
        
        //parse csv to create array of nsdictionaries synchronously
        //it will display an error message to the user for us if needed
        NSArray * newEntries = [self.dataModel parseCSV:_csvField.stringValue assetDir:_dirField.stringValue];
        
        //there were errors parsing the csv
        if( !newEntries || newEntries.count < 1)
        {
            [self.indicatorView hideAndStopAnimating];
            return;
        }
        
        self.indicatorView.label.stringValue = @"Uploading Documents";
        HTTPRequestCompletionBlock createManyCompletionBlock = ^void(long statusCode, NSError * error, NSArray * documents)
        {
            //Since our implementation is abstract, only the VRTDOM child class implementations know how to interpret the response data for their POST requests
            //We use a delegate
            dispatch_async(dispatch_get_main_queue(), ^{
                [self.indicatorView hideAndStopAnimating];
            });
        };
        
        //[self.dataModel submitCreateManyRequestWithCSV:_csvField.stringValue andAssetDir:_dirField.stringValue];
        //[self.dataModel submitCreateManyRequestWithDicts:newEntries andCompletionHandler:createManyCompletionBlock];
    }
     */
}

-(void)cancelButtonClicked:(id)sender
{
    NSLog(@"cancelButtonClicked");
    //MCUApplicationDelegate *appDelegate = (MCUApplicationDelegate *)[[NSApplication sharedApplication] delegate];
    //[appDelegate dismissModalWindow];
}

-(void)createModalButtons
{
    //everybody gets a submit button
    
    NSView * topView = nil;//'_scrollListView;//[_inputFields objectForKey:[_inputFieldKeys objectAtIndex:_inputFieldKeys.count -1]];
    self.submitButton = [self createModalButton:@"Submit" topView:topView selector:@selector(submitButtonClicked:)];
    self.cancelButton = [self createModalButton:@"Cancel" topView:self.submitButton selector:@selector(cancelButtonClicked:)];

    self.modalButtons = [NSMutableArray new];
    [self.modalButtons addObject:self.submitButton];
    [self.modalButtons addObject:self.cancelButton];

    /*)
    NSLayoutConstraint * bottom = [NSLayoutConstraint constraintWithItem: self.scrollView
                                                              attribute: NSLayoutAttributeBottom
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.cancelButton attribute:NSLayoutAttributeBottom
                                                             multiplier:1 constant:0];
    
    */
    //[self addConstraints:@[ bottom ]];

}

- (void)processAlertReturnCode:(NSInteger)returnCode
{
    if (returnCode == NSModalResponseOK )//|| returnCode == NSOKButton)
    {
        NSLog(@"(returnCode == NSOKButton)");
        //[self.window makeKeyWindow];
    }
    else if (returnCode == NSModalResponseCancel)// || returnCode == NSCancelButton)
    {
        NSLog(@"(returnCode == NSCancelButton)");
        
        //MastryAdminAppDelegate *appDelegate = (MastryAdminAppDelegate *)[[NSApplication sharedApplication] delegate];
    [[NSApp modalWindow] makeKeyWindow];
        //[appDelegate.window makeKeyWindow];

    }
    else if(returnCode == NSAlertFirstButtonReturn)
    {
        NSLog(@"if (returnCode == NSAlertFirstButtonReturn)");
    }
    else if (returnCode == NSAlertSecondButtonReturn)
    {
        NSLog(@"else if (returnCode == NSAlertSecondButtonReturn)");
    }
    else if (returnCode == NSAlertThirdButtonReturn)
    {
        NSLog(@"else if (returnCode == NSAlertThirdButtonReturn)");
    }
    else
    {
        NSLog(@"All Other return code %ld",(long)returnCode);
    }
    
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    /*
     The following options are deprecated in 10.9. Use NSAlertFirstButtonReturn instead
     NSAlertDefaultReturn = 1,
     NSAlertAlternateReturn = 0,
     NSAlertOtherReturn = -1,
     NSAlertErrorReturn = -2
     NSOKButton = 1, // NSModalResponseOK should be used
     NSCancelButton = 0 // NSModalResponseCancel should be used
     */
    
    [self processAlertReturnCode:returnCode];
    
}

-(void)showAlertView:(NSString*)title message:(NSString*)message async:(BOOL)async
{
    if( async )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self showAlertView:title message:message completionHandler:nil];
        });
    }
    else
        [self showAlertView:title message:message completionHandler:nil];
}
-(void)showAlertView:(NSString*)title message:(NSString*)message completionHandler:(void (^ __nullable)(NSModalResponse returnCode))handler
{
    
    //MastryAdminAppDelegate *appDelegate = (MastryAdminAppDelegate *)[[NSApplication sharedApplication] delegate];
    //[self.window setStyleMask:appDelegate.modalWindowStyleMask];

    //dispatch_sync(dispatch_get_main_queue(), ^{
        
        [self.indicatorView hideAndStopAnimating];
        
        NSAlert *alert = [[NSAlert alloc] init];
        //alert.delegate = self;
        //[alert addButtonWithTitle:@"Continue"];
        //[alert addButtonWithTitle:@"OK"];
        [alert setMessageText:title];
        [alert setInformativeText:message];
        [alert setAlertStyle:NSAlertStyleWarning];

        //[alert beginSheetModalForWindow:[self window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:nil];
        NSModalResponse response = [alert runModal];

        [self processAlertReturnCode:response];
        
        //CMThruApplicationDelegate *appDelegate = (CMThruApplicationDelegate *)[[NSApplication sharedApplication] delegate];
        [self.window setStyleMask:NSWindowStyleMaskTitled];
        //[self.window display];
        [self.window layoutIfNeeded];
        [self.window setViewsNeedDisplay:YES];
        
    //});
    
}


- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    //[self.descriptionLabel drawRect:dirtyRect];
    //[self.descriptionLabel drawCellInside:self.descriptionLabel.cell];
    // Drawing code here.
    
    //self.imageView.layer.cornerRadius = self.imageView.frame.size.width/2.0;
    
}
@end
