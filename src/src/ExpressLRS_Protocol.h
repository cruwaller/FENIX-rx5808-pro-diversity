/* 
    esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS
    
    3 bytes transferred {group, task, data}
*/

#ifndef EXPRESSLRS_PROTOCOL_H
#define EXPRESSLRS_PROTOCOL_H

#define FUNC_Ex             0x4578 // 'Ex'

// ExLRS Functions
#define ExLRS_MODE          0x00
#define ExLRS_TX_POWER      0x01
#define ExLRS_TLM           0x02

// Modes
#define ExLRS_50Hz          2
#define ExLRS_100Hz         1
#define ExLRS_200Hz         0

// TLM
#define ExLRS_TLM_OFF       0
#define ExLRS_TLM_ON        1

#endif