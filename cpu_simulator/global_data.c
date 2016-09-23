//
//  global_data.c
//  cpu_simulator
//
//  Created by 戴一通 on 9/23/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//
#include "global_data.h"
#include "data_type.h"
#include "struct_library.h"

int BC_a = 0, BC_i=0, BC_t=0, BC_m=0;//buffer counter
int pointer = 0;//always point to the current free instruciton buffer
mem_word buffer[200];//buffer instructions at the 1st stage
int simMode;//allow use to choose mode
int IC=0;//counter of number of instructions
int nopTest = yes;//test if there is a nop
struct scoreboard scb;
struct miniBuffer FU_integer[2], FU_adder[2], FU_mult[6], load_store, fui[10], fua[10], fum[10], fumem[10];
struct latch if_ro, ro_ex, ex_wb;
