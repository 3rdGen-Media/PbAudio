//
//  CMidiKeyboard.h
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/16/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#ifndef CMidiKeyboard_h
#define CMidiKeyboard_h

//FREQUENCY DEFINITIONS

static const float CENT_FREQ_SCALAR = 1.00057778951;

static const float AUDIBLE_LOGARITHMIC_FREQUENCIES[11] =
{
    20.00,
    40.00,
    80.00,
    160.0,
    320.0,
    640.0,
    1280.0,
    2560.0,
    5120.0,
    10240.0,
    20480.0
};

// freq definitions from A0 to C8, putting in tables is much cheaper than calculating 12th root
static const float WESTERN_NOTE_FREQUENCIES[95] =
{
    27.50,
    29.14,
    30.87,
    32.70,
    34.65,
    36.71,
    38.89,
    41.20,
    43.65,
    46.25,
    49.00,
    51.91,
    55.00,
    58.27,
    61.74,
    65.41,
    69.30,
    73.42,
    77.78,
    82.41,
    87.31,
    92.50,
    98.00,
    103.83,
    110.00,
    116.54,
    123.47,
    130.81,
    138.59,
    146.83,
    155.56,
    164.81,
    174.61,
    185.00,
    196.00,
    207.65,
    220.00,
    233.08,
    246.94,
    261.63,
    277.18,
    293.66,
    311.13,
    329.63,
    349.23,
    369.99,
    392.00,
    415.30,
    440.00,
    466.16,
    493.88,
    523.25,
    554.37,
    587.33,
    622.25,
    659.26,
    698.46,
    739.99,
    783.99,
    830.61,
    880.00,
    932.33,
    987.77,
    1046.50,
    1108.73,
    1174.66,
    1244.51,
    1318.51,
    1396.91,
    1479.98,
    1567.98,
    1661.22,
    1760.00,
    1864.66,
    1975.53,
    2093.00,
    2217.46,
    2349.32,
    2489.02,
    2637.02,
    2793.83,
    2959.96,
    3135.96,
    3322.44,
    3520.00,
    3729.31,
    3951.07,
    4186.01,
    4434.92,
    4698.64,
    4978.03,
    5274.04 ,
    5587.65 ,
    5919.91 ,
    6271.93
};

static const float MAX_NOTE_FREQ = 4186.01;

static const char* WESTERN_NOTE_NAMES[95] =
{
    "A0",
    "A#0",
    "B0",
    "C1",
    "C#1",
    "D1",
    "D#0",
    "E1",
    "F1",
    "F#1",
    "G1",
    "G#1",
    "A1",
    "A#1",
    "B1",
    "C2",
    "C#2",
    "D2",
    "D#2",
    "E2",
    "F2",
    "F#2",
    "G2",
    "G#2",
    "A2",
    "A#2",
    "B2",
    "C3",
    "C#3",
    "D3",
    "D#3",
    "E3",
    "F3",
    "F#3",
    "G3",
    "G#3",
    "A3",
    "A#3",
    "B3",
    "C4",
    "C#4",
    "D4",
    "D#4",
    "E4",
    "F4",
    "F#4",
    "G4",
    "G#4",
    "A4",
    "A#4",
    "B4",
    "C5",
    "C#5",
    "D5",
    "D#5",
    "E5",
    "F5",
    "F#5",
    "G5",
    "G#5",
    "A5",
    "A#5",
    "B5",
    "C6",
    "C#6",
    "D6",
    "D#6",
    "E6",
    "F6",
    "F#6",
    "G6",
    "G#6",
    "A6",
    "A#6",
    "B6",
    "C7",
    "C#7",
    "D7",
    "D#7",
    "E7",
    "F7",
    "F#7",
    "G7",
    "G#7",
    "A7",
    "A#7",
    "B7",
    "C8"
};

//Define Opaque OS Kernel Event Queue Data Type for Queue Mechanisms exposed by the OS Kernel
#ifdef CR_TARGET_WIN32
//For Win32 Interprocess Events, there is no queue manually created or exposed with an event that can be passed and triggered/waited on between processes as with BSD kqueue + pipe combination.
//For Win32 Interthread Messages/Events, every thread comes with a dedicated event queue "under the hood" in the kernel.
//The handles to these thread event/message queues themselves are not exposed as they can be identified and sent to by their corresponding thread address stored as unsigned int
//Thread "Messages"/Events can be retrieved from a thread's manually implemented run loop by calling the GetMessage/PeekMessage API ust like on the Win32 platform main thread message/event loop message pump.
typedef unsigned int  cm_thread_queue_id;  //On Win32, a platform thread queue id is the same as a platform thread id is the same as...
typedef unsigned int  cm_kernel_queue_id;  //a platform kernel queue id
#elif defined(__APPLE__)
typedef int           cm_kernel_queue_id;  //on BSD platforms kernel queues are just kqueues, and kqueue handles are just file descriptors
#endif

extern cm_kernel_queue_id CMTriggerEventQueue; //a global kqueue singleton for the main Core Midi Event Loop to IPC with real-time audio threads


//END FREQUENCY DEFINITIONS

#endif /* CMidiKeyboard_h */
