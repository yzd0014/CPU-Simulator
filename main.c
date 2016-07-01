//
//  main.c
//  accumulator
//
//  Created by 戴一通 on 9/6/14.
//  Copyright (c) 2014 ___FULLUSERNAME__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

# define fast 1
# define slow 0

typedef uint32_t mem_address;
typedef uint32_t mem_word;
typedef double mem_data;
typedef int32_t reg_word;

const int yes = 1;
const int no = 0;

const int dataLength = 5000;
const int codeLength = 100;
const int sourceLength = 800;
int BC_a = 0, BC_i=0, BC_t=0, BC_m=0;//buffer counter
int pointer = 0;//always point to the current free instruciton buffer
mem_word buffer[200];//buffer instructions at the 1st stage
int simMode;//allow use to choose mode
int IC=0;//counter of number of instructions
int nopTest = yes;//test if there is a nop

const int integer = 1;
const int adder = 2;
const int mult = 3;
const int mem = 4;

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
} scb;

struct miniBuffer{
    uint16_t opcode;
    int rs, rt, rd;
    double A, B;
    double ALUout;
    int counter;
}FU_integer[2], FU_adder[2], FU_mult[6], load_store, fui[10], fua[10], fum[10], fumem[10];

struct latch{
    struct miniBuffer integerBuffer, adderBuffer, multBuffer, memBuffer;
} if_ro, ro_ex, ex_wb;

void initialize_latch(struct latch *input){
    input->adderBuffer.opcode = 0;
    input->integerBuffer.opcode = 0;
    input->multBuffer.opcode = 0;
    input->memBuffer.opcode = 0;
}

//load data from harddrive to memory
void loader(char *a){
    FILE *fp;
    int i = 0;
    char filePath[50];
    
    printf("Please enter the path of the file: ");
    scanf("%s", filePath);
    getchar();//eat a enter from keyboard
    
    if ((fp = fopen (filePath, "r")) == NULL)
    {
        perror ("File open error!\n");
        exit (1);
    }
    do{
        a[i]=fgetc(fp);
        i++;
    }while(a[i-1]!=EOF);
    
    fclose(fp);
}
//Fetch data from source code and put data into data segment
void getData(char *sorceCode, mem_data *data_seg){
    int m;
    for(int i=2;i<200;i++){
        if(sorceCode[i]=='.'&&sorceCode[i+1]=='a'&&sorceCode[i+2]=='s'){
            m = i+9;
            for(int j = (sorceCode[i-3]-65)*50;j<dataLength;j++){//(variable's asscii code -65)*50=address
                if(sorceCode[m]=='"'){break;}//final " indicats the end of a string
                data_seg[j]=sorceCode[m];
                m++;
            }
        }
        else if(sorceCode[i]=='.'&&sorceCode[i+1]=='t'&&sorceCode[i+2]=='e'&&sorceCode[i+3]=='x'&&sorceCode[i+4]=='t'){
            break;//.test manifest the end of data segment
        }
    }
}
mem_word getName(int i, char *sorceCode, int j){ //j=1 means rs, j=2 means rt, get reigster's name
    mem_word reg_name;//translation work follows the rule of original MIPS.
    
        reg_name = (sorceCode[i]-48)*10 + (sorceCode[i+1]-48);
        if(j == 1){
            reg_name = reg_name << 21;//put name into proper postion of instruciton
        }
        else if(j == 3){reg_name = reg_name << 11;}
        else{
            reg_name = reg_name << 16;
        }
    
    
       return reg_name;
}

int findCode(char *sorceCode, int k){//fetch the character the belongs to code
    int result = 0;
    if((sorceCode[k]=='a'&&sorceCode[k+1]=='d'&&sorceCode[k+2]=='d'&&sorceCode[k+3]=='i'&&sorceCode[k+4]==' ')||
       (sorceCode[k]=='b'&&(sorceCode[k-1]=='\n'||sorceCode[k-1]=='\r')&&sorceCode[k+1]==' ')||
       (sorceCode[k]=='b'&&sorceCode[k+1]=='e'&&sorceCode[k+2]=='q'&&sorceCode[k+3]=='z'&&sorceCode[k+4]==' ')||
       (sorceCode[k]=='b'&&sorceCode[k+1]=='g'&&sorceCode[k+2]=='e'&&sorceCode[k+3]==' ')||
       (sorceCode[k]=='b'&&sorceCode[k+1]=='n'&&sorceCode[k+2]=='e')||
       (sorceCode[k]=='l'&&sorceCode[k+1]=='a')||
       (sorceCode[k]=='l'&&sorceCode[k+1]=='b')||
       (sorceCode[k]=='l'&&sorceCode[k+1]=='i'&&sorceCode[k+2]==' ')||
       (sorceCode[k]=='s'&&sorceCode[k+1]=='u'&&sorceCode[k+2]=='b'&&sorceCode[k+3]=='i')||
       (sorceCode[k]=='s'&&sorceCode[k+1]=='y'&&sorceCode[k+2]=='s')||
       (sorceCode[k]=='a'&&sorceCode[k+1]=='d'&&sorceCode[k+2]=='d'&&sorceCode[k+3]==' '&&sorceCode[k-1]!='f')||
       (sorceCode[k]=='n'&&sorceCode[k+1]=='o'&&sorceCode[k+2]=='p')||
       (sorceCode[k]=='f'&&sorceCode[k+1]=='a'&&sorceCode[k+2]=='d'&&sorceCode[k+3]=='d'&&sorceCode[k+4]==' ')||
       (sorceCode[k]=='f'&&sorceCode[k+1]=='s'&&sorceCode[k+2]=='u'&&sorceCode[k+3]=='b'&&sorceCode[k+4]==' ')||
       (sorceCode[k]=='f'&&sorceCode[k+1]=='m'&&sorceCode[k+2]=='u'&&sorceCode[k+3]=='l'&&sorceCode[k+4]==' ')||
       (sorceCode[k]=='s'&&sorceCode[k+1]=='.'&&sorceCode[k+2]=='d')||
       (sorceCode[k]=='l'&&sorceCode[k+1]=='.'&&sorceCode[k+2]=='d')){
        result = 1;
    }
    return  result;
}

//translate ASCII code into binary machine code
mem_word assembler(char *sorceCode, int i, int j){
    mem_word binaryCode=0;
    mem_address address;
    int counter=0;
    int16_t label=0;
    uint16_t unlabel=0;
    mem_word rs=0, rt=0;
    int trigger=0;
    
    char data1[10];
    int m=0;//initial number of digit of a number
    mem_word data2=0;//data2 holds the final result which is data
    mem_word data3=0;//data3 holds every single digit of a number
    
    if(sorceCode[i]=='a'&&sorceCode[i+3]=='i'){//addi
        binaryCode = 1;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+6, sorceCode, 1) + getName(i+11, sorceCode, 2);
        for(int n=i+14;n<sourceLength;n++){//search the number in assemble instruction
            if((sorceCode[n]>=48 && sorceCode[n]<=57)){//once find a number, start to record every digit
                data1[m]=sorceCode[n];
                m++;
            }
            //once reach the end of a number, translate all digits into one number which is data2
            if(sorceCode[n-1]>=48&&sorceCode[n-1]<=57&&(sorceCode[n]<48||sorceCode[n]>57)){
                for(int k=m-1;k>=0;k--){
                    data3 = data1[k]-48;
                    data2 = data2 + data3 * pow(10, m-1-k);
                }
                break;
            }
        }
        binaryCode = binaryCode + data2;
    }
    else if(sorceCode[i]=='a'&&sorceCode[i+3]==' '){//add
        binaryCode = 11;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+5, sorceCode, 3) + getName(i+10, sorceCode, 1) + getName(i+15, sorceCode, 2);
    }
    else if(sorceCode[i]=='f'&&sorceCode[i+1]=='a'){//fadd
        binaryCode = 12;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+6, sorceCode, 3) + getName(i+11, sorceCode, 1) + getName(i+16, sorceCode, 2);
    }
    else if(sorceCode[i]=='f'&&sorceCode[i+1]=='s'){//fsub
        binaryCode = 13;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+6, sorceCode, 3) + getName(i+11, sorceCode, 1) + getName(i+16, sorceCode, 2);
    }
    else if(sorceCode[i]=='f'&&sorceCode[i+1]=='m'){//fmul
        binaryCode = 14;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+6, sorceCode, 3) + getName(i+11, sorceCode, 1) + getName(i+16, sorceCode, 2);
    }
    else if(sorceCode[i]=='s'&&sorceCode[i+1]=='.'){//s.d
        binaryCode = 15;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+5, sorceCode, 1) + getName(i+11, sorceCode, 2);
        
    }
    else if(sorceCode[i]=='l'&&sorceCode[i+1]=='.'){//l.d
        binaryCode = 16;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+5, sorceCode, 1) + getName(i+11, sorceCode, 2);
    }
    
    else if(sorceCode[i]=='n'){
        binaryCode = 0;
    }
    else if(sorceCode[i]=='b'&&sorceCode[i+1]== ' '){//b
        binaryCode = 6;
        binaryCode = binaryCode << 26;
        for(int k=0;k<sourceLength;k++){
            if(sorceCode[k]== '.'&&sorceCode[k+1]=='t'&&sorceCode[k+2]=='e')
            {trigger = 1; continue; }//turn on the switch when we are searching code segment.
            if(trigger==1&&findCode(sorceCode, k)==1){
                counter++;//count the number of instructions in order to get relative offset
                }
            else if(trigger==1&&sorceCode[i+2]==sorceCode[k]&&sorceCode[i+3]==sorceCode[k+1]&&
                    sorceCode[i+4]==sorceCode[k+2]&&sorceCode[k-2]!='b'){//once we find destination, jump out
                label = counter;
                unlabel=label;//translate offset into unsign type in order to put it into instruction
                break;
            }
            
        }
        binaryCode = binaryCode + unlabel;
    }
    else if(sorceCode[i]=='b'&&sorceCode[i+1]=='e'){//beqz
        binaryCode = 7;
        binaryCode = binaryCode << 26;
        rs = getName(i+6, sorceCode, 1);
        for(int k=0;k<sourceLength;k++){
            if(sorceCode[k]== '.'&&sorceCode[k+1]=='t'&&sorceCode[k+2]=='e')
            {trigger = 1; continue; }
            if(trigger == 1&&findCode(sorceCode, k)==1){
                counter++;
            }
            else if(trigger==1&&sorceCode[i+10]==sorceCode[k]&&sorceCode[i+11]==sorceCode[k+1]&&
                    sorceCode[i+12]==sorceCode[k+2]&&sorceCode[k-2]!=','){
                label = counter;
                unlabel = label;
                break;
            }
        }
        binaryCode = binaryCode + rs + unlabel;
    }
    else if(sorceCode[i]=='b'&&sorceCode[i+1]=='g'){//bge
        binaryCode = 8;
        binaryCode = binaryCode << 26;
        rs=getName(i+5, sorceCode, 1);
        rt=getName(i+10, sorceCode, 2);
        for(int k=0;k<sourceLength;k++){
            if(sorceCode[k]== '.'&&sorceCode[k+1]=='t'&&sorceCode[k+2]=='e')
            {trigger = 1; continue; }
            if(trigger == 1&&findCode(sorceCode, k)==1){
                counter++;
            }
            else if(trigger == 1&&sorceCode[i+14]==sorceCode[k]&&sorceCode[i+15]==sorceCode[k+1]&&
                    sorceCode[i+16]==sorceCode[k+2]&&sorceCode[k-2]!=','){
                label = counter;
                unlabel = label;
                break;
            }
            
        }
        binaryCode = binaryCode + rs + rt +unlabel;
    }
    else if(sorceCode[i]=='b'&&sorceCode[i+1]=='n'){//bne
        binaryCode = 9;
        binaryCode = binaryCode << 26;
        rs=getName(i+5, sorceCode, 1);
        rt=getName(i+10, sorceCode, 2);
        for(int k=0;k<sourceLength;k++){
            if(sorceCode[k]== '.'&&sorceCode[k+1]=='t'&&sorceCode[k+2]=='e')
            {trigger = 1; continue; }
            if(trigger==1&&findCode(sorceCode, k)==1){
                counter++;
            }
            else if(trigger==1&&sorceCode[i+14]==sorceCode[k]&&sorceCode[i+15]==sorceCode[k+1]&&
                    sorceCode[i+16]==sorceCode[k+2]&&sorceCode[k-2]!=','){
                label = counter;
                unlabel = label;
                break;
            }
            
        }
        binaryCode = rs + rt + unlabel+ binaryCode;
    }
    else if (sorceCode[i]=='l'&&sorceCode[i+1]=='a'){//la
        binaryCode = 3;
        binaryCode = binaryCode << 26;
        address = (sorceCode[i+8]-65)*50;
        rs = getName(i+4, sorceCode, 1);
        binaryCode = binaryCode + address+ rs;
        
    }
    else if(sorceCode[i]=='l'&&sorceCode[i+1]=='b'){//lb
        binaryCode = 4;
        binaryCode = binaryCode << 26;
        
        rs = getName(i+4, sorceCode, 1);
        
        if(sorceCode[i+8]!='('){
          address = sorceCode[i+8]-65;
          binaryCode = rs + binaryCode + address*50;
        }
        else{
            rt = getName(i+10, sorceCode, 2);
            binaryCode = binaryCode + rs + rt;
        }
    }
    else if(sorceCode[i]=='l'&&sorceCode[i+1]=='i'){//li
        binaryCode = 5;
        binaryCode = binaryCode << 26;
        
        for(int n=i+8;n<sourceLength;n++){//search the digits, the way is the same with what we use in addi instruction
            if((sorceCode[n]>=48 && sorceCode[n]<=57)){
                data1[m]=sorceCode[n];
                m++;
            }
            if(sorceCode[n-1]>=48&&sorceCode[n-1]<=57&&(sorceCode[n]<48||sorceCode[n]>57)){
                for(int k=m-1;k>=0;k--){
                        data3 = data1[k]-48;
                        data2 = data2 + data3 * pow(10, m-1-k);
                    }
                break;
            }
        }
        rs = getName(i+4, sorceCode, 1);
        binaryCode = binaryCode + data2 + rs;
        
    }
    else if(sorceCode[i]=='s'&&sorceCode[i+1]=='u'){//subi
        binaryCode = 2;
        binaryCode = binaryCode << 26;
        binaryCode = binaryCode + getName(i+6, sorceCode, 1) + getName(i+11, sorceCode, 2);
        for(int n=i+14;n<sourceLength;n++){//search the digits, the way is the same with what we use in addi instruction
            if((sorceCode[n]>=48 && sorceCode[n]<=57)){
                data1[m]=sorceCode[n];
                m++;
            }
            if(sorceCode[n-1]>=48&&sorceCode[n-1]<=57&&(sorceCode[n]<48||sorceCode[n]>57)){
                for(int k=m-1;k>=0;k--){
                    data3 = data1[k]-48;
                    data2 = data2 + data3 * pow(10, m-1-k);
                }
                break;
            }
        }
        binaryCode = binaryCode + data2;
    }
    else if(sorceCode[i]=='s'&&sorceCode[i+1]=='y'){//system call
        binaryCode = 10;
        binaryCode = binaryCode << 26;
    }
    return binaryCode;
}

//Fetch code from sorce code and put them into code segment
void getCode(char *sorceCode, mem_word *code_seg){
    int trigger=0;
    int j=0;
    
    for(int i=0;i<sourceLength;i++){
        if(sorceCode[i]=='.'&&sorceCode[i+1]=='t'&&sorceCode[i+2]=='e'&&sorceCode[i+3]=='x'&&sorceCode[i+4]=='t'){
            trigger=1;//skip code segment
            continue;
        }

        if(trigger == 1&&findCode(sorceCode, i)==1){
           code_seg[j]=assembler(sorceCode, i, j);//once we find the code we translate it into binary code.
           j++;//put the binary code into main memory
        }
    }
}

void buffer_action(mem_word *buffer){
    for(int i=0;i<19;i++){
        buffer[i]=buffer[i+1];
    }
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
                
                scb->re_status[(buffer[0]>>11)%32]= adder;//set destination's register status in scoreboard
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
                
                scb->re_status[(buffer[0]>>11)%32]= adder;
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
                
                scb->re_status[(buffer[0]>>11)%32]= mult;
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
                
                scb->re_status[(buffer[0]>>21)%32] = mem;
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
                
                scb->re_status[(buffer[0]>>21)%32] = integer;
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
                
                scb->re_status[(buffer[0]>>21)%32] = integer;
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
                
                scb->re_status[(buffer[0]>>11)%32]= integer;
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
                
                scb->re_status[(buffer[0]>>21)%32] = mem;
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
                
                scb->re_status[(buffer[0]>>21)%32] = mem;
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
                
                scb->re_status[(buffer[0]>>21)%32] = mem;
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

void flush(mem_word *input){//flush buffer in 1st stage if branch happens
    for(int i = 0;i<20; i++){
        input[i] = 0;
    }
}

void reset_scb(struct scoreboard *scb, int FU_type){//when instruction release its FU, totally reset its scoreboard
    switch(FU_type){
        case adder:
            scb->adder.Busy=no;
            scb->adder.Fi = -1;
            scb->adder.Fj = -1;
            scb->adder.Fk = -1;
            scb->adder.Qj = -1;
            scb->adder.Rk = -1;
            break;
        case integer:
            scb->integer.Busy = no;
            scb->integer.Fi = -1;
            scb->integer.Fj = -1;
            scb->integer.Fk = -1;
            scb->integer.Qj = -1;
            scb->integer.Qk = -1;
            break;
        case mult:
            scb->mult.Busy = no;
            scb->mult.Fi = -1;
            scb->mult.Fj = -1;
            scb->mult.Fk = -1;
            scb->mult.Qj = -1;
            scb->mult.Qk = -1;
            break;
        case mem:
            scb->mem.Busy = no;
            scb->mem.Fi = -1;
            scb->mem.Fj = -1;
            scb->mem.Fk = -1;
            scb->mem.Qj = -1;
            scb->mem.Qk = -1;
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
            if(simMode == fast){reset_scb(scb, adder);}//release function unit
            break;
        case 13:
            FU_adder[0].ALUout = ro_ex.adderBuffer.A - ro_ex.adderBuffer.B;
            ro_ex.adderBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, adder);}//release function unit
            break;
        
    }
    
    result.integerBuffer = FU_integer[0];
    FU_integer[0] = ro_ex.integerBuffer;
    switch(ro_ex.integerBuffer.opcode){
        case 1:
            FU_integer[0].ALUout = ro_ex.integerBuffer.A + ro_ex.integerBuffer.B;
            ro_ex.integerBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, integer);}
            break;
        case 2:
            FU_integer[0].ALUout = ro_ex.integerBuffer.A - ro_ex.integerBuffer.B;
            ro_ex.integerBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, integer);}
            break;
        case 11:
            FU_integer[0].ALUout = ro_ex.integerBuffer.A + ro_ex.integerBuffer.B;
            ro_ex.integerBuffer.opcode = 0;//departure of instruction
            if(simMode == fast){reset_scb(scb, integer);}
            break;
        case 6:
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, integer);}
            break;
        case 7:
            if(ro_ex.integerBuffer.A != 0){FU_integer[0].ALUout = 0;}
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, integer);}
            break;
        case 8:
            if(ro_ex.integerBuffer.A < ro_ex.integerBuffer.B){FU_integer[0].ALUout = 0;}
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, integer);}
            break;
        case 9:
            if(ro_ex.integerBuffer.A == ro_ex.integerBuffer.B){FU_integer[0].ALUout = 0;}
            ro_ex.integerBuffer.opcode = 0;
            if(simMode == fast){reset_scb(scb, integer);}
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
        if(simMode == fast){reset_scb(scb, mult);}
    }
    
    
    if(ro_ex.memBuffer.opcode == 3||ro_ex.memBuffer.opcode == 4||ro_ex.memBuffer.opcode==5||
       ro_ex.memBuffer.opcode==15||ro_ex.memBuffer.opcode==16){
        load_store = ro_ex.memBuffer;// deal with all memory access instructions
    }
    switch(load_store.opcode){
        case 3:
            load_store.counter--;//counter have been set 2 at 2nd stage in the last clock cycle
            if(load_store.counter == 0)//execution will delay 2 clock cycles
            {result.memBuffer = load_store; reset_scb(scb, mem); load_store.opcode = 0;}
            break;
        case 4:
            load_store.counter--;
            if(load_store.counter == 0)
            {   result.memBuffer = load_store;//once instrucion finished, transfer data to the next latch
                result.memBuffer.ALUout = data_seg[(int)load_store.A];//load
                reset_scb(scb, mem);//release FU
                load_store.opcode = 0;
            }
            break;
        case 5:
            load_store.counter--;
            if(load_store.counter == 0)
            {result.memBuffer = load_store; reset_scb(scb, mem); load_store.opcode = 0;}
            break;
        case 15:
            load_store.counter--;
            if(load_store.counter == 0){
                data_seg[(int)load_store.B] = load_store.A;
                reset_scb(scb, mem); load_store.opcode = 0;
                nopTest=no;//store will be finished here so indicate at least there is one instruction finished in this cycle
            }
            break;
        case 16:
            load_store.counter--;
            if(load_store.counter == 0)
            {   result.memBuffer = load_store;
                result.memBuffer.ALUout = data_seg[(int)load_store.A];
                reset_scb(scb, mem); load_store.opcode = 0;
            }
            break;
    }
    
    
    return result;
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

void WB(struct latch ex_wb, reg_word *reg, double *freg, struct scoreboard *scb, int *userMode, int *PC){

    if(ex_wb.adderBuffer.opcode!=0){//when an adder instruction come it will enter its buffer
        if(simMode == slow){reset_scb(scb, adder);}
        fua[BC_a] = ex_wb.adderBuffer;//move date from latch to buffer
        ex_wb.adderBuffer.opcode = 0;//departure of instruction
        BC_a++;
    }
    switch (fua[0].opcode) {
        case 12:
        case 13:
            if(noWAR(scb, fua[0].rd)==yes){//make sure no waw
                scb_check(scb, adder);//update scoreboard to inform RAW hazard
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
        if(simMode == slow){reset_scb(scb, integer);}
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
        if(simMode == slow){reset_scb(scb, integer);}
        fui[BC_i] = ex_wb.integerBuffer;
        ex_wb.integerBuffer.opcode = 0;
        BC_i++;
    }
    switch(fui[0].opcode){
        case 1:
        case 2:
            if(noWAR(scb, fui[0].rs)==yes){
                scb_check(scb, integer);
                reg[fui[0].rs] = fui[0].ALUout;
                scb->re_status[fui[0].rs] = 0;
                
                datamove(fui);
                BC_i--;
                nopTest=no;
            }
            break;
        case 11:
            if(noWAR(scb, fui[0].rd)){
                scb_check(scb, integer);
                reg[fui[0].rd] = fui[0].ALUout;
                scb->re_status[fui[0].rd] = 0;
                
                datamove(fui);
                BC_i--;
                nopTest=no;
            }
            break;
    }
    
    if(ex_wb.multBuffer.opcode!=0){
        if(simMode == slow){reset_scb(scb, mult);}
        fum[BC_t] = ex_wb.multBuffer;
        ex_wb.multBuffer.opcode = 0;
        BC_t++;
    }
    switch(fum[0].opcode){
        case 14:
            if(noWAR(scb, fum[0].rd)==yes){
                scb_check(scb, mult);
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
                scb_check(scb, mem);
                freg[fumem[0].rs] = fumem[0].ALUout;
                scb->re_status[fumem[0].rs] = 0;
                
                datamove(fumem);
                BC_m--;
                nopTest=no;
            }
            break;
        case 5:
            if(noWAR(scb, fumem[0].rs)==yes){
                scb_check(scb, mem);
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
int main(int argc, const char * argv[])
{
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

