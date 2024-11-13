//
//  pba_dsp_waveforms.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/13/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef pba_dsp_waveforms_h
#define pba_dsp_waveforms_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <malloc.h>

#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#endif


static float pba_decibels_for_buffer(void** srcBuffers, uint64_t nBufferChannels, uint64_t nFrames)
{
    float dbFS = 0.f;
    
    //src
    float* samplesL = (float*)(srcBuffers[0]);
    float* samplesR = (float*)(srcBuffers[1]);
    
    /*
    //First, compute the RMS sum of a chunk of data
    float sumSquared = 0;
    for (int i = 0 ; i < nFrames ; i++)
    {
        float sampleL = fabs(samplesL[i]);//(samplesL[i]+1.f)/2.f; // map to 0-1 range
        sumSquared += (sampleL * sampleL);
    }
    float rms = sumSquared/(float)nFrames;
    
    //Next convert the RMS value to dB
    float dBFS = 20.f * log10f(rms);
    */
    
    
    //LOOP over each INPUT CHANNEL
    for ( int i=0; i<nBufferChannels; i++ )
    {
        /*
        // Pull audio and apply input gain
        vDSP_vsmul((float*)audio->mBuffers[i].mData, 1, &THIS->outputGain, THIS->inputBuffer[i], 1, frames);
        
        //get peak input amplitude magnitude across channel buffer
        float peak = 0.0f;
        vDSP_maxmgv(THIS->inputBuffer[i], 1, &peak, frames);
        //update our stored peak amplitude if necessary
        if ( peak > THIS->peakAmplitude[i] ) THIS->peakAmplitude[i] = peak;
        */

        float avg = 0.0f;
        //get average input amplitude magnitude across channel buffer
#ifdef __APPLE__
        vDSP_meamgv((float*)(srcBuffers[i]), 1, &avg, nFrames);
#else
        assert(1 == 0);
#endif
        //retain only the greatest avereage for monitoring threshold levels across input pairs
        if(avg > dbFS) dbFS = avg;

        //copy the input with gain applied back to the buffer
        //memcpy( (float*)audio->mBuffers[i].mData, THIS->inputBuffer[i],  sizeof(float)*frames );
    }
    
    dbFS = 20.0f * log10f(dbFS);

    return dbFS;
}

/*!
 * Generate oscillator/LFO
 *
 *  This function produces, sample by sample, an oscillator signal that approximates a sine wave. Its
 *  output lies in the range 0 - 1.
 *
 * @param rate Oscillation rate, per sample (frequency / sample rate)
 * @param position On input, current oscillator position; on output, new position.
 * @return One sample of oscillator signal
 */
static inline float pseudo_sin(float rate, float * position)
{
    float x = *position;
    x *= x;
    x -= 1.0;
    x *= x;
    *position += rate;
    if ( *position > 1.0 ) *position -= 2.0;
    return x;
}


static void GenerateSineSamplesFloat(float **Buffer, size_t BufferLengthInSamples, unsigned long Frequency, unsigned short ChannelCount, double SamplesPerSecond, double Amplitude, double * InitialTheta)
{
    size_t i, j;
    double sampleIncrement = (Frequency * (M_PI*2)) / SamplesPerSecond;
    
    double theta = (InitialTheta!=NULL) ? *InitialTheta : 0;
    
    //allocate a buffer
    *Buffer = (float*)malloc( BufferLengthInSamples * ChannelCount * sizeof(float));
    
    for(i = 0; i<BufferLengthInSamples; i++)
    {
        
        double sinValue = Amplitude * sin(theta);
        for(j=0;j<ChannelCount;j++)
        {
            (*Buffer)[i*ChannelCount+j] = (float)sinValue;
        }
        theta+=sampleIncrement;
    }
    
    if( InitialTheta != NULL )
    {
        *InitialTheta = theta;
    }
    
}


#endif /* pba_dsp_waveforms_h */
