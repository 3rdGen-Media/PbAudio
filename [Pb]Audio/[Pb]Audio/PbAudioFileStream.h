//
//  PbAudioFileStream.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 11/6/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#ifndef PbAudioFileStream_h
#define PbAudioFileStream_h

#ifdef XNZ_AUDIO
//#include <xnz/xnz.h>
#include <xnz/cr_file.h>
#include <xnz/xnz_aif_codec.h>
#include <xnz/xnz_wav_codec.h>

#ifndef __APPLE__
typedef CRFile* ExtAudioFileRef;
#endif

#else

#ifdef __APPLE__
//#include <stdio.h>
//#import <Foundation/Foundation.h>
//#import <Accelerate/Accelerate.h>
#import <AudioToolbox/AudioToolbox.h>
#else
typedef IMFSourceReader* ExtAudioFileRef;

#endif

#endif


//#define XNG_IMAGE_ALIGNMENT 4096

//#include <math.h>
//#include <limits.h>
//#include "xng_image_buffer.h"

typedef enum xng_audio_form
{
    XNG_AUDIO_FORM_PCM = 0, //raw sample buffer
    XNG_AUDIO_FORM_AIF = 1, //APPLE     aiff
    XNG_AUDIO_FORM_WAV = 2, //MICROSOFT wav
    XNG_AUDIO_FORM_MP3 = 3, //MPEG-3
    XNG_AUDIO_FORM_MP4 = 4, //MPEG-4 TS
}xng_audio_form;


typedef struct PBAFile
{
    union
    {
#ifdef XNZ_AUDIO
        CRFile              file;
        XNZ_AIF_ARCHIVE     aif;
        XNZ_WAV_ARCHIVE     wav;
#else
        ExtAudioFileRef     file;
#endif
    };

    PBAStreamFormat         sourceFormat;
    PBAStreamFormat         conversionFormat;
    
    //TO DO:  Should this be arbitrary channel size?
    void *                  samples[2]; // stereo buffer for unpacking audio file;
    uint64_t                numFrames;

    PBASampleType           type;       //enumerate a sample packing protocol for the given file format
    xng_audio_form          form;
}PBAFile;

typedef struct PBAFile* PBAFileRef;

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamOpen(const char * fileURL, const char * fileExt, PBAStreamFormatRef converterFormat, PBAFileRef inputAudioFileRef);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamReadFrames(PBAFileRef audioFileRef, unsigned long long numFramesToRead, void ** sampleBuffers);
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamClose(ExtAudioFileRef inputAudioFileRef);



#endif /* PbAudioFileStream_h */
