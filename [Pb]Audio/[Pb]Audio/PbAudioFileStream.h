//
//  PbAudioFileStream.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 11/6/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#ifndef PbAudioFileStream_h
#define PbAudioFileStream_h

//#include <stdio.h>
//#import <Foundation/Foundation.h>
//#import <Accelerate/Accelerate.h>
#import <AudioToolbox/AudioToolbox.h>
//#import "TheAmazingAudioEngine.h"
//#import "AbstractAudioDefines.h"

typedef struct PBAFile
{
    ExtAudioFileRef         file;
    PBAStreamFormat         sourceFormat;
    PBAStreamFormat         conversionFormat;
    unsigned long long      numFrames;
    
    //TO DO:  Should this be arbitrary channel size?
    void *                  samples[2]; // stereo buffer for unpacking audio file;
}PBAFile;

typedef struct PBAFile* PBAFileRef;

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamOpen(const char * fileURL, const char * fileExt, PBAStreamFormat converterFormat, PBAFileRef inputAudioFileRef);
PB_AUDIO_API PB_AUDIO_INLINE unsigned long long PBAFileStreamReadFrames(PBAFileRef audioFileRef, unsigned long long numFramesToRead, void ** sampleBuffers );
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamClose(ExtAudioFileRef inputAudioFileRef);



#endif /* PbAudioFileStream_h */
