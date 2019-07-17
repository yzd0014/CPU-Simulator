//
//  global_data.h
//  cpu_simulator
//
//  Created by 戴一通 on 9/23/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//

#ifndef global_data_h
#define global_data_h
#include "data_type.h"
#include "struct_library.h"

# define fast 1
# define slow 0
# define yes 1
# define no 0
# define dataLength 5000
# define codeLength 100
# define sourceLength 800
# define Integer 1
# define Adder 2
# define Mult 3
# define Mem 4

extern int BC_a, BC_i, BC_t, BC_m;//buffer counter
extern int pointer;//always point to the current free instruciton buffer
extern mem_word buffer[200];//buffer instructions at the 1st stage
extern int simMode;//allow use to choose mode
extern int IC;//counter of number of instructions
extern int nopTest;//test if there is a nop

extern struct scoreboard scb;
extern struct miniBuffer FU_integer[2], FU_adder[2], FU_mult[6], load_store, fui[10], fua[10], fum[10], fumem[10];
extern struct latch if_ro, ro_ex, ex_wb;
#endif /* global_data_h */

