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

#pragma mark -- Conversion Implementations


void pba_transform_s16i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    char** sourceBuffers = (char**)srcBuffers;
    char** byteBuffers   = (char**)dstBuffers;

    //raw byte source buffer
    char* samplesL = (sourceBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //raw byte dst buffer
    char* byteBufferL = byteBuffers[0];
    //char* byteBufferR = byteBuffers[1];

    for (int i = 0; i < nFrames; i++)
    {
        int32_t iSample;
        float   fSample;

        char sourceSampleL[4] = { 0, samplesL[(i*nBufferChannels)   * 2 + 0], samplesL[(i*nBufferChannels)   * 2 + 1], 0 };
        char sourceSampleR[4] = { 0, samplesL[(i*nBufferChannels+1) * 2 + 0], samplesL[(i*nBufferChannels+1) * 2 + 1], 0 };

        memcpy(&(byteBufferL[(i * nBufferChannels + 0) * 3]), sourceSampleL, 3);
        memcpy(&(byteBufferL[(i * nBufferChannels + 1) * 3]), sourceSampleR, 3);

        //fBufferL[i * 2] = fBufferL[i * 2 + 1] = fSample;
    }

    return;
}


//pack 16-bit int to 32-bit float
//assumes samples are in host format endian order
//assumes dstBuffers are interleaved stereo
void pba_transform_s16i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    int16_t ** shortBuffers = (int16_t**)srcBuffers;
 
    //source
    int16_t* shortSamplesL = shortBuffers[0];// (player->sourceAudioFile.samples[0]);
    //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    //float* fBufferR = (float*)ioData->mBuffers[1].mData;


    for (int32_t i = 0; i < nFrames; i++)
    {
        //target sample vars
        float    fSampleL, fSampleR;

        //source stream indices per channel
        uint64_t iL = (((i) * nBufferChannels) + 0);
        uint64_t iR = (((i) * nBufferChannels) + nBufferChannels-1);

        //get source samples for current frame
        int16_t iSampleL = shortSamplesL[iL];
        int16_t iSampleR = shortSamplesL[iR];

        //convert to target sample range
        fSampleL = ((float)iSampleL) / 32768.f;
        fSampleR = ((float)iSampleR) / 32768.f;

        //place sample in output stream
        fBufferL[i * 2]     = fSampleL;
        fBufferL[i * 2 + 1] = fSampleR;

    }

    return;
}

PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s16i_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    int16_t ** shortBuffers = (int16_t**)srcBuffers;
 
    //source
    int16_t* shortSamplesL = shortBuffers[0];// (player->sourceAudioFile.samples[0]);
    //int16_t* shortSamplesR = (int16_t*)(player->sourceAudioFile.samples[1]);

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    float* fBufferR = dstBuffers[1] ? (float*)dstBuffers[1] : fBufferL;

    for (int32_t i = 0; i < nFrames; i++)
    {
        //target sample vars
        float    fSampleL, fSampleR;

        //source stream indices per channel
        uint64_t iL = (((i) * nBufferChannels) + 0);
        uint64_t iR = (((i) * nBufferChannels) + nBufferChannels-1);

        //get source samples for current frame
        int16_t iSampleL = shortSamplesL[iL];
        int16_t iSampleR = shortSamplesL[iR];

        //convert to target sample range
        fSampleL = ((float)iSampleL) / 32768.f;
        fSampleR = ((float)iSampleR) / 32768.f;

        //place sample in output stream
        fBufferL[i] = fSampleL;
        fBufferR[i] = fSampleR;

    }

    return;
    
}

void pba_transform_s24i_s24i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    char** sourceBuffers = (char**)srcBuffers;
    char** byteBuffers   = (char**)dstBuffers;

    //raw byte source buffer
    char* samplesL = (sourceBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //raw byte dst buffer
    char* byteBufferL = byteBuffers[0];
    //char* byteBufferR = byteBuffers[1];

    for (int i = 0; i < nFrames; i++)
    {
        int32_t iSample;
        float   fSample;

        typedef union ubytes32
        {
            char     bytes[4];
            uint32_t u;
        }ubytes32;

        ubytes32 sourceSampleL = { samplesL[(i*nBufferChannels)   * 3 + 0], samplesL[(i*nBufferChannels)  * 3 + 1], samplesL[(i*nBufferChannels)   * 3 + 2], 0 };
        ubytes32 sourceSampleR = { samplesL[(i*nBufferChannels+1) * 3 + 0], samplesL[(i*nBufferChannels+1)* 3 + 1], samplesL[(i*nBufferChannels+1) * 3 + 2], 0 };

        //memcpy(&iSample, &sourceSample, 4);

        //map 32-bit integer range [-2147483648, 2147483647] to 32-bit float [-1.0, 1.0]
        //fSample = ((float)iSample) / 2147483647.f;

        //sourceSampleL.u = sourceSampleL.u >> 4;
        //sourceSampleR.u = sourceSampleR.u >> 4;

        memcpy(&(byteBufferL[(i * nBufferChannels + 0) * 3]), sourceSampleL.bytes, 3);
        memcpy(&(byteBufferL[(i * nBufferChannels + 1) * 3]), sourceSampleR.bytes, 3);

        //fBufferL[i * 2] = fBufferL[i * 2 + 1] = fSample;
    }

}


//pack 24-bit interleaved int to 32-bit interleaved float
//assumes samples are in host format endian order
//assumes dstBuffers are interleaved stereo
void pba_transform_s24i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    char** sourceBuffers = (char**)srcBuffers;

    //raw byte source buffer
    char* samplesL = (sourceBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    //float* fBufferR = (float*)ioData->mBuffers[1].mData;

    for (int i = 0; i < nFrames; i++)
    {
        int32_t iSampleL, iSampleR;
        float   fSampleL, fSampleR;

        //place 24 bit le signed integer in most significant bytes of a 32 bit le signed integer
        unsigned char sourceSampleL[4] = { 0, samplesL[(i) * nBufferChannels * 3 + 0], samplesL[(i) * nBufferChannels * 3 + 1], samplesL[(i) * nBufferChannels * 3 + 2] };
        unsigned char sourceSampleR[4] = { 0, samplesL[(((i)*nBufferChannels) + nBufferChannels - 1) * 3 + 0], samplesL[(((i)*nBufferChannels) + nBufferChannels - 1) * 3 + 1], samplesL[(((i)*nBufferChannels) + nBufferChannels - 1) * 3 + 2] };

        memcpy(&iSampleL, sourceSampleL, 4);
        memcpy(&iSampleR, sourceSampleR, 4);

        //SO A
        //iSample = sourceSample[3] << 24 | sourceSample[2] << 16 | sourceSample[1] << 8;
        //fSample = (float)iSample / (float)(INT_MAX - 256);

        //SO B
        //iSample = ((sourceSample[3] << 24) | (sourceSample[2] << 16) | (sourceSample[1] << 8)) >> 8;
        //fSample = ((float)iSample) / 8388607.0;

        //Mine
        //map 32-bit integer range [-2147483648, 2147483647] to 32-bit float [-1.0, 1.0]
        fSampleL = ((float)iSampleL) / 2147483647.f;
        fSampleR = ((float)iSampleR) / 2147483647.f;

        //memcpy(&(byteBufferL[(i * nStreamChannels + 0) * 3]), sourceSample, 3);
        //memcpy(&(byteBufferL[(i * nStreamChannels + 1) * 3]), sourceSample, 3);

        fBufferL[i * 2]     = fSampleL;
        fBufferL[i * 2 + 1] = fSampleR;

    } 
}

//pack 24-bit interleaved int to 32-bit non-interleaved float
//assumes samples are in host format endian order
//assumes dstBuffers are non-interleaved stereo
void pba_transform_s24i_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    char** sourceBuffers = (char**)srcBuffers;

    //raw byte source buffer
    char* samplesL = (sourceBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    float* fBufferR = dstBuffers[1] ? (float*)dstBuffers[1] : fBufferL;

    for (int i = 0; i < nFrames; i++)
    {
        int32_t iSampleL, iSampleR;
        float   fSampleL, fSampleR;

        //place 24 bit le signed integer in most significant bytes of a 32 bit le signed integer
        unsigned char sourceSampleL[4] = { 0, samplesL[(i) * nBufferChannels * 3 + 0], samplesL[(i) * nBufferChannels * 3 + 1], samplesL[(i) * nBufferChannels * 3 + 2] };
        unsigned char sourceSampleR[4] = { 0, samplesL[(((i)*nBufferChannels) + nBufferChannels - 1) * 3 + 0], samplesL[(((i)*nBufferChannels) + nBufferChannels - 1) * 3 + 1], samplesL[(((i)*nBufferChannels) + nBufferChannels - 1) * 3 + 2] };

        memcpy(&iSampleL, sourceSampleL, 4);
        memcpy(&iSampleR, sourceSampleR, 4);

        //SO A
        //iSample = sourceSample[3] << 24 | sourceSample[2] << 16 | sourceSample[1] << 8;
        //fSample = (float)iSample / (float)(INT_MAX - 256);

        //SO B
        //iSample = ((sourceSample[3] << 24) | (sourceSample[2] << 16) | (sourceSample[1] << 8)) >> 8;
        //fSample = ((float)iSample) / 8388607.0;

        //Mine
        //map 32-bit integer range [-2147483648, 2147483647] to 32-bit float [-1.0, 1.0]
        fSampleL = ((float)iSampleL) / 2147483647.f;
        fSampleR = ((float)iSampleR) / 2147483647.f;

        //memcpy(&(byteBufferL[(i * nStreamChannels + 0) * 3]), sourceSample, 3);
        //memcpy(&(byteBufferL[(i * nStreamChannels + 1) * 3]), sourceSample, 3);

        fBufferL[i] = fSampleL;
        fBufferR[i] = fSampleR;

    }
}


PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_s32i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    //raw byte source buffer
    int32_t* samplesL = (float*)(srcBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    //float* fBufferR = (float*)dstBuffers[1];

    for (int i = 0; i < nFrames; i++)
    {
        int32_t iSampleL, iSampleR;
        float   fSampleL, fSampleR;

        //place 24 bit le signed integer in most significant bytes of a 32 bit le signed integer
        iSampleL = samplesL[i * nBufferChannels];
        iSampleR = samplesL[(((i)*nBufferChannels) + nBufferChannels - 1)];

        fSampleL = ((float)iSampleL) / 2147483647.f;
        fSampleR = ((float)iSampleR) / 2147483647.f;

        fBufferL[i * 2]     = fSampleL;
        fBufferL[i * 2 + 1] = fSampleR;
    }

}


PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f32i_f32i(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    uint64_t i = 0;
    //raw byte source buffer
    float* samplesL = (float*)(srcBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    //float* fBufferR = (float*)dstBuffers[1];

    for (i = 0; i < nFrames; i++)
    {
        int32_t iSampleL, iSampleR;
        float   fSampleL, fSampleR;

        //place 24 bit le signed integer in most significant bytes of a 32 bit le signed integer
        fSampleL = samplesL[i * nBufferChannels];
        fSampleR = samplesL[(((i)*nBufferChannels) + nBufferChannels - 1)];

        fBufferL[i * 2]     = fSampleL;
        fBufferL[i * 2 + 1] = fSampleR;
    } 

}

PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f32i_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    uint64_t i = 0;
    //raw byte source buffer
    float* samplesL = (float*)(srcBuffers[0]);// + frameIndex*2]
    //char* samplesR = (sourceBuffers[0]);// + frameIndex*2]

    //dest
    float* fBufferL = (float*)dstBuffers[0];
    float* fBufferR = dstBuffers[1] ? (float*)dstBuffers[1] : fBufferL;

    for (i = 0; i < nFrames; i++)
    {
        //int32_t iSampleL, iSampleR;
        float   fSampleL, fSampleR;

        //place 24 bit le signed integer in most significant bytes of a 32 bit le signed integer
        fSampleL = samplesL[i * nBufferChannels];
        fSampleR = samplesL[(((i)*nBufferChannels) + nBufferChannels - 1)];

        fBufferL[i] = fSampleL;
        fBufferR[i] = fSampleR;
    }

}

PB_AUDIO_API PB_AUDIO_INLINE void pba_transform_f32_f32(void** srcBuffers, void** dstBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    uint64_t channel;
    for(channel = 0; channel < nBufferChannels; channel++)
    {
        //source
        float* samplesL = (float*)(srcBuffers[channel]);// + frameIndex*2]

        //dest
        float* fBufferL = (float*)dstBuffers[channel];
        
        memcpy(fBufferL, samplesL, nFrames * sizeof(float));
    }
}


PB_AUDIO_API PB_AUDIO_INLINE PBASampleType PBAStreamFormatGetType(PBAStreamFormat *format)
{
#ifdef _WIN32
    if ( (format->wFormatTag == WAVE_FORMAT_PCM) ||
       ( (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) && memcmp(&((WAVEFORMATEXTENSIBLE *)format)->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID)) == 0 ) )
    {
        if (format->wBitsPerSample == 32) return SampleType32BitPCM_Interleaved;
        if (format->wBitsPerSample == 24) return SampleType24BitPCM_Interleaved;
        if (format->wBitsPerSample == 16) return SampleType16BitPCM_Interleaved;
    }
    else if ( (format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) ||
        ((format->wFormatTag == WAVE_FORMAT_EXTENSIBLE) && memcmp(&((WAVEFORMATEXTENSIBLE*)format)->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID)) == 0))
    {
        return 	SampleType32BitFloat_Interleaved;
    }
#elif defined(__APPLE__)
    if ( format->mFormatID == kAudioFormatLinearPCM )
    {
        AudioFormatFlags isFloat          = (format->mFormatFlags & kAudioFormatFlagIsFloat) / kAudioFormatFlagIsFloat;
        AudioFormatFlags isNonInterleaved = ((format->mFormatFlags & kAudioFormatFlagIsNonInterleaved) / kAudioFormatFlagIsNonInterleaved);
        
        if (format->mBitsPerChannel == 32)
        {
            if( isFloat) return SampleType32BitFloat_Interleaved + (isNonInterleaved * 4);
            else         return SampleType32BitPCM_Interleaved   + (isNonInterleaved * 4);
        }
        if (format->mBitsPerChannel == 24) return SampleType24BitPCM_Interleaved + (isNonInterleaved * 4);
        if (format->mBitsPerChannel == 16) return SampleType16BitPCM_Interleaved + (isNonInterleaved * 4);
    }

    
#endif
    
    return SampleTypeUnknown;
}


//build a map of n source format indices to n output format indices
