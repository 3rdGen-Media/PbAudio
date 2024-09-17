//
//  CMidiControl.c
//  MCUCommander
//
//  Created by Joe Moulton on 12/7/23.
//

#include "CMidiControl.h"
#include "assert.h"

const int cm_control_mode_list_size = 4; //excluding Unknown for now
const char * cm_control_mode_names[] =
{
    //"Undefined",
    "Single Dot",
    "Boost/Cut",
    "Wrap",
    "Spread"
};


double CMControlUpdate(CMControl * control, CMControlDirection controlDirection, CMControlStep controlStep)
{
    //copy control state
    CMControlState controlState = control->state;
    
    //update the control state
    controlDirection ? (controlState -= controlStep) : (controlState += controlStep);
    if( controlState < 0 ) controlState += 32;
    else if( controlState > 31 ) controlState -= 32;
    
    //Update the state of the encoder knob so it can be picked up for consumption by the UI during draw
    control->state = controlState;

    //Determine the current midi value based on the control's rulesets
    uint16_t max = control->max > 0 ? control->max : 127;
    if( controlDirection )
    {
        if( (control->velocity2 - controlStep) < control->min )
            control->velocity2 = 0;
        else
            control->velocity2 -= controlStep;
    }
    else
    {
        if( (control->velocity2 + controlStep) > max )
            control->velocity2 = max;
        else
            control->velocity2 += controlStep;
    }
    
    //return normalized value
    return (double)control->velocity2 / (double)max;
}


void CMControlModeMessage(CMControl * control, CMControlIndex controlIndex, uint8_t * txMessage, uint8_t * txMessageSize)
{
    uint16_t min = control->min;
    uint16_t max = (control->max > 0 ) ? control->max : 127;
    double ledValue = (double)control->velocity2 / (double)max;
    uint8_t ledByte = (uint8_t)( ledValue * 11.0 );
    
    //sending a value of 0 to the encoder means that the v-pot will not display an led
    //but we still want 0 to display as the minimum led value
    if (ledByte == 0 ) ledByte = 0x01;
    
    *txMessageSize = 0;
    //_txMessageType = kMIDIMessageTypeChannelVoice1;

    if(control->mode == CMControlModeSingleDot) //single LED light ranged from min to max.
    {
        //Construct a V-POT Rotary Encoder LED message to send back to the device
        txMessage[0] = (1 << 6) | (control->mode << 4) | ledByte;
        txMessage[1] = 0x30 | controlIndex;
        txMessage[2] = 0xB0;
        txMessage[3] = 0x20;//packet->words[3];
        *txMessageSize = 4;//sizeof(beWord);
    }
    else if(control->mode == CMControlModeBoostCut) //multiple LEDs starting at the center top, traveling down the left or right side. Commonly used for pan effects.
    {
        int startLED = 6;
        int endLED   = 6;
        
        if( ledValue >= 0.5)  //Boost
        {
            startLED = ledByte - 1;
            endLED   = ledByte + 1;
            if(startLED < 6) startLED = 6;
            
            //resignal only the last two leds indicating the current value
            //as all the others below have already been previously signaled
            for( int ledIndex = startLED; ledIndex < endLED; ledIndex++ )
            {
                uint8_t nextLedByte = ledIndex;
                
                txMessage[ *txMessageSize + 0] = (1 << 6) | (control->mode << 4) | nextLedByte;
                txMessage[ *txMessageSize + 1] = 0x30 | controlIndex;
                txMessage[ *txMessageSize + 2] = 0xB0;
                txMessage[ *txMessageSize + 3] = 0x20;
                *txMessageSize += 4;//sizeof(beWord);
            }
        }
        else if( ledValue < 0.5 )    //Cut
        {
            startLED = ledByte + 1;
            endLED = ledByte;
            if( startLED >=6 ) startLED = endLED;
            
            //resignal only the last two leds indicating the current value
            //as all the others below have already been previously signaled
            for( int ledIndex = startLED; ledIndex >= endLED; ledIndex-- )
            {
                uint8_t nextLedByte = ledIndex;
                
                txMessage[ *txMessageSize + 0] = (1 << 6) | (control->mode << 4) | nextLedByte;
                txMessage[ *txMessageSize + 1] = 0x30 | controlIndex;
                txMessage[ *txMessageSize + 2] = 0xB0;
                txMessage[ *txMessageSize + 3] = 0x20;
                *txMessageSize += 4;//sizeof(beWord);
            }
            
        }
        else assert(1==0);
        
    }
    else if(control->mode == CMControlModeWrap) //similar to Dot except multiple LEDs fill to the current position
    {
        //resignal only the last two leds indicating the current value
        //as all the others below have already been previously signaled
        for( int ledIndex = ledByte - 1; ledIndex < ledByte+1; ledIndex++ )
        {
            uint8_t nextLedByte = ledIndex;
            
            txMessage[ *txMessageSize + 0] = (1 << 6) | (control->mode << 4) | nextLedByte;
            txMessage[ *txMessageSize + 1] = 0x30 | controlIndex;
            txMessage[ *txMessageSize + 2] = 0xB0;
            txMessage[ *txMessageSize + 3] = 0x20;
            *txMessageSize += 4;//sizeof(beWord);
        }
    }
    else if(control->mode == CMControlModeSpread) //similar to Boost/Cut except LEDs on left and right are symmetric
    {
        int startLED = 6;
        int endLED   = 6;
                            
        if( ledValue >= 0.5)  //Boost
        {
            startLED = ledByte - 1;
            endLED   = ledByte + 1;
            if(startLED < 6) startLED = 6;

            //spread requires mapping to range 1-6 instead of 1-11
            startLED -= 5;
            endLED -= 5;
            
            //resignal only the last two leds indicating the current value
            //as all the others below have already been previously signaled
            for( int ledIndex = startLED; ledIndex < endLED; ledIndex++ )
            {
                uint8_t nextLedByte = ledIndex;
                
                txMessage[ *txMessageSize + 0] = (1 << 6) | (control->mode << 4) | nextLedByte;
                txMessage[ *txMessageSize + 1] = 0x30 | controlIndex;
                txMessage[ *txMessageSize + 2] = 0xB0;
                txMessage[ *txMessageSize + 3] = 0x20;
                *txMessageSize += 4;//sizeof(beWord);
            }
        }
        else if( ledValue < 0.5 )    //Cut
        {
            //when cutting we are already within 1-6 range but we need to invert
            //so we add 6 to match the Boost conditional
            startLED = (12 - ledByte) - 1;
            endLED   = (12 - ledByte) + 1;
            if(startLED < 6) startLED = 6;

            //spread requires mapping to range 1-6 instead of 1-11
            startLED -= 5;
            endLED -= 5;
            
            //resignal only the last two leds indicating the current value
            //as all the others below have already been previously signaled
            for( int ledIndex = startLED; ledIndex < endLED; ledIndex++ )
            {
                uint8_t nextLedByte = ledIndex;
                
                txMessage[ *txMessageSize + 0] = (1 << 6) | (control->mode << 4) | nextLedByte;
                txMessage[ *txMessageSize + 1] = 0x30 | controlIndex;
                txMessage[ *txMessageSize + 2] = 0xB0;
                txMessage[ *txMessageSize + 3] = 0x20;
                *txMessageSize += 4;//sizeof(beWord);
            }
        }
        else assert(1==0);
    }
    else assert(1==0);
    
    
}
