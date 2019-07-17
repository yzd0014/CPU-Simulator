//
//  machine_code_generator.c
//  cpu_simulator
//
//  Created by 戴一通 on 9/23/16.
//  Copyright © 2016 ___FULLUSERNAME__. All rights reserved.
//
#include "machine_code_generator.h"
#include <stdio.h>
#include <math.h>
#include "data_type.h"
#include "struct_library.h"
#include "global_data.h"

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


