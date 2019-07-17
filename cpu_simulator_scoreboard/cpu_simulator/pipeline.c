//
//  pipline.c
//  cpu_simulator
//
//  Created by 戴一通 on 9/22/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//
#include "pipeline.h"
#include "data_type.h"
#include "struct_library.h"
#include "global_data.h"
#include "pipe_component.h"

void WB(struct latch ex_wb, reg_word *reg, double *freg, struct scoreboard *scb, int *userMode, int *PC){
    
    if(ex_wb.adderBuffer.opcode!=0){//when an adder instruction come it will enter its buffer
        if(simMode == slow){reset_scb(scb, Adder);}
        fua[BC_a] = ex_wb.adderBuffer;//move date from latch to buffer
        ex_wb.adderBuffer.opcode = 0;//departure of instruction
        BC_a++;
    }
    switch (fua[0].opcode) {
        case 12:
        case 13:
            if(noWAR(scb, fua[0].rd)==yes){//make sure no waw
                scb_check(scb, Adder);//update scoreboard to inform RAW hazard
                freg[fua[0].rd]= fua[0].ALUout;// write back
                scb->re_status[fua[0].rd] = 0;//update socreboard for register status
                
                datamove(fua);//pull all data one element in final stage's buffer
                BC_a--;//upata final stage's buffer pointer
                nopTest=no;//indicate one instruction is finished in crruent cycle
            }
            break;
    }
    
    if(ex_wb.integerBuffer.opcode==6||ex_wb.integerBuffer.opcode==7||
       ex_wb.integerBuffer.opcode==8||ex_wb.integerBuffer.opcode==9){
        if(simMode == slow){reset_scb(scb, Integer);}
        if(ex_wb.integerBuffer.ALUout != 0){//finishing of branch
            *PC = ex_wb.integerBuffer.ALUout;
            pointer = 0;
            flush(buffer);
            nopTest=no;
        }
        
    }
    if(ex_wb.multBuffer.opcode == 10){//system call will be finished here
        *userMode = 0;
        nopTest=no;
    }
    if(ex_wb.integerBuffer.opcode!=0&&ex_wb.integerBuffer.opcode!=6&&ex_wb.integerBuffer.opcode!=7&&
       ex_wb.integerBuffer.opcode!=8&&ex_wb.integerBuffer.opcode!=9){//when an initeger instruciton come, it will enter buffer
        if(simMode == slow){reset_scb(scb, Integer);}
        fui[BC_i] = ex_wb.integerBuffer;
        ex_wb.integerBuffer.opcode = 0;
        BC_i++;
    }
    switch(fui[0].opcode){
        case 1:
        case 2:
            if(noWAR(scb, fui[0].rs)==yes){
                scb_check(scb, Integer);
                reg[fui[0].rs] = fui[0].ALUout;
                scb->re_status[fui[0].rs] = 0;
                
                datamove(fui);
                BC_i--;
                nopTest=no;
            }
            break;
        case 11:
            if(noWAR(scb, fui[0].rd)){
                scb_check(scb, Integer);
                reg[fui[0].rd] = fui[0].ALUout;
                scb->re_status[fui[0].rd] = 0;
                
                datamove(fui);
                BC_i--;
                nopTest=no;
            }
            break;
    }
    
    if(ex_wb.multBuffer.opcode!=0){
        if(simMode == slow){reset_scb(scb, Mult);}
        fum[BC_t] = ex_wb.multBuffer;
        ex_wb.multBuffer.opcode = 0;
        BC_t++;
    }
    switch(fum[0].opcode){
        case 14:
            if(noWAR(scb, fum[0].rd)==yes){
                scb_check(scb, Mult);
                freg[fum[0].rd] = fum[0].ALUout;
                scb->re_status[fum[0].rd] = 0;
                
                datamove(fum);
                BC_t--;
                nopTest=no;
            }
            break;
    }
    
    if(ex_wb.memBuffer.opcode!=0){
        fumem[BC_m] = ex_wb.memBuffer;
        ex_wb.memBuffer.opcode = 0;
        BC_m++;
    }
    switch(fumem[0].opcode){
        case 3:
        case 4:
        case 16:
            if(noWAR(scb, fumem[0].rs)==yes){
                scb_check(scb, Mem);
                freg[fumem[0].rs] = fumem[0].ALUout;
                scb->re_status[fumem[0].rs] = 0;
                
                datamove(fumem);
                BC_m--;
                nopTest=no;
            }
            break;
        case 5:
            if(noWAR(scb, fumem[0].rs)==yes){
                scb_check(scb, Mem);
                if(fumem[0].rs>15){
                    freg[fumem[0].rs] = fumem[0].ALUout;
                }
                else{
                    reg[fumem[0].rs] = fumem[0].ALUout;
                }
                scb->re_status[fumem[0].rs] = 0;
                
                datamove(fumem);
                BC_m--;
                nopTest=no;
            }
            break;
    }
}

struct latch EX(struct latch ro_ex, struct miniBuffer *FU_integer, struct miniBuffer *FU_adder, struct miniBuffer *FU_mult,
                mem_data *data_seg, struct scoreboard *scb){
    struct latch result;
    initialize_latch(&result);
    
    result.adderBuffer = FU_adder[0];//pipline in pipline, it's pull model
    FU_adder[0] = ro_ex.adderBuffer;//use a media to implement two stages pipline
    switch(ro_ex.adderBuffer.opcode){
        case 12://because it's pull model so calculation happend at last in the method
            FU_adder[0].ALUout = ro_ex.adderBuffer.A + ro_ex.adderBuffer.B;
            ro_ex.adderBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, Adder);}//release function unit
            break;
        case 13:
            FU_adder[0].ALUout = ro_ex.adderBuffer.A - ro_ex.adderBuffer.B;
            ro_ex.adderBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, Adder);}//release function unit
            break;
            
    }
    
    result.integerBuffer = FU_integer[0];
    FU_integer[0] = ro_ex.integerBuffer;
    switch(ro_ex.integerBuffer.opcode){
        case 1:
            FU_integer[0].ALUout = ro_ex.integerBuffer.A + ro_ex.integerBuffer.B;
            ro_ex.integerBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
        case 2:
            FU_integer[0].ALUout = ro_ex.integerBuffer.A - ro_ex.integerBuffer.B;
            ro_ex.integerBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
        case 11:
            FU_integer[0].ALUout = ro_ex.integerBuffer.A + ro_ex.integerBuffer.B;
            ro_ex.integerBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
        case 6:
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
        case 7:
            if(ro_ex.integerBuffer.A != 0){FU_integer[0].ALUout = 0;}
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
        case 8:
            if(ro_ex.integerBuffer.A < ro_ex.integerBuffer.B){FU_integer[0].ALUout = 0;}
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
        case 9:
            if(ro_ex.integerBuffer.A == ro_ex.integerBuffer.B){FU_integer[0].ALUout = 0;}
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, Integer);}
            break;
    }
    
    
    result.multBuffer = FU_mult[4];
    for(int i=4; i>=1; i--){
        FU_mult[i]=FU_mult[i-1];
    }
    FU_mult[0] = ro_ex.multBuffer;
    if(ro_ex.multBuffer.opcode == 14){
        FU_mult[0].ALUout = ro_ex.multBuffer.A * ro_ex.multBuffer.B;
        ro_ex.multBuffer.opcode = 0;
        if(simMode == fast){reset_scb(scb, Mult);}
    }
    
    
    if(ro_ex.memBuffer.opcode == 3||ro_ex.memBuffer.opcode == 4||ro_ex.memBuffer.opcode==5||
       ro_ex.memBuffer.opcode==15||ro_ex.memBuffer.opcode==16){
        load_store = ro_ex.memBuffer;// deal with all memory access instructions
    }
    switch(load_store.opcode){
        case 3:
            load_store.counter--;//counter have been set 2 at 2nd stage in the last clock cycle
            if(load_store.counter == 0)//execution will delay 2 clock cycles
            {result.memBuffer = load_store; reset_scb(scb, Mem); load_store.opcode = 0;}
            break;
        case 4:
            load_store.counter--;
            if(load_store.counter == 0)
            {   result.memBuffer = load_store;//once instrucion finished, transfer data to the next latch
                result.memBuffer.ALUout = data_seg[(int)load_store.A];//load
                reset_scb(scb, Mem);//release FU
                load_store.opcode = 0;
            }
            break;
        case 5:
            load_store.counter--;
            if(load_store.counter == 0)
            {result.memBuffer = load_store; reset_scb(scb, Mem); load_store.opcode = 0;}
            break;
        case 15:
            load_store.counter--;
            if(load_store.counter == 0){
                data_seg[(int)load_store.B] = load_store.A;
                reset_scb(scb, Mem); load_store.opcode = 0;
                nopTest=no;//store will be finished here so indicate at least there is one instruction finished in this cycle
            }
            break;
        case 16:
            load_store.counter--;
            if(load_store.counter == 0)
            {   result.memBuffer = load_store;
                result.memBuffer.ALUout = data_seg[(int)load_store.A];
                reset_scb(scb, Mem); load_store.opcode = 0;
            }
            break;
    }
    
    
    return result;
}

struct latch RO(struct scoreboard *scb, reg_word *reg, double *freg){//read operands
    
    struct latch result;
    initialize_latch(&result);
    switch(if_ro.adderBuffer.opcode){
        case 12:
        case 13:
            if(scb->adder.Rj==yes&&scb->adder.Rk==yes){//check RAW hazard
                result.adderBuffer = if_ro.adderBuffer;//transfer all information from current latch to next latch
                result.adderBuffer.A = freg[if_ro.adderBuffer.rs];//fetch oprands
                result.adderBuffer.B = freg[if_ro.adderBuffer.rt];
                if_ro.adderBuffer.opcode = 0;//reset current latch
                scb->adder.Rj = no;//update scoreboard to inform this register can be written now
                scb->adder.Rk = no;
            }
            break;
    }
    switch(if_ro.integerBuffer.opcode){
        case 1:
        case 2:
            if(scb->integer.Rj==yes){
                result.integerBuffer = if_ro.integerBuffer;
                result.integerBuffer.A = reg[if_ro.integerBuffer.rt];
                if_ro.integerBuffer.opcode = 0;
                scb->integer.Rj=no;
            }
            break;
        case 7:
            if(scb->integer.Rj==yes){
                result.integerBuffer = if_ro.integerBuffer;
                result.integerBuffer.A = reg[if_ro.integerBuffer.rs];
                if_ro.integerBuffer.opcode = 0;
                scb->integer.Rj=no;
            }
            break;
        case 8:
        case 9:
        case 11:
            if(scb->integer.Rj == yes&&scb->integer.Rk == yes){
                result.integerBuffer = if_ro.integerBuffer;
                result.integerBuffer.A = reg[if_ro.integerBuffer.rs];
                result.integerBuffer.B = reg[if_ro.integerBuffer.rt];
                if_ro.integerBuffer.opcode = 0;
                scb->integer.Rj=no;
                scb->integer.Rk=no;
            }
            break;
        case 6:
            result.integerBuffer = if_ro.integerBuffer;
            if_ro.integerBuffer.opcode = 0;
            break;
    }
    switch(if_ro.memBuffer.opcode){
        case 4:
        case 16:
            if(scb->mem.Rj==yes){
                result.memBuffer = if_ro.memBuffer;
                result.memBuffer.A = reg[if_ro.memBuffer.rt];
                result.memBuffer.counter = 2;
                if_ro.memBuffer.opcode = 0;
                scb->mem.Rj=no;
            }
            break;
        case 15:
            if(scb->mem.Rj==yes){
                result.memBuffer = if_ro.memBuffer;
                result.memBuffer.A = freg[if_ro.memBuffer.rs];
                result.memBuffer.B = reg[if_ro.memBuffer.rt];
                result.memBuffer.counter = 2;
                if_ro.memBuffer.opcode = 0;
                scb->mem.Rj=no;
            }
            break;
        case 3:
        case 5:
            result.memBuffer = if_ro.memBuffer;
            result.memBuffer.counter = 2;
            if_ro.memBuffer.opcode = 0;
            break;
            
    }
    switch(if_ro.multBuffer.opcode){
        case 14:
            if(scb->mult.Rj==yes&&scb->mult.Rk==yes){
                result.multBuffer = if_ro.multBuffer;
                result.multBuffer.A = freg[if_ro.multBuffer.rs];
                result.multBuffer.B = freg[if_ro.multBuffer.rt];
                if_ro.multBuffer.opcode=0;
                scb->mult.Rj=no;
                scb->mult.Rk=no;
            }
            break;
        case 10:
            result.multBuffer = if_ro.multBuffer;//system call will be treated like a mult instruction
            if_ro.multBuffer.opcode=0;
    }
    
    return result;
}

void IF(mem_word *code_seg, int *PC, struct scoreboard *scb, struct latch *result){
    buffer[pointer] = code_seg[*PC];
    pointer = pointer + 1;
    *PC = *PC + 1;
    
    switch(buffer[0]>>26){
        case 12:
            if(scb->adder.Busy == no&&scb->re_status[(buffer[0]>>11)%32] == 0){//check structual hazard and WAW hazard
                pointer = pointer - 1;//update pointer for buffer
                scb->adder.Busy = yes;//update scoreboard, adder FU become busy
                scb->adder.Op = 12;//set op code for adder FU in scoreboard
                scb->adder.Fi = (buffer[0]>>11)%32;//set destination register name in scoreboard
                scb->adder.Fj = (buffer[0]>>21)%32;//set input register name in scoreboard
                scb->adder.Fk = (buffer[0]>>16)%32;//set input register name in scoreboard
                
                if(scb->re_status[(buffer[0]>>21)%32]==0){scb->adder.Rj = yes;}//if input is ready then mark in scoreboard
                else{scb->adder.Rj = no;scb->adder.Qj = scb->re_status[(buffer[0]>>21)%32];}//if input is not ready mark which fu will write it
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->adder.Rk = yes;}
                else{scb->adder.Rk = no;scb->adder.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>11)%32]= Adder;//set destination's register status in scoreboard
                result->adderBuffer.rs = (buffer[0]>>21)%32;//transfer instruction's rs name to next latch
                result->adderBuffer.rt = (buffer[0]>>16)%32;//transfer instruction's rt name to next latch
                result->adderBuffer.rd = (buffer[0]>>11)%32;//this name will be used to write data back at last stage
                result->adderBuffer.opcode = 12;//transfer op code to next latch
                
                buffer_action(buffer);//pull all date in buffer
                IC++;//a new instrcution have been issued, update instruction counter
            }
            break;
        case 13:
            if(scb->adder.Busy == no&&scb->re_status[(buffer[0]>>11)%32] == 0){
                pointer = pointer - 1;
                scb->adder.Busy = yes;
                scb->adder.Op = 13;
                scb->adder.Fi = (buffer[0]>>11)%32;
                scb->adder.Fj = (buffer[0]>>21)%32;
                scb->adder.Fk = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32]==0){scb->adder.Rj = yes;}
                else{scb->adder.Rj = no;scb->adder.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->adder.Rk = yes;}
                else{scb->adder.Rk = no;scb->adder.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>11)%32]= Adder;
                result->adderBuffer.rs = (buffer[0]>>21)%32;
                result->adderBuffer.rt = (buffer[0]>>16)%32;
                result->adderBuffer.rd = (buffer[0]>>11)%32;
                result->adderBuffer.opcode = 13;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 14:
            if(scb->mult.Busy == no&&scb->re_status[(buffer[0]>>11)%32] == 0){
                pointer = pointer - 1;
                scb->mult.Busy = yes;
                scb->mult.Op = 14;
                scb->mult.Fi = (buffer[0]>>11)%32;
                scb->mult.Fj = (buffer[0]>>21)%32;
                scb->mult.Fk = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32]==0){scb->mult.Rj = yes;}
                else{scb->mult.Rj = no; scb->mult.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->mult.Rk = yes;}
                else{scb->mult.Rk = no; scb->mult.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>11)%32]= Mult;
                result->multBuffer.rs = (buffer[0]>>21)%32;
                result->multBuffer.rt = (buffer[0]>>16)%32;
                result->multBuffer.rd = (buffer[0]>>11)%32;
                result->multBuffer.opcode = 14;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 15:
            if(scb->mem.Busy == no){
                pointer = pointer - 1;
                scb->mem.Busy = yes;
                scb->mem.Op = 15;
                scb->mem.Fj = (buffer[0]>>21)%32;
                scb->mem.Fk = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32]==0){scb->mem.Rj = yes;}
                else{scb->mem.Rj = no; scb->mem.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->mem.Rk = yes;}
                else{scb->mem.Rk = no; scb->mem.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                result->memBuffer.rs = (buffer[0]>>21)%32;
                result->memBuffer.rt = (buffer[0]>>16)%32;
                result->memBuffer.opcode = 15;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 16:
            if(scb->mem.Busy == no&&scb->re_status[(buffer[0]>>21)%32]==0){
                pointer = pointer - 1;
                scb->mem.Busy = yes;
                scb->mem.Op = 16;
                scb->mem.Fi = (buffer[0]>>21)%32;
                scb->mem.Fj = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->mem.Rj = yes;}
                else{scb->mem.Rj = no; scb->mem.Qj = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>21)%32] = Mem;
                result->memBuffer.rs = (buffer[0]>>21)%32;
                result->memBuffer.rt = (buffer[0]>>16)%32;
                result->memBuffer.opcode = 16;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 6:
            if(scb->integer.Busy == no){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 6;
                
                result->integerBuffer.ALUout = buffer[0]%0x10000;
                result->integerBuffer.opcode = 6;
                buffer_action(buffer);
                IC++;
            }
            break;
        case 7:
            if(scb->integer.Busy == no){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 7;
                scb->integer.Fj = (buffer[0]>>21)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32] == 0){scb->integer.Rj = yes;}
                else{scb->integer.Rj = no;scb->integer.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                result->integerBuffer.rs = (buffer[0]>>21)%32;
                result->integerBuffer.ALUout = buffer[0]%0x10000;
                result->integerBuffer.opcode = 7;
                buffer_action(buffer);
                IC++;
            }
            break;
        case 8:
            if(scb->integer.Busy == no){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 8;
                scb->integer.Fj = (buffer[0]>>21)%32;
                scb->integer.Fk = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32] == 0){scb->integer.Rj = yes;}
                else{scb->integer.Rj = no;scb->integer.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                if(scb->re_status[(buffer[0]>>16)%32] == 0){scb->integer.Rk = yes;}
                else{scb->integer.Rk = no;scb->integer.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                result->integerBuffer.rs = (buffer[0]>>21)%32;
                result->integerBuffer.rt = (buffer[0]>>16)%32;
                result->integerBuffer.ALUout = buffer[0]%0x10000;
                result->integerBuffer.opcode = 8;
                buffer_action(buffer);
                IC++;
            }
            break;
        case 9:
            if(scb->integer.Busy == no){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 9;
                scb->integer.Fj = (buffer[0]>>21)%32;
                scb->integer.Fk = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32] == 0){scb->integer.Rj = yes;}
                else{scb->integer.Rj = no;scb->integer.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                if(scb->re_status[(buffer[0]>>16)%32] == 0){scb->integer.Rk = yes;}
                else{scb->integer.Rk = no;scb->integer.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                result->integerBuffer.rs = (buffer[0]>>21)%32;
                result->integerBuffer.rt = (buffer[0]>>16)%32;
                result->integerBuffer.ALUout = buffer[0]%0x10000;
                result->integerBuffer.opcode = 9;
                buffer_action(buffer);
                IC++;
            }
            break;
        case 2:
            if(scb->integer.Busy == no&&scb->re_status[(buffer[0]>>21)%32]==0){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 2;
                scb->integer.Fi = (buffer[0]>>21)%32;
                scb->integer.Fj = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->integer.Rj = yes;}
                else{scb->integer.Rj = no; scb->integer.Qj = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>21)%32] = Integer;
                result->integerBuffer.rs = (buffer[0]>>21)%32;
                result->integerBuffer.rt = (buffer[0]>>16)%32;
                result->integerBuffer.B = buffer[0]%0x10000;
                result->integerBuffer.opcode = 2;
                
                buffer_action(buffer);
                IC++;
            }
            
            break;
        case 1:
            if(scb->integer.Busy == no&&scb->re_status[(buffer[0]>>21)%32]==0){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 1;
                scb->integer.Fi = (buffer[0]>>21)%32;
                scb->integer.Fj = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->integer.Rj = yes;}
                else{scb->integer.Rj = no; scb->integer.Qj = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>21)%32] = Integer;
                result->integerBuffer.rs = (buffer[0]>>21)%32;
                result->integerBuffer.rt = (buffer[0]>>16)%32;
                result->integerBuffer.B = buffer[0]%0x10000;
                result->integerBuffer.opcode = 1;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 11:
            if(scb->integer.Busy == no&&scb->re_status[(buffer[0]>>11)%32] == 0){
                pointer = pointer - 1;
                scb->integer.Busy = yes;
                scb->integer.Op = 11;
                scb->integer.Fi = (buffer[0]>>11)%32;
                scb->integer.Fj = (buffer[0]>>21)%32;
                scb->integer.Fk = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>21)%32]==0){scb->integer.Rj = yes;}
                else{scb->integer.Rj = no;scb->integer.Qj = scb->re_status[(buffer[0]>>21)%32];}
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->integer.Rk = yes;}
                else{scb->integer.Rk = no;scb->integer.Qk = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>11)%32]= Integer;
                result->integerBuffer.rs = (buffer[0]>>21)%32;
                result->integerBuffer.rt = (buffer[0]>>16)%32;
                result->integerBuffer.rd = (buffer[0]>>11)%32;
                result->integerBuffer.opcode = 11;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 3:
            if(scb->mem.Busy == no&&scb->re_status[(buffer[0]>>21)%32]==0){
                pointer = pointer - 1;
                scb->mem.Busy = yes;
                scb->mem.Op = 3;
                scb->mem.Fi = (buffer[0]>>21)%32;
                
                scb->re_status[(buffer[0]>>21)%32] = Mem;
                result->memBuffer.rs = (buffer[0]>>21)%32;
                result->memBuffer.ALUout = buffer[0]%0x10000;
                result->memBuffer.opcode = 3;
                
                buffer_action(buffer);
                IC++;
            }
            
            break;
        case 4:
            if(scb->mem.Busy == no&&scb->re_status[(buffer[0]>>21)%32]==0){
                pointer = pointer - 1;
                scb->mem.Busy = yes;
                scb->mem.Op = 4;
                scb->mem.Fi = (buffer[0]>>21)%32;
                scb->mem.Fj = (buffer[0]>>16)%32;
                
                if(scb->re_status[(buffer[0]>>16)%32]==0){scb->mem.Rj = yes;}
                else{scb->mem.Rj = no; scb->mem.Qj = scb->re_status[(buffer[0]>>16)%32];}
                
                scb->re_status[(buffer[0]>>21)%32] = Mem;
                result->memBuffer.rs = (buffer[0]>>21)%32;
                result->memBuffer.rt = (buffer[0]>>16)%32;
                result->memBuffer.opcode = 4;
                
                buffer_action(buffer);
                IC++;
            }
            
            break;
        case 5:
            if(scb->mem.Busy == no&&scb->re_status[(buffer[0]>>21)%32]==0){
                pointer = pointer - 1;
                scb->mem.Busy = yes;
                scb->mem.Op = 5;
                scb->mem.Fi = (buffer[0]>>21)%32;
                
                scb->re_status[(buffer[0]>>21)%32] = Mem;
                result->memBuffer.rs = (buffer[0]>>21)%32;
                result->memBuffer.ALUout = buffer[0]%0x10000;
                result->memBuffer.opcode = 5;
                
                buffer_action(buffer);
                IC++;
            }
            break;
        case 0:
            if(scb->integer.Busy == no){
                pointer = pointer - 1;
                buffer_action(buffer);
            }
            break;
        case 10:
            if(scb->integer.Busy==no&&scb->adder.Busy==no&&scb->mult.Busy==no&&scb->mem.Busy==no){
                result->multBuffer.opcode = 10;
                pointer = pointer - 1;
                buffer_action(buffer);
                IC++;
            }
            
    }
    
}
void initialize_scb(struct scoreboard *scb){
    scb->adder.Busy = no;
    scb->adder.Fi = -1;
    scb->adder.Fj = -1;
    scb->adder.Fk = -1;
    scb->adder.Qj = -1;
    scb->adder.Rk = -1;
    
    scb->integer.Busy = no;
    scb->integer.Fi = -1;
    scb->integer.Fj = -1;
    scb->integer.Fk = -1;
    scb->integer.Qj = -1;
    scb->integer.Qk = -1;
    
    scb->mult.Busy = no;
    scb->mult.Fi = -1;
    scb->mult.Fj = -1;
    scb->mult.Fk = -1;
    scb->mult.Qj = -1;
    scb->mult.Qk = -1;
    
    scb->mem.Busy = no;
    scb->mem.Fi = -1;
    scb->mem.Fj = -1;
    scb->mem.Fk = -1;
    scb->mem.Qj = -1;
    scb->mem.Qk = -1;
    
    for(int i=0; i<32;i++){
        scb->re_status[i]=0;
    }
}
