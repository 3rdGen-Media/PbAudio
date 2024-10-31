//
//  pbaudio_format.h
//  CRViewer
//
//  Created by Joe Moulton on 6/25/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_format_h
#define pbaudio_format_h

//#include "pba_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * The audio description used throughout TAAE
 *
 *  This is 32-bit floating-point, non-interleaved stereo PCM.
 */
//AudioStreamBasicDescription
//This structure encapsulates all the information for describing the basic
//format properties of a stream of audio data.

#ifdef __APPLE__
#import <AudioToolbox/AudioToolbox.h>
typedef AudioStreamBasicDescription PBAStreamFormat;
typedef AudioComponentDescription	PBAComponentDescription;
#elif defined(_WIN32)

#include <tchar.h>

#include <mmsystem.h>
#include <mmreg.h>

#include <ks.h>           //  KS.H must be included before KSMEDIA.H
#include <ksmedia.h>      //  define KSDATAFORMAT_SUBTYPE_PCM and KSDATAFORMAT_SUBTYPE_IEEE_FLOAT

//these includes solve linker error with PKEY_AudioEngine_DeviceFormat  
//https://stackoverflow.com/questions/9773822/how-to-fix-a-linker-error-with-pkey-device-friendlyname
#include <setupapi.h>  
#include <initguid.h>     // Put this in to get rid of linker errors.  
#include <devpkey.h>      // Property keys defined here are now defined inline. 

//Media Foundation
#include <mmdeviceapi.h>    
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include <functiondiscoverykeys_devpkey.h>  //PKEY_AudioEngine_DeviceFormat

#pragma comment(lib, "ksuser.lib")          //link KSDATAFORMAT_SUBTYPE_PCM and KSDATAFORMAT_SUBTYPE_IEEE_FLOAT

//#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
//#pragma comment(lib, "mfapi.lib")


#pragma comment(lib, "avrt.lib")

typedef union PBAStreamFormat
{
    //WAVEFORMATEXTENSIBLE          extendedFormat;
    WAVEFORMATEXTENSIBLE_IEC61937 extendedFormat;
    struct
    {
        WORD    wFormatTag;        /* format type */
        WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
        DWORD   nSamplesPerSec;    /* sample rate */
        DWORD   nAvgBytesPerSec;   /* for buffer estimation */
        WORD    nBlockAlign;       /* block size of data */
        WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
        WORD    cbSize;            /* The count in bytes of the size of extra information (after cbSize) */
    };
}PBAStreamFormat;

#endif

typedef  PBAStreamFormat* PBAStreamFormatRef;

//By default on Darwin platforms, we will request a 32-bit floating point non-interleaved linear PCM stereo Format
//On WIN32 all formats are interleaved by default?
PB_AUDIO_EXTERN PBAStreamFormat const _audioFormat;


static PB_AUDIO_INLINE PBAStreamFormat PBAStreamFormatWithChannelsAndRate(int channels, double rate)
{
	PBAStreamFormat description = _audioFormat;
#ifdef __APPLE__
    description.mChannelsPerFrame = channels;
    description.mSampleRate = rate;
#elif defined(_WIN32)
	description.nChannels = channels;
	description.nSamplesPerSec = (DWORD)rate;
#endif
	return description;
}

typedef enum PBAStreamFormatSampleType
{
    //SampleType8BitPCM_Interleaved,
	SampleType16BitPCM_Interleaved,
    SampleType24BitPCM_Interleaved,
    SampleType32BitPCM_Interleaved,
    SampleType32BitFloat_Interleaved,
    
    SampleType16BitPCM,
    SampleType24BitPCM,
    SampleType32BitPCM,
    SampleType32BitFloat,
    
	SampleTypeUnknown

}PBAStreamFormatSampleType;

typedef PBAStreamFormatSampleType PBASampleType; //Shorthand Alias

PB_AUDIO_API PB_AUDIO_INLINE PBASampleType PBAStreamFormatGetType(PBAStreamFormat* format);


//AEChannelSet AEChannelSetDefault = {0, 1};



#ifdef __APPLE__
//AudioComponentDescription
//A structure used to describe the unique and identifying IDs of an audio component
static AudioComponentDescription PBAudioComponentDescriptionMake(OSType manufacturer, OSType type, OSType subtype) {
    AudioComponentDescription description;
    memset(&description, 0, sizeof(description));
    description.componentManufacturer = manufacturer;
    description.componentType = type;
    description.componentSubType = subtype;
    return description;
}
#endif

PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamFormatPrint(PBAStreamFormat * format);

#pragma mark -- Format Conversion Routines

//define an abstract buffer conversion function
typedef void (*PBATransformFunc) (void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);

//16 bit interleaved source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s16i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s16i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s16i_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);

//16 bit source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_f32(void* srcBuffer, void* dstBuffer, uint64_t nFrames);

//24 bit interleaved source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s24i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s24i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);

//24 bit non-interleaved source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s24i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s24i_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);


//32 bit interleaved signed int source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f24i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s32i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);


//32 bit float interleaved source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f24i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f32i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f32i_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);

//32 bit float non-interleaved source conversions
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s16(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f24i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);
//PB_AUDIO_API PB_AUDIO_INLINE pba_transform_s16_s24_padded(void* srcBuffer, void* dstBuffer, uint64_t nFrames);
PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f32_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames);

//build a map of n source format indices to n output format indices
//PBAConvertFunc PbAudioConvert[SampleTypeUnknown][SampleTypeUnknown];
static PBATransformFunc pb_audio_transform[SampleTypeUnknown][SampleTypeUnknown] =
{
    //16 bit interleaved source conversions
    {NULL, pba_transform_s16i_s24i, NULL, pba_transform_s16i_f32i, NULL, NULL, NULL, pba_transform_s16i_f32},

    //24 bit interleaved source conversions
    {NULL, pba_transform_s24i_s24i, NULL, pba_transform_s24i_f32i,  NULL, NULL, NULL, pba_transform_s24i_f32},

    //32 bit int interleaved source conversions
    {NULL, NULL, NULL, pba_transform_s32i_f32i,  NULL, NULL, NULL, NULL},

    //32 bit float interleaved source conversions
    {NULL, NULL, NULL, pba_transform_f32i_f32i,  NULL, NULL, NULL, pba_transform_f32i_f32},
    
    //16 bit non-interleaved source conversions
    {NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL},

    //24 bit non-interleaved source conversions
    {NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL},
    
    //32 bit int non-interleaved source conversions
    {NULL, NULL, NULL, NULL,  NULL, NULL, NULL, NULL},

    //32 bit float non-interleaved source conversions
    {NULL, NULL, NULL, NULL,  NULL, NULL, NULL, pba_transform_f32_f32},

};

#ifdef __cplusplus
}
#endif

#endif /* pbaudio_format_h */
