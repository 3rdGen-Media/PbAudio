//
//  CMidiControl.h
//  MCUCommander
//
//  Created by Joe Moulton on 12/5/23.
//

#ifndef CMidiControl_h
#define CMidiControl_h

#include "stdint.h"

//we need some sort of header for the CFEnum definititions
#ifdef CF_ENUM
#define CM_ENUM CF_ENUM
#else
#define CM_ENUM(x, y) enum y
#endif

//Abstract Hardware Control Definitions
typedef uint8_t  CMControlIndex;
typedef int8_t   CMControlState;

enum CMControlDirectionEnum
{
    CMControlIncrement             =   0x00,
    CMControlDecrement             =   0x40
};


enum CMControlStepEnum
{
    CMControlStep1                 =   0x01,
    CMControlStep2                 =   0x04,
    CMControlStep3                 =   0x08,
    CMControlStep4                 =   0x0c,
    CMControlStep5                 =   0x0f
};

enum CMControlTypeEnum
{
    CMControlTypeEncoder        =  0xB0,
    CMControlTypeFader          =  0xE0,
    CMControlTypeSwitch         =  0x90,
    CMControlTypeButtonGroup    =  0xA0,
    CMControlTypeUnknown        =  0xF0
};

enum CMEncdoderControlModeEnum
{
    CMControlModeSingleDot      =  0x00,
    CMControlModeBoostCut       =  0x01,
    CMControlModeWrap           =  0x02,
    CMControlModeSpread         =  0x03
};

enum CMButtonControlModeEnum
{
    CMButtonControlModeDefault  =  0x00,
    CMButtonControlModeHold     =  0x01
};

extern const int    cm_control_mode_list_size;
extern const char * cm_control_mode_names[];


typedef uint8_t CMControlDirection;
typedef uint8_t CMControlStep;
typedef uint8_t CMControlType;
typedef uint8_t CMControlMode;

typedef struct CMControl
{
    //Control Display
    char               scribble[2][8];

    //Core Control Definition
    union
    {
        uint16_t       expression; //high res velocity
        uint8_t        velocity, velocity2;
    };
    
    CMControlType       type;
    int8_t              state;
    uint8_t             index;

    //Control Commands
    uint8_t             commandBuffer;

    //Control Rulesets
    CMControlMode       mode;
    uint16_t            min, max;

}CMControl;

typedef struct CMDisplay
{
    char        scribble[2][64];   //2 led strips of length 55
    CMControl * controls[8];       //pointers to corresponding connected control
    uint8_t     index;
}CMDisplay;

double CMControlUpdate(CMControl * control, CMControlDirection controlDirection, CMControlStep controlStep);
void CMControlModeMessage(CMControl * control, CMControlIndex controlIndex, uint8_t * txMessage, uint8_t * txMessageSize);

#endif /* CMidiControl_h */
