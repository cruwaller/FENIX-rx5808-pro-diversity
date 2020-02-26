/* 
    esp-now setup for communicating to https://github.com/AlessandroAU/ExpressLRS
    
    3 bytes transferred {group, task, data}
*/

#ifndef EXPRESSLRS_PROTOCOL_H
#define EXPRESSLRS_PROTOCOL_H

enum group {
    ExLRS = 0,
    VTx = 1
};

enum task {
    // ExLRS
    setMode = 0,
    setTlm = 1,
    setPower = 2,

    // VTx
    setChannelBand = 0
};

#endif