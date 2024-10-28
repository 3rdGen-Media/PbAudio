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

-(id)initWithView:(CocoaView*)view
{
    self = [super init];
    if( self)
    {
        self.view = view;
    }
    return self;
}


- (void)viewDidLoad
{
    NSLog(@"SettingsModalViewController::viewDidLoad");

    [super viewDidLoad];
    
    //_windowID = 1; assert(_windowID > 0 ); //self.view.window.windowNumber;
    //[self loadCoreRenderView];
    //[CRAppDelegate.sharedInstance setColorSpaceProfile:[CGColorSpace genericRGBColorSpace]];

    //[self createChildLayerView];

}


-(void)viewWillUnload
{
    NSLog(@"SettingsModalViewController::viewWillUnload");
    
    //[self removeSubviews];
}

#if !TARGET_OS_OSX

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear: animated];

}


- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
#endif

@end
