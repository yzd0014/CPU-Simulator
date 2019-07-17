//
//  pipline.h
//  cpu_simulator
//
//  Created by 戴一通 on 9/22/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//

#ifndef pipline_h
#define pipline_h
#include "data_type.h"
#include "struct_library.h"

void WB(struct latch ex_wb, reg_word *reg, double *freg, struct scoreboard *scb, int *userMode, int *PC);
struct latch EX(struct latch ro_ex, struct miniBuffer *FU_integer, struct miniBuffer *FU_adder, struct miniBuffer *FU_mult, mem_data *data_seg, struct scoreboard *scb);
struct latch RO(struct scoreboard *scb, reg_word *reg, double *freg);
void IF(mem_word *code_seg, int *PC, struct scoreboard *scb, struct latch *result);
void initialize_scb(struct scoreboard *scb);

#endif /* pipline_h */
