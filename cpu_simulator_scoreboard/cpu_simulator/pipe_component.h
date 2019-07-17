//
//  pipe_component.h
//  cpu_simulator
//
//  Created by 戴一通 on 9/22/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//

#ifndef pipe_component_h
#define pipe_component_h
#include "struct_library.h"

void reset_scb(struct scoreboard *scb, int FU_type);
int noWAR(struct scoreboard *scb, int dr);
void scb_check(struct scoreboard *scb, int FU);
void datamove(struct miniBuffer *input);
void flush(mem_word *input);
void buffer_action(mem_word *buffer);
void initialize_latch(struct latch *input);
#endif /* pipe_component_h */
