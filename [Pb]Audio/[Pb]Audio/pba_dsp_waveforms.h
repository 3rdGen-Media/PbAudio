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
