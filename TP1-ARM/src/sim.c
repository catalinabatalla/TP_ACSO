#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "shell.h"

// Estructura

typedef struct {
    uint32_t opcode; 
    uint32_t Rd;
    uint32_t Rn;
    uint32_t Rm;
    uint32_t Ra;
    uint32_t Rt;
    uint32_t shift;
    //quizas en vez de tener ochenta mil inmediatos, se puede tener un solo inmediato y un tipo de inmediato y lo vas shifteando o achicando
    uint32_t imm12, imm3, imm16, imm6, imm26, imm19, immr, immrs, imm9;
    uint32_t cond;
    uint32_t opt;
} Instruction;

//Function prototypes
Instruction decode_instruction(uint32_t instruction);
void update_flags(uint32_t result);
void process_instruction();
void execute_instruction(Instruction ins);


// Define las máscaras de los opcodes
#define ADDS_extended 0b11010110001 //opcode del manual ACTUA COMO UNA MASCARANA 
#define ADDS_inmediate 0b10010001
#define SUBS_extended_register 0b11101011000 //cambio el 21 por 0
#define SUBS_inmediate 0b11110001
#define HLT 0b11010100010
#define CMP_extended 0b11101011001 //alias to the SUBS, the encodings match the encoding of SUBS extended, the descroption gives the operational pseudocode for the intruction
#define ANDS_shifted_register 0b11101010
#define EOR_shifted_register 0b11001010
#define ORR_shifted_register 0b10101010
#define B 0b000101
#define B_cond 0b01010100
#define LDRUB 0b00111000010
#define MOVZ 0b110100101
#define ADD_extended 0b10001011001
#define LSL_inmmediate 0b1101001101 //CREO CHEQUEAR pag 757
#define LSR_inmediate 0b1101001101 //CREO CHEQUEAR pag 757
#define STUR 0b11111000000
#define STURB 0b00111000000
#define STURH 0b01111000000
#define LDUR 0b11111000010
#define MUL 0b10011011000
#define CBZ 0b10110100
#define CBNZ 0b10110101




uint32_t get_opcode(uint32_t instruction){
    for (int i = 31; i >= 0; i--){
        if ((instruction >> i) & 1){
            return instruction >> i;     
        }
        else if ((instruction >> i) & 0x7FF){
            return instruction >> i;
        }
    }
}

// Función para extraer el opcode de una instrucción
Instruction decode_instruction(uint32_t instruction) {
    Instruction ins;
    uint32_t opcode = (instruction >> 21) & 0x7FF; //desplaza 21 bits a la derecha y hace una mascara para obtener los 11 bits del opcode
    
    uint32_t Rd = (instruction >> 0) & 0x1F; //en cada caso se obtiene los primeros cuatro bits (registro destino) (0-4)
    uint32_t Rn = (instruction >> 5) & 0x1F; //en cada caso se obtiene los siguientes cuatro bits (registro origen) (5-9)
    uint32_t Rm = (instruction >> 16) & 0x1F; //en cada caso se obtiene los ultimos cuatro bits (registro origen) (16-20)
    uint32_t Ra = (instruction >> 10) & 0x1F; //en cada caso se obtiene los 5 bits inferiores (10-14) // ESTE ES EN EL CASO DE MUL
    uint32_t Rt = (instruction >> 0) & 0x1F; //en cada caso se obtiene los 5 bits inferiores (0-4) // ESTE ES EN EL CASO DE CBZ Y CBNZ

    uint32_t shift = (instruction >> 22) & 0x1; //en cada caso se obtiene el bit 22 (shift) (22 - 23)

    uint32_t imm12 = (instruction >> 10) & 0xFFF; //en cada caso se obtiene los 12 bits inferiores (10-21)
    uint32_t imm3 = (instruction >> 10) & 0x7; //en cada caso se obtiene los 3 bits inferiores (10-12)
    uint32_t imm16 = (instruction >> 5) & 0xFFFF; //en cada caso se obtiene los 16 bits inferiores (5-20)
    uint32_t imm6 = (instruction >> 10) & 0x3F; //en cada caso se obtiene los 6 bits inferiores (10-15) //IGUAL A IMMS
    uint32_t imm26 = (instruction >> 0) & 0x3FFFFFF; //en cada caso se obtiene los 26 bits inferiores (0-25)
    uint32_t imm19 = (instruction >> 5) & 0x7FFFF; //en cada caso se obtiene los 19 bits inferiores (5-23)
    uint32_t immr = (instruction >> 16) & 0x3F; //en cada caso se obtiene los 6 bits inferiores (16-21) es con R porque es lo de la derecha e ozq
    //el immrs es igual a imm6 que es el que va con immr
    uint32_t imm9 = (instruction >> 12) & 0x1FF; //en cada caso se obtiene los 9 bits inferiores (12-20)

    uint32_t cond = (instruction >> 0) & 0xF; //en cada caso se obtiene los 4 bits inferiores (0-3)
    uint32_t opt = (instruction >> 13) & 0x2;   //en cada caso se obtiene los 4 bits inferiores (13 -15)

    if (opcode == ADDS_inmediate || opcode == SUBS_inmediate){
        ins.imm12 = imm12;
        if (ins.shift == 1){
            ins.imm12 = imm12 <<= 12;
        }
    }
    return ins;
}


// Función que actualiza los flags
void update_flags(uint32_t result) {
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result < 0);
}


void execute_instruction(Instruction ins){
    switch(ins.opcode){
        case ADDS_extended:
            NEXT_STATE.REGS[ins.Rd] = CURRENT_STATE.REGS[ins.Rn] + CURRENT_STATE.REGS[ins.Rm]; // realiza la suma de los registros Rn y Rm y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[ins.Rd]); // actualiza las flags con el resultado
            break;
        case ADDS_inmediate:
            NEXT_STATE.REGS[ins.Rd] = CURRENT_STATE.REGS[ins.Rn] + ins.imm12; // realiza la suma de los registros Rn y el inmediato y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[ins.Rd]); // actualiza las flags con el resultado
            break;
        case SUBS_extended_register:
            NEXT_STATE.REGS[ins.Rd] = CURRENT_STATE.REGS[ins.Rn] - CURRENT_STATE.REGS[ins.Rm]; // realiza la resta de los registros Rn y Rm y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[ins.Rd]); // actualiza las flags con el resultado
            break;
        case SUBS_inmediate:
            NEXT_STATE.REGS[ins.Rd] = CURRENT_STATE.REGS[ins.Rn] - ins.imm12; // realiza la resta de los registros Rn y el inmediato y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[ins.Rd]); // actualiza las flags con el resultado
            break;
        case HLT:
            printf("Se detiene la ejecución\n");
            RUN_BIT = 0; // apaga el bit de ejecución
            break;

        case CMP_extended: //EL CMP ES COMO ALIADO DE SUBS, QUIZAS PODEMOS METERLO TODO EN UNO
            NEXT_STATE.REGS[ins.Rd] = CURRENT_STATE.REGS[ins.Rn] - CURRENT_STATE.REGS[ins.Rm]; // realiza la resta de los registros Rn y Rm y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[ins.Rd]); // actualiza las flags con el resultado
            break;
    }
}

void process_instruction(){
    Instruction ins;
    //1. lee la instrucción de la memoria
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);

    //2. decodifica la instrucción
    Instruction ins_dec = decode_instruction(instruction);

    //3. muestro la instrucción decodificada
    printf("PC: 0x%08lx\n", CURRENT_STATE.PC);
    printf("Instrucción: 0x%08x\n", instruction);
    printf("Opcode: 0x%08x\n", ins_dec.opcode);
    printf("Rd: 0x%08x\n", ins_dec.Rd);
    printf("Rn: 0x%08x\n", ins_dec.Rn);
    printf("Rm: 0x%08x\n", ins_dec.Rm);
    printf("Ra: 0x%08x\n", ins_dec.Ra);
    printf("Imm12: 0x%08x\n", ins_dec.imm12);

    //4. ejecuta la instrucción
    execute_instruction(ins_dec);

    if (RUN_BIT){ //no avanzar si se apretó HLT
        //5. actualiza el estado
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
}
