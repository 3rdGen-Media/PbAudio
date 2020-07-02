//
//  PBABufferList.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreateWithFormat(PBAStreamFormat audioFormat, int frameCount)
{
    int i, j;
    int numberOfBuffers = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? audioFormat.mChannelsPerFrame : 1;
    int channelsPerBuffer = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? 1 : audioFormat.mChannelsPerFrame;
    int bytesPerBuffer = audioFormat.mBytesPerFrame * frameCount;
    
    AudioBufferList *audio = malloc(sizeof(AudioBufferList) + (numberOfBuffers-1)*sizeof(AudioBuffer));
    if ( !audio ) {
        return NULL;
    }
    audio->mNumberBuffers = numberOfBuffers;
    for ( i=0; i<numberOfBuffers; i++ ) {
        if ( bytesPerBuffer > 0 ) {
            audio->mBuffers[i].mData = calloc(bytesPerBuffer, 1);
            if ( !audio->mBuffers[i].mData ) {
                for ( j=0; j<i; j++ ) free(audio->mBuffers[j].mData);
                free(audio);
                return NULL;
            }
        } else {
            audio->mBuffers[i].mData = NULL;
        }
        audio->mBuffers[i].mDataByteSize = bytesPerBuffer;
        audio->mBuffers[i].mNumberChannels = channelsPerBuffer;
    }
    return audio;
}


PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreate(int frameCount) {
    return PBABufferListCreateWithFormat(_audioFormat, frameCount);
}



//Schedule to Audio Buffer List


PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilenceWithFormat(const AudioBufferList *bufferList, AudioStreamBasicDescription audioFormat, UInt32 offset, UInt32 length)
{
    int i;
    //printf("PBAudioBufferListSilenceWithFormat\n");

    for ( i=0; i<bufferList->mNumberBuffers; i++ )
    {
        memset((char*)bufferList->mBuffers[i].mData + offset * audioFormat.mBytesPerFrame,0, length * audioFormat.mBytesPerFrame);
    }
}


//List silence to default engine format
PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilence(const AudioBufferList *bufferList, UInt32 offset, UInt32 length)
{
    PBABufferListSilenceWithFormat(bufferList, _audioFormat, offset, length);
}


