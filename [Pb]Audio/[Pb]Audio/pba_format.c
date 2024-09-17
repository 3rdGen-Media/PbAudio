//
//  PbAudioFormat.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"


PB_AUDIO_EXTERN PBAStreamFormat const _audioFormat = {

#ifdef __APPLE__
    .mFormatID          = kAudioFormatLinearPCM,
    .mFormatFlags       = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved,
    .mChannelsPerFrame  = 2,
    .mBytesPerPacket    = sizeof(float),
    .mFramesPerPacket   = 1,
    .mBytesPerFrame     = sizeof(float),
    .mBitsPerChannel    = 8 * sizeof(float),
    .mSampleRate        = 0,
#elif defined (_WIN32)
	/*.wFormatTag         =*/ 1, /* format type */
    /*.nChannels;         =*/ 2, /* number of channels (i.e. mono, stereo...) */
    /*.nSamplesPerSec	  =*/ 0, /* sample rate */
    /*.nAvgBytesPerSec    =*/ 0, /* for buffer estimation */
    /*.nBlockAlign		  =*/ 8, /* block size of data */
    /*.wBitsPerSample	  =*/ 8 * sizeof(float),     /* number of bits per sample of mono data */
    /*.cbSize			  =*/ 0             /* the count in bytes of the size of */

#endif
};



PB_AUDIO_API PB_AUDIO_INLINE void PBAStreamFormatPrint(PBAStreamFormat * format)
{
#ifdef __APPLE__
    
    int bit;
    unsigned * bits;
    
    /*
    struct AudioStreamBasicDescription
    {
        Float64             mSampleRate;
        AudioFormatID       mFormatID;
        AudioFormatFlags    mFormatFlags;
        UInt32              mBytesPerPacket;
        UInt32              mFramesPerPacket;
        UInt32              mBytesPerFrame;
        UInt32              mChannelsPerFrame;
        UInt32              mBitsPerChannel;
        UInt32              mReserved;
    };
    */
    
    printf("\nformat->mSampleRate = %g", format->mSampleRate);
    int fourCC = CFSwapInt32HostToBig(format->mFormatID);
    /*
    if ( isascii(((char*)&fourCC)[0]) && isascii(((char*)&fourCC)[1]) && isascii(((char*)&fourCC)[2]) ) {
        NSLog(@"%s:%d: %s: '%4.4s' (%d)", file, line, operation, (char*)&fourCC, (int)result);
    } else {
        NSLog(@"%s:%d: %s: %d", file, line, operation, (int)result);
    }
    */
    printf("\nformat->mFormatID = %4.4s", (char*)&fourCC);
    printf("\nformat->mFormatFlags = ");
    bits = pba_convert_to_binary((unsigned *)&(format->mFormatFlags), 32);

    for(bit = 32; bit--;)
        printf("%u", +bits[bit]);
        
    free(bits);
    
    printf("\nformat->mBytesPerPacket = %u", format->mBytesPerPacket);
    printf("\nformat->mFramesPerPacket = %u", format->mFramesPerPacket);
    printf("\nformat->mBytesPerFrame = %u", format->mBytesPerFrame);
    printf("\nformat->mChannelsPerFrame = %u", format->mChannelsPerFrame);
    printf("\nformat->mBitsPerChannel = %u", format->mBitsPerChannel);
    printf("\nformat->mReserved = %u\n", format->mReserved);

#elif defined(_WIN32)
    
#endif
    
    
}
