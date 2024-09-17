//
//  NSCheckboxView.m
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/31/22.
//

#import "NSCheckboxView.h"

@interface NSCheckboxView()
{
    
    
}

@property (nonatomic, retain) NSArray * identifiers;
@property (nonatomic, retain) NSString * title;
@property (nonatomic) NSTextAlignment alignment;

@property (nonatomic, retain) NSTextField * titleLabel;
@property (nonatomic, retain) NSMutableArray * buttons;

@end

@implementation NSCheckboxView

@synthesize buttons = _buttons;

-(void)dealloc
{
    [self.buttons removeAllObjects];
}

-(void)createTitleLabel
{
    if(self.title )
    {
        self.titleLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
        self.titleLabel.focusRingType = NSFocusRingTypeNone;
        self.titleLabel.stringValue = self.title;
        self.titleLabel.wantsLayer = YES;

        self.titleLabel.editable = YES;
        self.titleLabel.drawsBackground = YES;
        self.titleLabel.layer.masksToBounds = NO;
        
        //self.thruIDLabel.usesSingleLineMode = NO;
        //self.thruIDLabel.maximumNumberOfLines = 1;
        self.titleLabel.textColor = NSColor.blackColor;
        self.titleLabel.backgroundColor = [NSColor clearColor];
        self.titleLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
        self.titleLabel.alignment = self.alignment;
        
        self.titleLabel.bezeled         = NO;
        self.titleLabel.editable        = NO;
        self.titleLabel.drawsBackground = NO;
        
        [self addSubview:self.titleLabel];
        
        self.titleLabel.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO
        
        if( self.alignment == NSTextAlignmentRight )
        {
            NSLayoutConstraint * justify = [NSLayoutConstraint constraintWithItem:self.titleLabel
                                                                        attribute: NSLayoutAttributeRight
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:[self.buttons objectAtIndex:0]
                                                                        attribute:NSLayoutAttributeLeft
                                                                        multiplier:1.0f constant:-14.0f];

            NSLayoutConstraint * centery = [NSLayoutConstraint constraintWithItem:self.titleLabel
                                                                        attribute: NSLayoutAttributeCenterY
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:self
                                                                        attribute:NSLayoutAttributeCenterY
                                                                        multiplier:1.0f constant:0.0f];
            
            [self addConstraints:@[ justify, centery]];

        }

        
    }
}

- (void)checkboxToggled:(nullable id)sender
{
    NSLog(@"NSCheckboxView::checkboxToggled:");
    assert( [sender isKindOfClass:NSButton.class]);
    if(self.delegate && [self.delegate respondsToSelector:@selector(buttonToggledAtIndex:sender:)])
        [self.delegate buttonToggledAtIndex:(int)((NSButton*)sender).tag sender:self];
}

-(NSSize)intrinsicContentSize
{
    NSButton * button = self.buttons.lastObject;//[self.buttons objectAtIndex:self.buttons.];
    //CGFloat buttonHeight = button.font.pointSize;
    //CGFloat fCount = (CGFloat)self.buttons.count + 1.;
    
    CGFloat padding = 2.;
    CGFloat buttonHeight  = button.frame.size.height + padding;
    CGFloat buttonsHeight = buttonHeight * self.buttons.count;

    //If the total button content within a DocumentView are less than the dimensions of a containing scrollview
    //then the content will appear with origin at the bottom of the DocumentView...
    CGFloat contentHeight =  buttonsHeight > self.superview.frame.size.height ?
                             buttonsHeight : self.superview.frame.size.height;
    
    NSLog(@"NSCheckBoxView::ContentHeight = %g (buttonsHeight = %g)", contentHeight, buttonsHeight);
    return CGSizeMake(self.superview.frame.size.width, contentHeight);
}

-(void)createCheckboxArray
{
    self.buttons = [NSMutableArray arrayWithCapacity:self.identifiers.count];
    
    if( self.alignment == NSTextAlignmentRight )
    {
        NSButton * prevButton = [[NSButton alloc] initWithFrame:CGRectZero];
        [prevButton.cell setButtonType:NSButtonTypeSwitch];
        prevButton.title = [self.identifiers objectAtIndex:self.identifiers.count-1];
        prevButton.tag = self.identifiers.count-1;
        [prevButton setAction:@selector(checkboxToggled:)];
        [prevButton setTarget:self];
        
        //[prevButton performClick:YES];
        
        [self addSubview:prevButton];
        
        prevButton.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO
        NSLayoutConstraint * right = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeRight
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self
                                                                    attribute:NSLayoutAttributeRight
                                                                    multiplier:1.0f constant:0.0f];

        NSLayoutConstraint * centery = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeCenterY
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self
                                                                    attribute:NSLayoutAttributeCenterY
                                                                    multiplier:1.0f constant:0.0f];
        [self addConstraints:@[ right, centery]];
        [self.buttons addObject:prevButton];

        
        for( int i = (int)self.identifiers.count-2; i>-1; i--)
        {
            NSButton * button = [[NSButton alloc] initWithFrame:CGRectZero];
            [button.cell setButtonType:NSButtonTypeSwitch];
            button.title = [self.identifiers objectAtIndex:i];
            button.tag = i;
            [button setAction:@selector(checkboxToggled:)];
            [button setTarget:self];
            
            [self addSubview:button];
            button.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO

            right = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeRight
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:prevButton
                                                                        attribute:NSLayoutAttributeLeft
                                                                        multiplier:1.0f constant:0.0f];
            centery = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeCenterY
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:self
                                                                        attribute:NSLayoutAttributeCenterY
                                                                        multiplier:1.0f constant:0.0f];
            [self addConstraints:@[ right, centery]];

            [self.buttons addObject:button];
            prevButton = button;
        }
        
        //reverse the array of buttons
        self.buttons = [NSMutableArray arrayWithArray:[[self.buttons reverseObjectEnumerator] allObjects]];
        
        [self createTitleLabel];
    }
    else if( self.alignment == NSTextAlignmentVertical )
    {
        NSButton * prevButton = [[NSButton alloc] initWithFrame:CGRectZero];
        [prevButton.cell setButtonType:NSButtonTypeSwitch];
        prevButton.title = [self.identifiers objectAtIndex:0];
        prevButton.tag = 0;
        //[prevButton sizeToFit]; //Note: this will cause weird microadjustments to the origin of checkbox buttons
        [prevButton setAction:@selector(checkboxToggled:)];
        prevButton.enabled = YES;
        prevButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder
        [self addSubview:prevButton];
        
        
        prevButton.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO
        NSLayoutConstraint * left = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeLeft
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self
                                                                    attribute:NSLayoutAttributeLeft
                                                                    multiplier:1.0f constant:4.0f];
        
        //Note: The only way I have been able to recreate ScrollView's DocumentView with different heights
        //is to give the DocumentView's children explicit height so that intrinsicContentSize reports the correct button heights :(
        NSLayoutConstraint * height = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeHeight
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self.superview
                                                                    attribute:NSLayoutAttributeHeight
                                                                    multiplier:0. constant:16.f];

        NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeTop
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self
                                                                    attribute:NSLayoutAttributeTop
                                                                    multiplier:1.0f constant:2.];
        [self addConstraints:@[ left, top, height]];
        [self.buttons addObject:prevButton];
         
        
        for( int i = 1; i<self.identifiers.count; i++)
        {
            NSButton * button = [[NSButton alloc] initWithFrame:CGRectZero];
            [button.cell setButtonType:NSButtonTypeSwitch];
            button.title = [self.identifiers objectAtIndex:i];
            button.tag = i;
            //[prevButton sizeToFit]; //Note: this will cause weird microadjustments to the origin of checkbox buttons
            [button setAction:@selector(checkboxToggled:)];
            button.enabled = YES;
            button.target = self; //without this popup items will appear grayed out if no other ui to make first responder
            [self addSubview:button];
            
            
            button.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO

            left = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeLeft
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:prevButton
                                                                        attribute:NSLayoutAttributeLeft
                                                                        multiplier:1.0f constant:0.0f];
            
            NSLayoutConstraint * height = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeHeight
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:prevButton
                                                                        attribute:NSLayoutAttributeHeight
                                                                        multiplier:1. constant:0.f];
            
            top = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeTop
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:prevButton
                                                                        attribute:NSLayoutAttributeBottom
                                                                        multiplier:1.0f constant:4.];
            [self addConstraints:@[ left, top, height]];
            [self.buttons addObject:button];
             prevButton = button;
        }
        
    }
    else
    {
        
        NSButton * prevButton = [[NSButton alloc] initWithFrame:CGRectZero];
        [prevButton.cell setButtonType:NSButtonTypeSwitch];
        prevButton.title = [self.identifiers objectAtIndex:0];
        prevButton.tag = 0;
        //[prevButton setAction:@selector(buttonToggled:)];

        [self addSubview:prevButton];
        
        prevButton.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO
        NSLayoutConstraint * left = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeLeft
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self
                                                                    attribute:NSLayoutAttributeLeft
                                                                    multiplier:1.0f constant:0.0f];

        NSLayoutConstraint * centery = [NSLayoutConstraint constraintWithItem:prevButton
                                                                    attribute: NSLayoutAttributeCenterY
                                                                        relatedBy:NSLayoutRelationEqual
                                                                       toItem:self
                                                                    attribute:NSLayoutAttributeCenterY
                                                                    multiplier:1.0f constant:0.0f];
        [self addConstraints:@[ left, centery]];
        [self.buttons addObject:prevButton];

        
        for( int i = 1; i<self.identifiers.count; i++)
        {
            NSButton * button = [[NSButton alloc] initWithFrame:CGRectZero];
            [button.cell setButtonType:NSButtonTypeSwitch];
            button.title = [self.identifiers objectAtIndex:i];
            button.tag = i;
            //[button setAction:@selector(buttonToggled:)];
            [self addSubview:button];
            button.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO

            left = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeLeft
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:prevButton
                                                                        attribute:NSLayoutAttributeRight
                                                                        multiplier:1.0f constant:0.0f];
            centery = [NSLayoutConstraint constraintWithItem:button
                                                                        attribute: NSLayoutAttributeCenterY
                                                                            relatedBy:NSLayoutRelationEqual
                                                                           toItem:self
                                                                        attribute:NSLayoutAttributeCenterY
                                                                        multiplier:1.0f constant:0.0f];
            [self addConstraints:@[ left, centery]];

            [self.buttons addObject:button];
            prevButton = button;
        }
        
        
    }

}

-(id)initWithIdentifiers:(NSArray*)identifiers Title:(NSString* _Nullable)title Justification:(NSTextAlignment)alignment
{
    self = [super initWithFrame:CGRectZero];
    if( self )
    {
        self.identifiers = identifiers;
        self.title = title;
        self.alignment = alignment;

        [self createCheckboxArray];
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

@end
