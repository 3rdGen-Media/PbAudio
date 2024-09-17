//
//  CMEventQueue.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/17/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "CMEventQueue.h"

cm_kernel_queue_id CMTriggerEventQueue; //a global kqueue singleton for the main Core Midi Event Loop to IPC with real-time audio threads
