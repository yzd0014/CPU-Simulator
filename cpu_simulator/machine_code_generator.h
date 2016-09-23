//
//  machine_code_generator.h
//  cpu_simulator
//
//  Created by 戴一通 on 9/23/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//

#ifndef machine_code_generator_h
#define machine_code_generator_h
#include "data_type.h"

void loader(char *a);
void getData(char *sorceCode, mem_data *data_seg);
void getCode(char *sorceCode, mem_word *code_seg);
#endif /* machine_code_generator_h */
