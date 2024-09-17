//
//  CMInputConnection.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/15/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#import "CMDOM.h"
NS_ASSUME_NONNULL_BEGIN

//Create a short MACRO reference to the global instance constructor
#define CMInput [CMInputConnection sharedInstance]

@interface CMInputConnection : CMDOM

+(CMInputConnection*)sharedInstance;

@property (nonatomic, retain) NSString * InputID;
@property (nonatomic, retain) NSString * Input;
@property (nonatomic, retain) NSString * InputDriver;
//@property (nonatomic, retain) NSString * Output;
//@property (nonatomic, retain) NSString * OutputDriver;
//@property (nonatomic, retain) NSString * Filter;

+(void)loadInputConnections;

//Create & Delete are called as class methods
+(CMInputConnection*)createInputConnection:(NSString*)inputID Endpoint:(MIDIThruConnectionEndpoint*)endpoint;
+(void)deleteInputConnection:(CMInputConnection*)inputConnectionDocument;

//but a data model instance can save its internal params to core midi when changed
-(void)saveInputConnection;
-(void)replaceInputConnection:(NSString*)InputID atIndex:(NSUInteger)domIndex;


-(id)initWithCMConnection:(struct CMConnection*)connection;

-(CMConnection*)connection;

@end

NS_ASSUME_NONNULL_END
