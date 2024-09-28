//
//  ToneGenerator.c
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/13/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "ToneGenerator.h"

typedef union{
    struct{
        uint16_t lo;
        uint16_t hi;
    }u16;
    uint32_t u32;
    int32_t  i32;
    float    f;
}Versatype32;

void Float_to_I24bit(int *dest, float *src, uint32_t length)
{
    Versatype32 data;

    while(length--){
        data.f = *src;
        data.i32 = ((uint32_t)(data.f * 8388608) & 0x00ffffff);
        *dest++ = data.u16.hi;
        *dest++ = data.u16.lo;
        src++;
    }
}

int convert(double val) {
    return lround(val * 0x7FFFFF) & 0xFFFFFF;
}

#ifdef __BLOCKS__
PBARenderPass ToneGeneratorRenderPass = ^ (AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp,
                                           PBAStreamFormatSampleType target, void* source, void* events, UInt32 nEvents)
#else
void CALLBACK ToneGeneratorRenderPass(struct PBABufferList * ioData, uint32_t frames, const struct PBATimeStamp * timestamp,
                                      PBAStreamFormatSampleType target, void* source, void* events, uint32_t nEvents)
#endif
{
    //UINT64 playbackSampleOffset = 0;
    uint32_t samplesToCopy = 0;
    uint32_t remainingSamples =0;
    uint32_t bufferIndex = 0;
    uint32_t frameIndex = 0;
        
    //Michael Tyson uses this to do some pthread locking... pthread locks are bullshit
    //Never ever lock a real-time thread!!!
    //AEManagedValueCommitPendingUpdates();0
        
//#ifdef DEBUG
//    uint64_t start = PBACurrentTimeInHostTicks();
//#endif
        
    ToneGenerator * generator = (ToneGenerator*)source;
    
    float * fBufferL = (float*)ioData->mBuffers[0].mData;
    float * fBufferR = (float*)ioData->mBuffers[1].mData;

    uint8_t* byteBufferL = (uint8_t*)ioData->mBuffers[0].mData;
    uint8_t* byteBufferR = (uint8_t*)ioData->mBuffers[1].mData;

    float * samplesL = (float*)(generator->SineWave.buffer);// + frameIndex*2]
    float * samplesR = (float*)(generator->SineWave.buffer);// + frameIndex*2]

    // Clear the output buffer
    //PBABufferListSilence(ioData, 0, frames);
    
    //Fill the buffer with source material
    samplesToCopy = frames;//clientStream->bufferFrameCount;// * 3 / 4; ;
    remainingSamples = (uint32_t)(generator->SineWave.length - generator->WaveSampleOffset);//audioEvent.audioData.size() - playbackSampleOffset;
    if(  remainingSamples > 0 && remainingSamples < samplesToCopy ) samplesToCopy = remainingSamples;

    //Debug output
    //printf("samplesToCopy = %d\n", samplesToCopy);
    //printf("remainingSamples = %d\n", remainingSamples);
    //printf("g_playbackSampleOffset = %d\n", g_playbackSampleOffset );

    float rate = generator->rate;
    static float position = 0.f;

    //TO DO: how to detect interleaved formats or prevent them
#ifdef WIN32
    bool interleaved = true;
#else
    bool interleaved = false;
#endif
    
    typedef union FourByteSample
    {
        uint32_t  u;
        int32_t   normalized;
        uint8_t   bytes[4];
    }FourByteSample;

    FourByteSample bSample;

    for ( int i=0; i<frames; i++ )
    {
        //float * fBufferL = (float*)ioData->mBuffers[0].mData;
        //float * fBufferR = (float*)ioData->mBuffers[1].mData;
        float fSample =  ( (pseudo_sin(rate, &position)) - 0.5 ) * 0.5;

        /*
        //bSample.normalized = convert(fSample);
        //Float_to_I24bit(&iSample, &fSample, 1);

        //convert to unsigned integer range
        bSample.normalized = (int32_t)(fSample * 16777215.);

        bSample.normalized = bSample.normalized & 0xFFFFFF00;
        //bSample.u == bSample.u >> 8;

        //place in byte buffer
        uint8_t* iSampleL = &byteBufferL[i*2*3];
        uint8_t* iSampleR = &byteBufferR[i*2*3];

        iSampleL[0] = bSample.bytes[0];
        iSampleL[1] = bSample.bytes[1];
        iSampleL[2] = bSample.bytes[2];

        //iSampleR[0] = bSample.bytes[1];
        //iSampleR[1] = bSample.bytes[2];
        //iSampleR[2] = bSample.bytes[3];
        */
        
        //if( interleaved ) fBufferL[i*2] = fBufferL[i*2+1] = fSample;
        //else              fBufferL[i]   = fBufferR[i]     = fSample;
    }

//#ifdef DEBUG
//        PBAStreamReportRenderTime(&PBAudio.OutputStreams[0], &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / PBAudio.OutputStreams[0].currentSampleRate);
//#endif
 };

//#ifndef __BLOCKS
//PB_AUDIO_EXTERN PBARenderPass ToneGeneratorRenderPass = ToneGeneratorRenderPassCallback;
//#endif

void ToneGeneratorSetFrequency(ToneGenerator* source, float freq, float sampleRate)
{
    source->sampleRate = sampleRate;
    source->freq = freq;
    source->rate = freq / sampleRate;
}

void ToneGeneratorInit(ToneGenerator* source, float freq, float sampleRate)
{
#ifdef _WIN32 //WIN32 uses interleaved buffers
   uint32_t nSineBufferChannels = 2;
#else
   uint32_t nSineBufferChannels = 1;
#endif
    
    //Generate an Output Source for the Stream (Playback from FLoating Point Sine Wave Buffer)
    uint64_t renderDataLengthInSamples = (uint64_t)sampleRate * 5;
    source->SineWave.length = renderDataLengthInSamples;
    
    source->SineWave.length = sampleRate*5.f;
    double currentSampleRate = PBAudio.OutputStreams[0].currentSampleRate;
    GenerateSineSamplesFloat(&source->SineWave.buffer, source->SineWave.length, freq, nSineBufferChannels, currentSampleRate, 0.25f, NULL);
    ToneGeneratorSetFrequency(source, freq, sampleRate);
}
