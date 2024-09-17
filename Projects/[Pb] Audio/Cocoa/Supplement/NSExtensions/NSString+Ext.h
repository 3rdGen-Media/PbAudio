//
//  NSString+Ext.h
//  Mastry
//
//  Created by Joe Moulton on 6/9/19.
//  Copyright © 2019 VRTVentures LLC. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <CoreMIDI/MIDIMessages.h>                   //Core Midi Protocol Definitions

NS_ASSUME_NONNULL_BEGIN

@interface NSString (Ext)

typedef NSString* _Nullable (*NSStringConstructor)(const char * str);
+(NSStringConstructor)NSString;
-(NSMutableString*)uriEncode:(BOOL)encodeSlash;
-(NSMutableString*)urlEncode:(BOOL)encodeSlash;

-(NSString*)toBase64;
-(NSString*)toBase64URL;

//currency
+(NSString*)formatCurrencyString:(unsigned long)currencyLong withSpace:(BOOL)space;

+(NSString *)stringForMIDIMessageType:(MIDIMessageType)messageType andStatus:(MIDICVStatus)status;
+(NSString *)stringForMIDICVStatus:(MIDICVStatus)status;


@end

NS_ASSUME_NONNULL_END
