//
//  pipe_component.c
//  cpu_simulator
//
//  Created by 戴一通 on 9/22/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//
#include "pipe_component.h"
#include "data_type.h"
#include "struct_library.h"
#include "global_data.h"

void reset_scb(struct scoreboard *scb, int FU_type){//when instruction release its FU, totally reset its scoreboard
    switch(FU_type){
        case Adder:
            scb->adder.Busy=no;
            scb->adder.Fi = -1;
            scb->adder.Fj = -1;
            scb->adder.Fk = -1;
            scb->adder.Qj = -1;
            scb->adder.Rk = -1;
            break;
        case Integer:
            scb->integer.Busy = no;
            scb->integer.Fi = -1;
            scb->integer.Fj = -1;
            scb->integer.Fk = -1;
            scb->integer.Qj = -1;
            scb->integer.Qk = -1;
            break;
        case Mult:
            scb->mult.Busy = no;
            scb->mult.Fi = -1;
            scb->mult.Fj = -1;
            scb->mult.Fk = -1;
            scb->mult.Qj = -1;
            scb->mult.Qk = -1;
            break;
        case Mem:
            scb->mem.Busy = no;
            scb->mem.Fi = -1;
            scb->mem.Fj = -1;
            scb->mem.Fk = -1;
            scb->mem.Qj = -1;
            scb->mem.Qk = -1;
            break;
            
    }
}

int noWAR(struct scoreboard *scb, int dr){//dr stands for destiantion register
    int output = no;
    if((scb->adder.Fj!=dr||scb->adder.Rj==no)&&
       (scb->integer.Fj!=dr||scb->integer.Rj==no)&&
       (scb->mult.Fj!=dr||scb->mult.Rj==no)&&
       (scb->mem.Fj!=dr||scb->mem.Rj==no)&&
       (scb->adder.Fk!=dr||scb->adder.Rk==no)&&
       (scb->integer.Fk!=dr||scb->integer.Rk==no)&&
       (scb->mult.Fk!=dr||scb->mult.Rk==no)&&
       (scb->mem.Fk!=dr||scb->mem.Rk==no)){
        output = yes;
    }
    
    return output;
}
void scb_check(struct scoreboard *scb, int FU){
    if(scb->adder.Qj == FU){scb->adder.Rj = yes;}
    if(scb->adder.Qk == FU){scb->adder.Rk = yes;}
    
    if(scb->integer.Qj == FU){scb->integer.Rj = yes;}
    if(scb->integer.Qj == FU){scb->integer.Rj = yes;}
    
    if(scb->mult.Qj == FU){scb->mult.Rj = yes;}
    if(scb->mult.Qj == FU){scb->mult.Rj = yes;}
    
    if(scb->mem.Qj == FU){scb->mem.Rj = yes;}
    if(scb->mem.Qj == FU){scb->mem.Rj = yes;}
}

void datamove(struct miniBuffer *input){
    for(int i=0;i<9;i++){
        input[i] = input[i+1];
    }
}
void flush(mem_word *input){//flush buffer in 1st stage if branch happens
    for(int i = 0;i<20; i++){
        input[i] = 0;
    }
}
void buffer_action(mem_word *buffer){
    for(int i=0;i<19;i++){
        buffer[i]=buffer[i+1];
    }
}
void initialize_latch(struct latch *input){
    input->adderBuffer.opcode = 0;
    input->integerBuffer.opcode = 0;
    input->multBuffer.opcode = 0;
    input->memBuffer.opcode = 0;
}
