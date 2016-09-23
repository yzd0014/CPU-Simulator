//
//  struct_library.h
//  cpu_simulator
//
//  Created by 戴一通 on 9/22/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//

#ifndef struct_library_h
#define struct_library_h
#include <stdio.h>
#include "data_type.h"

struct FU_status{
    int Busy;
    int Op;
    int Fi;
    int Fj;
    int Fk;
    int Qj;
    int Qk;
    int Rj;
    int Rk;
};
struct scoreboard{
    struct FU_status integer;
    struct FU_status adder;
    struct FU_status mult;
    struct FU_status mem;
    int re_status[32];
};
struct miniBuffer{
    uint16_t opcode;
    int rs, rt, rd;
    double A, B;
    double ALUout;
    int counter;
};
struct latch{
    struct miniBuffer integerBuffer, adderBuffer, multBuffer, memBuffer;
};
#endif /* struct_library_h */
