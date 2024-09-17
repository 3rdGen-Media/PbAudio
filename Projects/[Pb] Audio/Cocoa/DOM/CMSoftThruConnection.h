//
//  CMSoftThruConnection.h
//  C4Commander
//
//  Created by Joe Moulton on 11/24/23.
//

#import "CMDOM.h"
NS_ASSUME_NONNULL_BEGIN

//Create a short MACRO reference to the global instance constructor
#define CMSoftThru [CMSoftThruConnection sharedInstance]

@interface CMSoftThruConnection : CMDOM

+(CMSoftThruConnection*)sharedInstance;

@property (nonatomic, retain) NSString * ThruID;
@property (nonatomic, retain) NSString * Input;
@property (nonatomic, retain) NSString * InputDriver;
@property (nonatomic, retain) NSString * Output;
@property (nonatomic, retain) NSString * OutputDriver;
@property (nonatomic, retain) NSString * Filter;
//@property (nonatomic, retain) NSString * Map;

+(void)loadSoftThruConnections;

//Create & Delete are called as class methods
+(CMSoftThruConnection*)createSoftThruConnection:(NSString*)thruID Params:(MIDIThruConnectionParams*)params;
+(void)deleteSoftThruConnection:(CMSoftThruConnection*)thruConnectionDocument;

//but a data model instance can save its internal params to core midi when changed
-(void)saveSoftThruConnection;
-(void)replaceSoftThruConnection:(NSString*)ThruID atIndex:(NSUInteger)domIndex;


-(id)initWithCMConnection:(struct CMConnection*)connection;

-(CMConnection*)connection;

@end

NS_ASSUME_NONNULL_END
