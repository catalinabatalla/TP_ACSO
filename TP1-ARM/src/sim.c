#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "shell.h"

// Define las máscaras de los opcodes
#define ADDS_extended  0b10101011000
#define ADDS_inmediate 0b10110001
#define SUBS_extended_register 0b11101011000
#define SUBS_inmediate 0b11110001
#define HLT 0b11010100010
#define CMP_extended 0b11101011001
#define CMP_inmediate 0b11110001
#define ANDS_shifted_register 0b11101010
#define EOR_shifted_register 0b11001010
#define ORR_shifted_register 0b10101010
#define B 0b000101
#define B_cond 0b01010100
#define BR 0b1101011000011111000000 //raro
#define LSL_inmediate 0b110100110
#define LSR_inmediate 0b110100110
#define STUR 0b11111000000
#define STURB 0b00111000000
#define STURH 0b01111000000
#define LDUR 0b11111000010
#define LDURH 0b01111000010
#define LDURB 0b00111000010
#define MOVZ 0b110100101
#define ADD_extended 0b10001011001
#define ADD_inmediate 0b10010001
#define MUL 0b10011011000
#define CBZ 0b10110100
#define CBNZ 0b10110101


// Define una estructura para almacenar los opcodes y sus longitudes
typedef struct {
    uint32_t opcode;
    int length; // Longitud en bits del opcode
} OpcodeEntry;

void utils(uint32_t instruction, uint32_t *Rd, uint32_t *Rn, uint32_t *Rm, 
    uint32_t *Ra, uint32_t *Rt, uint32_t *shift, uint32_t *immr, 
    uint32_t *cond, uint32_t *opt) {
    *Rd = (instruction >> 0) & 0x1F; //en cada caso se obtiene los primeros cuatro bits (registro destino) (0-4)
    *Rn = (instruction >> 5) & 0x1F; //en cada caso se obtiene los siguientes cuatro bits (registro origen) (5-9)
    *Rm = (instruction >> 16) & 0x1F; //en cada caso se obtiene los ultimos cuatro bits (registro origen) (16-20)
    *Ra = (instruction >> 10) & 0x1F; //en cada caso se obtiene los 5 bits inferiores (10-14) // ESTE ES EN EL CASO DE MUL
    *Rt = (instruction >> 0) & 0x1F; //en cada caso se obtiene los 5 bits inferiores (0-4) // ESTE ES EN EL CASO DE CBZ Y CBNZ
    *shift = (instruction >> 22) & 0x1; //en cada caso se obtiene el bit 22 (shift) (22 - 23)
    *immr = (instruction >> 16) & 0x3F; //en cada caso se obtiene los 6 bits inferiores (16-21) es con R porque es lo de la derecha e ozq
    *cond = (instruction >> 0) & 0xF; //en cada caso se obtiene los 4 bits inferiores (0-3)
    *opt = (instruction >> 13) & 0x2;  //en cada caso se obtiene los 4 bits inferiores (13 -15)
}

void update_flags(uint32_t result) {
    NEXT_STATE.FLAG_Z = (result == 0);
    NEXT_STATE.FLAG_N = (result < 0);
}

// Define un array con los opcodes conocidos
OpcodeEntry opcode_dict[] = {
    {ADDS_extended, 11},
    {ADDS_inmediate, 8},
    {SUBS_extended_register, 11},
    {SUBS_inmediate, 8},
    {HLT, 11},
    {CMP_extended, 11},
    {ANDS_shifted_register, 8},
    {EOR_shifted_register, 8},
    {ORR_shifted_register, 8},
    {B, 6},
    {B_cond, 8},
    {LDURB, 11},
    {MOVZ, 9},
    {ADD_extended, 11},
    {LSL_inmediate, 10},
    {LSR_inmediate, 10},
    {STUR, 11},
    {STURB, 11},
    {STURH, 11},
    {LDUR, 11},
    {MUL, 11},
    {CBZ, 8},
    {CBNZ, 8}
};
const int opcode_dict_size = sizeof(opcode_dict) / sizeof(opcode_dict[0]);



uint32_t get_opcode(uint32_t instruction){
    
    for(int i = 0; i < opcode_dict_size; i++){
        // uint32_t mask = (1 << opcode_dict[i].length) - 1;
        uint32_t extracted_opcode = instruction >> (32 - opcode_dict[i].length);
        if(extracted_opcode == opcode_dict[i].opcode){
            return opcode_dict[i].opcode;
        }
    }
}

const char* identify_instruction(uint32_t instruction) {
    uint32_t opcode = get_opcode(instruction);

    if (opcode == ADDS_extended) {
        return "ADDS_extended";
    } else if (opcode == ADDS_inmediate) {
        return "ADDS_inmediate";
    } else if (opcode == SUBS_extended_register) {
        return "SUBS_extended_register";
    } else if (opcode == SUBS_inmediate) {
        return "SUBS_inmediate";
    } else if (opcode == HLT) {
        return "HLT";
    } else if (opcode == CMP_extended) {
        return "CMP_extended";
    } else if (opcode == ANDS_shifted_register) {
        return "ANDS_shifted_register";
    } else if (opcode == EOR_shifted_register) {
        return "EOR_shifted_register";
    } else if (opcode == ORR_shifted_register) {
        return "ORR_shifted_register";
    } else if (opcode == B) {
        return "B";
    } else if (opcode == B_cond) {
        return "B_cond";
    } else if (opcode == LDURB) {
        return "LDRUB";
    } else if (opcode == MOVZ) {
        return "MOVZ";
    } else if (opcode == ADD_extended) {
        return "ADD_extended";
    } else if (opcode == LSL_inmediate) {
        return "LSL_inmmediate";
    } else if (opcode == LSR_inmediate) {
        return "LSR_inmediate";
    } else if (opcode == STUR) {
        return "STUR";
    } else if (opcode == STURB) {
        return "STURB";
    } else if (opcode == STURH) {
        return "STURH";
    } else if (opcode == LDUR) {
        return "LDUR";
    } else if (opcode == MUL) {
        return "MUL";
    } else if (opcode == CBZ) {
        return "CBZ";
    } else if (opcode == CBNZ) {
        return "CBNZ";
    } else {
        return "Unknown Instruction";
    }
}


void execute_instruction( uint32_t ins){
    uint32_t opcode = get_opcode(ins);
    uint32_t Rd, Rn, Rm, Ra, Rt, shift, immr, cond, opt;
    utils(ins, &Rd, &Rn, &Rm, &Ra, &Rt, &shift, &immr, &cond, &opt);

    uint32_t imm12 = (ins >> 10) & 0xFFF;
    printf("Rd: %x\n", Rd);
    printf("Rn: %x\n", Rn);

    
    if (Rd >= 32 || Rn >= 32 || Rm >= 32) {
        printf("Error: Registro fuera de rango (Rd: %u, Rn: %u, Rm: %u)\n", Rd, Rn, Rm);
        return;
    }
    else{
        printf("Registros dentro de rango\n");
    }

    switch(opcode){
        case ADDS_extended:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] + CURRENT_STATE.REGS[Rm]; // realiza la suma de los registros Rn y Rm y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[Rd]); // actualiza las flags con el resultado
            break;
        case ADDS_inmediate:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] + imm12; // realiza la suma de los registros Rn y el inmediato y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[Rd]); // actualiza las flags con el resultado
            if (shift == 1){
                imm12 = imm12 <<= 12;
            }
            break;
        case SUBS_extended_register:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] - CURRENT_STATE.REGS[Rm]; // realiza la resta de los registros Rn y Rm y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[Rd]); // actualiza las flags con el resultado
            break;
        case SUBS_inmediate:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] - imm12; // realiza la resta de los registros Rn y el inmediato y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[Rd]); // actualiza las flags con el resultado
            if (shift == 1){
                imm12 = imm12 <<= 12;
            }
            break;
        case HLT:
            printf("Se detiene la ejecución\n");
            RUN_BIT = 0; // apaga el bit de ejecución
            break;

        case CMP_extended: //EL CMP ES COMO ALIADO DE SUBS, QUIZAS PODEMOS METERLO TODO EN UNO
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] - CURRENT_STATE.REGS[Rm]; // realiza la resta de los registros Rn y Rm y lo guarda en Rd
            update_flags(NEXT_STATE.REGS[Rd]); // actualiza las flags con el resultado
            break;
    }
}


void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = get_opcode(instruction);
    printf("instruction: %x\n", instruction);
    printf("identificador: %s\n", identify_instruction(instruction));
    identify_instruction(opcode);
    execute_instruction(instruction);
    if (RUN_BIT) {
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }

    
}
