//
//  main.c
//  cpu_simulator
//
//  Created by 戴一通 on 9/22/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//

#include <stdio.h>
#include "data_type.h"
#include "struct_library.h"
#include "global_data.h"
#include "pipeline.h"
#include "machine_code_generator.h"

int main(int argc, const char * argv[]) {
    // insert code here...
    char sorceCode[sourceLength];
    mem_data data_seg[dataLength];
    mem_word code_seg[codeLength];
    reg_word reg[32];//32 integer registers
    double freg[32];//16 float point registers
   
    freg[17]=0;
    freg[18]=-0.57;
    freg[30]=1.65;
    
    freg[20]=0;
    freg[21]=-0.92;
    freg[31]=0.8;
    
    for(int i = 0; i< dataLength; i++){
        data_seg[i]= 0.00;
    }//initialize data segment
    for(int i = 0; i< codeLength; i++){
        code_seg[i]=10;
    }//initialize code segment
    for(int i = 0; i<32; i++){
        reg[i]=0;
    }//initialize register
    initialize_scb(&scb);//initialize scoreboard
    
    loader(sorceCode);
    getData(sorceCode, data_seg);
    getCode(sorceCode, code_seg);
    
    int PC = 0;
    int userMode = 1;
    int C=0;
    int NOP=0;
    
    printf("Please enter which mode you want to use(0 is standard mode/1 is performance mode):\n");
    scanf("%d", &simMode);
    
    while(userMode == 1){
        C++;
        WB(ex_wb, reg, freg, &scb, &userMode, &PC);
        ex_wb = EX(ro_ex, FU_integer, FU_adder, FU_mult, data_seg, &scb);
        ro_ex = RO(&scb, reg, freg);
        IF(code_seg, &PC, &scb, &if_ro);
        
        if(nopTest == yes){
            NOP++;
        }
        nopTest = yes;
    }
    printf("\nC = %d\n", C);
    printf("IC = %d\n", IC);
    printf("number of nops: %d\n", NOP);
    
    return 0;
}
