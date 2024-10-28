//
//  ViewController.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#import "AudioUnitViewController.h"
#import "AudioUnitView.h"
#import "PBAudioUnit.h"

@implementation AudioUnitViewController

-(void)createActiveDeviceViewController
{
    //self.deviceView = [[MCUCommanderView alloc] initWithFrame:self.view.frame];
    //self.deviceViewController = [[C4CommanderViewController alloc] initWithView:self.deviceView];
    
    //[self.view addSubview:self.deviceView];
    //[self addChildViewController:self.deviceViewController];
}

-(id)initWithView:(CocoaView*)view
{
    self = [super init];
    
    if (self)
    {
        NSLog(@"AudioUnitViewController::initWithView()");
        self.view = view;
        
        //[self createActiveDeviceViewController];
    }
    
    return self;
    
}

-(void)loadRootView
{
    //NSWindow * appWindow = [[NSApplication sharedApplication] windows].firstObject;
    //CGFloat screenScale  = appWindow.backingScaleFactor;
    
    //The window rect passed to OpenGL/Metal Layers should be in point size
    //such that the layer::setBounds: method will set the CoreRender crView.contextSize appropriately
    CGRect windowRect  = CGRectMake(0,0, CocoaScreenSize.width, CocoaScreenSize.height);

    //However, for IOSurface backed layers must be created with account for screen scale like an offscreen render target
    //and we create them with the glView.frame rather manually muxing scale because view.frame sets itself with scale accounted
    //CGRect surfaceRect = CGRectMake(0,0, CocoaScreenSize.width * screenScale, CocoaScreenSize.height * screenScale);
    
    //For Metal we mimic the OpenGL trick where we create the window at screen size then resize to the desired window size
    AudioUnitView * rootView = [[AudioUnitView alloc] initWithFrame:windowRect];
    NSLog(@"AudioUnit.rootView.frame.size = (%g, %g)", rootView.frame.size.width, rootView.frame.size.height);
    self.view = rootView; //[self.view addSubview:mtlView]; //SceneView must be the root view in order to resize window based on content size
    
    //CGFloat scale = MIN( 1.0/retinaScale, 1.0/retinaScale );
    //crView.transform = CGAffineTransformScale(CGAffineTransformIdentity, scale, scale);

    //Create HUD Child Layer View backed by IOSurface
    //self.HUDView = [CRSurfaceLayerView frame:mtlView.frame parent:mtlView.crView];

    //On iOS and OSX, we rely on the layer setBounds: method to be called to get the size of the GL screen framebuffers before calling sendToCoreRender
    //[mtlView sendToCoreRender:_windowID];
    
#if TARGET_OS_OSX
    //make the OSX window half the screen size
    rootView.frame = CGRectMake(0,0, NSScreen.mainScreen.frame.size.width/2., NSScreen.mainScreen.frame.size.height/2.);
#endif
    
    //_SceneView = mtlView;
    //[self.view addSubview:self.HUDView];
}

#if TARGET_OS_OSX

-(id)initWithWindow:(NSWindow*)window
{
    self = [super init];
    if (self)
    {
        //_windowID = (CGWindowID)window.windowNumber; assert(_windowID > 0 );
        //[self loadCoreRenderView];
        //[CRAppDelegate.sharedInstance setColorSpaceProfile:[NSColorSpace sRGBColorSpace]];
        
        [self loadRootView];
    }
    return self;
}

- (void)viewDidLoad
{
    NSLog(@"AudioUnitViewController::viewDidLoad");

    [super viewDidLoad];
    
    //_windowID = 1; assert(_windowID > 0 ); //self.view.window.windowNumber;
    
    [self loadRootView]; //[self loadCoreRenderView];
    //[CRAppDelegate.sharedInstance setColorSpaceProfile:[CGColorSpace genericRGBColorSpace]];

    //[self createChildLayerView];

}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

#else

-(id)initWithWindow:(CocoaWindow*)window
{
    self = [super init];
    if( self )
    {
        //_windowID = 1;
    }
    return self;
}

- (void)viewDidLoad
{
    NSLog(@"CRViewController::viewDidLoad");

    [super viewDidLoad];
    
    //_windowID = 1; assert(_windowID > 0 ); //self.view.window.windowNumber;
    
    [self loadRootView]; //[self loadCoreRenderView];
    //[CRAppDelegate.sharedInstance setColorSpaceProfile:[CGColorSpace genericRGBColorSpace]];

    //[self createChildLayerView];

}

-(void)viewWillUnload
{
    NSLog(@"CRViewController::viewWillUnload");
    
    //[self removeSubviews];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}
#endif


@end



@implementation AudioUnitViewController (AUAudioUnitFactory)

/*
 private func configureSwiftUIView(audioUnit: AUAudioUnit) {
     if let host = hostingController {
         host.removeFromParent()
         host.view.removeFromSuperview()
     }
     
     guard let observableParameterTree = audioUnit.observableParameterTree else {
         return
     }
     let content = SampleInstrumentMainView(parameterTree: observableParameterTree)
     let host = HostingController(rootView: content)
     self.addChild(host)
     host.view.frame = self.view.bounds
     self.view.addSubview(host.view)
     hostingController = host
     
     // Make sure the SwiftUI view fills the full area provided by the view controller
     host.view.translatesAutoresizingMaskIntoConstraints = false
     host.view.topAnchor.constraint(equalTo: self.view.topAnchor).isActive = true
     host.view.leadingAnchor.constraint(equalTo: self.view.leadingAnchor).isActive = true
     host.view.trailingAnchor.constraint(equalTo: self.view.trailingAnchor).isActive = true
     host.view.bottomAnchor.constraint(equalTo: self.view.bottomAnchor).isActive = true
     self.view.bringSubviewToFront(host.view)
 }
 */


#pragma mark -- Audio Unit Extension Entry Point

- (nullable AUAudioUnit *) createAudioUnitWithComponentDescription:(AudioComponentDescription) desc error:(NSError **)error
{
    /*
    return try DispatchQueue.main.sync {
        
        audioUnit = try SampleInstrumentAudioUnit(componentDescription: componentDescription, options: [])
        
        guard let audioUnit = self.audioUnit as? SampleInstrumentAudioUnit else {
            log.error("Unable to create SampleInstrumentAudioUnit")
            return audioUnit!
        }
        
        defer {
            // Configure the SwiftUI view after creating the AU, instead of in viewDidLoad,
            // so that the parameter tree is set up before we build our @AUParameterUI properties
            DispatchQueue.main.async {
                self.configureSwiftUIView(audioUnit: audioUnit)
            }
        }
        
        audioUnit.setupParameterTree(SampleInstrumentParameterSpecs.createAUParameterTree())
        
        self.observation = audioUnit.observe(\.allParameterValues, options: [.new]) { object, change in
            guard let tree = audioUnit.parameterTree else { return }
            
            // This insures the Audio Unit gets initial values from the host.
            for param in tree.allParameters { param.value = param.value }
        }
        
        guard audioUnit.parameterTree != nil else {
            log.error("Unable to access AU ParameterTree")
            return audioUnit
        }
        
        return audioUnit
    */
    
    self.audioUnit = [[PBAudioUnit alloc] initWithComponentDescription:desc error:error];
    return self.audioUnit;
}

@end
