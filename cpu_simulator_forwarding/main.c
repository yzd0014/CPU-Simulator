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

typedef uint32_t mem_address;
typedef uint32_t mem_word;
typedef int8_t mem_data;
typedef int32_t reg_word;
const int dataLength = 5000;
const int codeLength = 100;
const int sourceLength = 800;

struct id_ex{
    uint16_t opcode;
    int8_t rs, rt, rd;
    reg_word A, B;
    uint16_t immediate;
} id_ex_new, id_ex_old;
struct ex_mem{
    uint16_t opcode;
    int32_t ALUout;
    int8_t rs, rd;
    uint16_t immediate;
} ex_mem_new, ex_mem_old;
struct mem_wb{
    uint16_t opcode;
    int32_t ALUout;
    int8_t rs, rd;
    uint16_t immediate;
    int8_t MDR;
} mem_wb_new, mem_wb_old;

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
    if(sorceCode[i]=='t'){
        reg_name = sorceCode[i+1]-48+8;//translate register name into number type.
        if(j == 1){
             reg_name = reg_name << 21;//put name into proper postion of instruciton
        }
        else if(j == 3){reg_name = reg_name << 11;}
        else{
            reg_name = reg_name << 16;
        }
    }
    else if(sorceCode[i]=='v'){
        reg_name = sorceCode[i+1]-48+2;
        if(j == 1){
            reg_name = reg_name <<21;
        }
        else if(j == 3){reg_name = reg_name << 11;}
        else{
            reg_name = reg_name <<16;
        }
    }
    else if(sorceCode[i]=='a'){
        reg_name = sorceCode[i+1]-48+4;
        if(j == 1){
            reg_name = reg_name<<21;
        }
        else if(j == 3){reg_name = reg_name << 11;}
        else{
            reg_name = reg_name<<16;
        }
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
       (sorceCode[k]=='a'&&sorceCode[k+1]=='d'&&sorceCode[k+2]=='d'&&sorceCode[k+3]==' ')||
       (sorceCode[k]=='n'&&sorceCode[k+1]=='o'&&sorceCode[k+2]=='p')){
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
                label = counter-j-2;
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
                label = counter-j-2;
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
                label = counter-j-2;
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
                label = counter-j-2;
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
mem_word IF(mem_word *code_seg, int *PC){
    mem_word output;
    output = code_seg[*PC];
    *PC = *PC + 1;
    return output;
}

struct id_ex ID(mem_word if_id, int *PC, reg_word *reg){
    int16_t label;
    mem_word instruction = if_id;
    struct id_ex result;
    result.A=0;
    result.B=0;
    result.immediate=0;
    result.opcode=0;
    result.rd=0;
    result.rs=0;
    result.rt=0;
    switch(instruction>>26){
        case 6:
            label = instruction%0x10000;
            *PC=*PC+label;
            break;
        case 7:
            label = instruction%0x10000;
            if(reg[(instruction>>21)%32]==0){*PC = *PC + label;}
            break;
        case 8:
            label = instruction%0x10000;
            if(reg[(instruction>>21)%32]>= reg[(instruction>>16)%32]){*PC = *PC + label;}
            break;
        case 9:
            label = instruction%0x10000;
            if(reg[(instruction>>21)%32]!= reg[(instruction>>16)%32]){*PC = *PC + label;}
            break;
        case 2:
            result.rs = (instruction>>21)%32;
            result.A = reg[(instruction>>16)%32];
            result.B = (instruction%0x10000);
            result.rt= (instruction>>16)%32;
            result.opcode=2;
            break;
        case 1:
            result.rs = (instruction>>21)%32;
            result.A = reg[(instruction>>16)%32];
            result.B = (instruction%0x10000);
            result.rt = (instruction>>16)%32;
            result.opcode=1;
            break;
        case 11:
            result.rd = (instruction>>11)%32;
            result.A = reg[(instruction>>21)%32];
            result.B = reg[(instruction>>16)%32];
            result.rs = (instruction>>21)%32;
            result.rt = (instruction>>16)%32;
            result.opcode=11;
            break;
        case 3:
            result.rs = (instruction>>21)%32;
            result.immediate=instruction%0x10000;
            result.opcode=3;
            break;
        case 4:
            if((instruction>>16)%32 == 0){result.immediate=instruction%0x10000;result.rs=(instruction>>21)%32;}//address is in instruction itself
            else{result.rs=(instruction>>21)%32; result.A=reg[(instruction>>16)%32]; }//address is stored in register
            result.opcode=4;
            break;
        case 5:
            result.rs=(instruction>>21)%32;
            result.immediate=instruction%0x10000;
            result.opcode=5;
            break;
        case 10:
            result.opcode = 10;
            if(reg[2]==4){result.immediate=reg[4];result.A=4;}//print system call
            else if(reg[2]==8){result.immediate=reg[4]; result.A=8;}//read from keyboard system call
            else if(reg[2]==10){result.A=10;}//end system call
            else if(reg[2]==1){printf("%d", reg[4]);}//print int system call
    }
            
    return result;
}

struct ex_mem EX(struct id_ex input){
    struct ex_mem result;
    result.ALUout=0;
    result.immediate=0;
    result.opcode=0;
    result.rd=0;
    result.rs=0;
    switch(input.opcode){
        case 2:
            result.ALUout=input.A-input.B;
            result.rs=input.rs;
            result.opcode=input.opcode;
            result.rd=0;
            break;
        case 1:
            result.ALUout=input.A+input.B;
            result.rs=input.rs;
            result.rd=0;
            result.opcode=input.opcode;
            break;
        case 11:
            result.ALUout = input.A+input.B;
            result.rd = input.rd;
            result.rs=0;
            result.opcode=input.opcode;
            break;
        case 3:
            result.rs=input.rs;
            result.immediate = input.immediate;
            result.rd=0;
            result.opcode = input.opcode;
            break;
        case 4:
            if(input.A!=0){result.ALUout = input.A;result.rs=input.rs;}
            if(input.immediate!=0){result.ALUout=input.immediate;result.rs=input.rs;}
            result.opcode=input.opcode;
            result.rd=0;
            break;
        case 5:
            result.rs=input.rs;
            result.immediate=input.immediate;
            result.opcode=input.opcode;
            result.rd=0;
            break;
        case 10:
            if(input.A==4){result.ALUout=input.A;result.immediate=input.immediate;}
            else if(input.A==8){result.ALUout=input.A;result.immediate=input.immediate;}
            else if(input.A==10){result.ALUout=input.A;}
            result.rd=0;
            result.rs=0;
            result.opcode=input.opcode;
            
            
    }
    return result;
}

struct mem_wb MEM(struct ex_mem input, mem_data *data_seg){
    struct mem_wb result;
    result.ALUout=0;
    result.immediate=0;
    result.MDR=0;
    result.opcode=0;
    result.rd=0;
    result.rs=0;
    switch(input.opcode){
        case 2:;
        case 1:
            result.ALUout=input.ALUout;
            result.rs=input.rs;
            result.opcode=input.opcode;
            break;
        case 11:
            result.ALUout=input.ALUout;
            result.rd=input.rd;
            result.opcode=input.opcode;
            break;
        case 3:
            result.rs=input.rs;
            result.immediate=input.immediate;
            result.opcode=input.opcode;
            break;
        case 4:
            result.MDR=data_seg[input.ALUout];
            result.rs=input.rs;
            result.opcode=input.opcode;
            break;
        case 5:
            result.rs=input.rs;
            result.immediate=input.immediate;
            result.opcode=input.opcode;
            break;
        case 0:
            result.ALUout=0;
            result.immediate=0;
            result.MDR=0;
            result.opcode=0;
            result.rd=0;
            result.rs=0;
            break;
        case 10:
            if(input.ALUout==4){
                for(int i =input.immediate;i< dataLength; i++){
                    if(data_seg[i]=='\0'){break;}
                    else{printf("%c", data_seg[i]);}//print content from memory
                }
            
            }
            else if(input.ALUout==8){
                char ch[50];
                gets(ch);
                for(int i=0;i<dataLength;i++){
                    if(ch[i]=='\0'){break;}
                    else{ data_seg[input.immediate+i]=ch[i];}//put data into memory
              }
            }
            else if(input.ALUout==10){result.ALUout=input.ALUout;}
            result.opcode=input.opcode;
    }
    return result;
}
void WB(struct mem_wb input, reg_word *reg, int *userMode){
    switch (input.opcode) {
        case 1:;
        case 2:
            reg[input.rs]=input.ALUout;
            break;
        case 11:
            reg[input.rd]=input.ALUout;
            break;
        case 3:
            reg[input.rs]=input.immediate;
            break;
        case 4:
            reg[input.rs]=input.MDR;
            break;
        case 5:
            reg[input.rs]=input.immediate;
            break;
        case 10:
            if(input.ALUout==10){*userMode = 0;}
       
    }
    
}

void detector_1(mem_word if_id, struct id_ex *id_ex, struct ex_mem *ex_em, struct mem_wb *mem_wb, reg_word *reg){
    
    if((if_id>>26)==7){//detect data hazard for No.7 instruction
        if((if_id>>21)%32==id_ex->rs){//search id_ex latch
            reg[(if_id>>21)%32]=id_ex->immediate;//access shortcut
        }
        
        
        else if((if_id>>21)%32==ex_em->rs||(if_id>>21)%32==ex_em->rd){//search ex_em latch
            if(ex_em->opcode==3||ex_em->opcode==5){//discern which instruction is in this latch
                reg[(if_id>>21)%32]=ex_em->immediate;//access shortcut
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                reg[(if_id>>21)%32]=ex_em->ALUout;
            }
        }
        
        else if((if_id>>21)%32==mem_wb->rs||(if_id>>21)%32==mem_wb->rd){//search mem_wb latch
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>21)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>21)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
               reg[(if_id>>21)%32]=mem_wb->MDR;
            }
        }
    }
    else if((if_id>>26)==8||(if_id>>26)==9){
        if((if_id>>21)%32==id_ex->rs){
            reg[(if_id>>21)%32]=id_ex->immediate;//access shortcut
        }
        
        else if((if_id>>21)%32==ex_em->rs||(if_id>>21)%32==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                reg[(if_id>>21)%32]=ex_em->immediate;
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                reg[(if_id>>21)%32]=ex_em->ALUout;
            }
        }
        else if((if_id>>21)%32==mem_wb->rs||(if_id>>21)%32==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>21)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>21)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[(if_id>>21)%32]=mem_wb->MDR;
            }
        }

    
        
        
        if((if_id>>16)%32==id_ex->rs){
            reg[(if_id>>21)%32]=id_ex->immediate;//access shortcut
        }
        
        else if((if_id>>16)%32==ex_em->rs||(if_id>>16)%32==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                reg[(if_id>>16)%32]=ex_em->immediate;
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                reg[(if_id>>16)%32]=ex_em->ALUout;
            }
        }
        else if((if_id>>16)%32==mem_wb->rs||(if_id>>16)%32==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>16)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>16)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[(if_id>>16)%32]=mem_wb->MDR;
            }
        }
        
    }
    else if((if_id>>26)==10){
        if(2==id_ex->rs){
            reg[2]=id_ex->immediate;//access shortcut
        }
        
        else if(2==ex_em->rs||2==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                reg[2]=ex_em->immediate;//access shortcut
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                reg[2]=ex_em->ALUout;
            }
        }
        else if(2==mem_wb->rs||2==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[2]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[2]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[2]=mem_wb->MDR;
            }
        }
        
            
            
            if(4==id_ex->rs){
                reg[4]=id_ex->immediate;//access shortcut
            }
            
            else if(4==ex_em->rs||4==ex_em->rd){
                if(ex_em->opcode==3||ex_em->opcode==5){
                    reg[4]=ex_em->immediate;//access shortcut
                }
                else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                    reg[4]=ex_em->ALUout;
                }
            }
            else if(4==mem_wb->rs||4==mem_wb->rd){
                if(mem_wb->opcode==3||mem_wb->opcode==5){
                    reg[4]=mem_wb->immediate;//access shortcut
                }
                else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                    reg[4]=mem_wb->ALUout;
                }
                else if (mem_wb->opcode==4){
                    reg[4]=mem_wb->MDR;
                }
            
            
        }
        
    }
    
    
    else if((if_id>>26)==1||(if_id>>26)==2){//only detech data hazard in mem_wb latch, rest will be detected in detector2
       if((if_id>>16)%32==mem_wb->rs||(if_id>>16)%32==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>16)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>16)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[(if_id>>16)%32]=mem_wb->MDR;
            }
        }
    }
    else if((if_id>>26)==11){//only detech data hazrd in mem_wb latch, rest will be detected in detector2
       
        if((if_id>>21)%32==mem_wb->rs||(if_id>>21)%32==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>21)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>21)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[(if_id>>21)%32]=mem_wb->MDR;
            }
        }
        
        if((if_id>>16)%32==mem_wb->rs||(if_id>>16)%32==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>16)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>16)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[(if_id>>16)%32]=mem_wb->MDR;
            }
        }
        
    }
    else if((if_id>>26)==4){
        if((if_id>>16)%32==id_ex->rs){
            reg[(if_id>>16)%32]=id_ex->immediate;//access shortcut
        }
        
        
        else if((if_id>>16)%32==ex_em->rs||(if_id>>16)%32==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                reg[(if_id>>16)%32]=ex_em->immediate;//access shortcut
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                reg[(if_id>>16)%32]=ex_em->ALUout;
            }
        }
        
        else if((if_id>>16)%32==mem_wb->rs||(if_id>>16)%32==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                reg[(if_id>>16)%32]=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                reg[(if_id>>16)%32]=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                reg[(if_id>>16)%32]=mem_wb->MDR;
            }
        }
    }
}


void detector_2(struct id_ex *id_ex, struct ex_mem *ex_em, struct mem_wb *mem_wb){
    if(id_ex->opcode==1||id_ex->opcode==2){
        if(id_ex->rt==ex_em->rs||id_ex->rt==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                id_ex->A=ex_em->immediate;//access shortcut
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                id_ex->A=ex_em->ALUout;
            }
        }
        else if(id_ex->rt==mem_wb->rs||id_ex->rt==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                id_ex->A=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                id_ex->A=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                id_ex->A=mem_wb->MDR;
            }
        }
    }
    else if(id_ex->opcode==11){
        if(id_ex->rs==ex_em->rs||id_ex->rs==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                id_ex->A=ex_em->immediate;
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                id_ex->A=ex_em->ALUout;
            }
        }
        else if(id_ex->rs==mem_wb->rs||id_ex->rs==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                id_ex->A=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                id_ex->A=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                id_ex->A=mem_wb->MDR;
            }
        }
        if(id_ex->rt==ex_em->rs||id_ex->rt==ex_em->rd){
            if(ex_em->opcode==3||ex_em->opcode==5){
                id_ex->A=ex_em->immediate;
            }
            else if(ex_em->opcode==1||ex_em->opcode==2||ex_em->opcode==11){
                id_ex->A=ex_em->ALUout;
            }
        }
        else if(id_ex->rt==mem_wb->rs||id_ex->rt==mem_wb->rd){
            if(mem_wb->opcode==3||mem_wb->opcode==5){
                id_ex->A=mem_wb->immediate;//access shortcut
            }
            else if(mem_wb->opcode==1||mem_wb->opcode==2||mem_wb->opcode==11){
                id_ex->A=mem_wb->ALUout;
            }
            else if (mem_wb->opcode==4){
                id_ex->A=mem_wb->MDR;
            }
        }
        
    }
}

int main(int argc, const char * argv[])
{
    
    
    //latch
    mem_word if_id_old=0, if_id_new=0;
    
    
    
    // insert code here...
    char sorceCode[sourceLength];
    mem_data data_seg[dataLength];
    mem_word code_seg[codeLength];
    reg_word reg[32];//32 registers
    
    for(int i = 0; i< dataLength; i++){
        data_seg[i]='\0';
    }//initialize data segment
    for(int i = 0; i< codeLength; i++){
        code_seg[i]=10;
    }//initialize code segment
    for(int i = 0; i<32; i++){
        reg[i]=0;
    }//initialize register
    
    
    loader(sorceCode);
    getData(sorceCode, data_seg);
    getCode(sorceCode, code_seg);
    
    int PC = 0;
    int userMode = 1;
    int IC=0;
    int C=0;
    int NOP=0;
    
    
    while(userMode == 1){
        C++;
        
        if_id_old = if_id_new;
        if_id_new = IF(code_seg, &PC);
        if(if_id_new!=10){IC++;}
        if(if_id_new==0){NOP++;}//IF stage
        
        detector_1(if_id_old, &id_ex_new, &ex_mem_new, &mem_wb_new, reg);
        id_ex_old = id_ex_new;
        id_ex_new = ID(if_id_old, &PC, reg);//ID stage
        
        detector_2(&id_ex_old, &ex_mem_new, &mem_wb_new);
        ex_mem_old = ex_mem_new;
        ex_mem_new = EX(id_ex_old);//EX stage
        
        mem_wb_old = mem_wb_new;
        mem_wb_new = MEM(ex_mem_old, data_seg);//MEM stage
        
        WB(mem_wb_old, reg, &userMode);//WB stage
    }
    printf("\nC = %d\n", C);
    printf("IC = %d\n", IC);
    printf("number of nops: %d\n", NOP);
    
    return 0;
}

