//
//  SamplePlayer.c
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/14/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "SamplePlayer.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __BLOCKS__
PBARenderPass SamplePlayerRenderPass = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, void* source, void* events, UInt32 nEvents)
#else
void SamplePlayerRenderPassCallback(struct PBABufferList* ioData, uint32_t frames, const struct PBATimeStamp* timestamp, void* source, void* events, uint32_t nEvents)
#endif
{
    //UINT64 playbackSampleOffset = 0;
    uint32_t samplesToCopy = 0;
    uint32_t remainingSamples =0;
    uint32_t bufferIndex = 0;
    uint32_t frameIndex = 0;

//#ifdef DEBUG
//        uint64_t start = PBACurrentTimeInHostTicks();
//#endif
          
        SamplePlayer * player = (SamplePlayer*)source;
    
        float * fBufferL = (float*)ioData->mBuffers[0].mData;
        float * fBufferR = (float*)ioData->mBuffers[1].mData;

        float * samplesL = (float*)(player->sourceAudioFile.samples[0]);// + frameIndex*2]
        float * samplesR = (float*)(player->sourceAudioFile.samples[1]);// + frameIndex*2]


        //Fill the buffer with source material
        samplesToCopy = frames;//clientStream->bufferFrameCount;// * 3 / 4; ;
        remainingSamples = (uint32_t)(player->sourceAudioFile.numFrames - player->SampleOffset);//audioEvent.audioData.size() - playbackSampleOffset;
        if(  remainingSamples > 0 && remainingSamples < samplesToCopy )
            samplesToCopy = remainingSamples;

        //Debug output
        //printf("samplesToCopy = %d\n", samplesToCopy);
        //printf("remainingSamples = %d\n", remainingSamples);
        //printf("g_playbackSampleOffset = %d\n", g_playbackSampleOffset );

        if( remainingSamples > 0 )
        {
            
            memcpy(fBufferL, &(samplesL[player->SampleOffset]), samplesToCopy * sizeof(float));
            //memcpy(fBufferR, &(samplesR[player->SampleOffset]), samplesToCopy * sizeof(float));
            if( samplesR ) memcpy(fBufferR, &(samplesR[player->SampleOffset]), samplesToCopy * sizeof(float));
            else           memcpy(fBufferR, &(samplesL[player->SampleOffset]), samplesToCopy * sizeof(float));
            
            player->SampleOffset += samplesToCopy;

        }

//#endif

//#ifdef DEBUG
//        PBAStreamReportRenderTime(&PBAudio.OutputStreams[0], &_audioReport, PBASecondsFromHostTicks(PBACurrentTimeInHostTicks() - start), (double)frames / PBAudio.OutputStreams[0].currentSampleRate);
//#endif

 };

#ifndef __BLOCKS__
PB_AUDIO_EXTERN PBARenderPass SamplePlayerRenderPass = SamplePlayerRenderPassCallback;
#endif

#ifdef _WIN32
#ifndef mChannelsPerFrame
#define mChannelsPerFrame nChannels
#endif
#endif

void SamplePlayerInit(SamplePlayer* player, const char * audioFileURL, const char * audioFileEXT, PBAStreamFormat converterFormat)
{
    //Choose an Output Source for the Stream (Playback from PCM File)
    //g_sourceBuffer = (char*)&(audioEvent.audioData[0]);
    //g_sourceBytesPerSample = 3;//fileFormatEx->wBitsPerSample / 8;//(pwfx->nBlockAlign /pwfx->nChannels);
    //g_sourceBufferChannels = 1;//fileFormatEx->nChannels;//g_sineBufferChannels;
    //g_sourceBufferLengthInSamples = audioEvent.audioData.size() / 3;//( g_sourceBytesPerSample * g_sourceBufferChannels);
        
    int i = 0;
#ifdef _WIN32 //WIN32 uses interleaved buffers
    uint32_t nSourceBufferChannels = 2;
#else
    uint32_t nSourceBufferChannels = 1;
#endif
    
    
    PBAFileStreamOpen(audioFileURL, audioFileEXT, PBAudio.OutputStreams[0].format, &player->sourceAudioFile);
    //g_sourceBuffer = (float*)malloc( sourceAudioFile.samples * sizeof(float) * _PBAMasterStream.format.mChannelsPerFrame );
    
    fprintf(stdout, "sourceAudioFile length of file in frames:   %llu", player->sourceAudioFile.numFrames);

    for( i = 0; i<player->sourceAudioFile.conversionFormat.mChannelsPerFrame; i++)
    {
        player->sourceAudioFile.samples[i] = (float*)malloc( player->sourceAudioFile.numFrames * sizeof(float) );
        memset(player->sourceAudioFile.samples[i], 0, player->sourceAudioFile.numFrames * sizeof(float) );
    }
    PBAFileStreamReadFrames(&player->sourceAudioFile, player->sourceAudioFile.numFrames, player->sourceAudioFile.samples);
}
