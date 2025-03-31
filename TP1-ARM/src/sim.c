#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "shell.h"

// Define las máscaras de los opcodes
#define ADDS_extended  0b10101011000
#define ADDS_immediate 0b10110001
#define SUBS_extended_register 0b11101011000
#define SUBS_immediate 0b11110001
#define HLT 0b11010100010
#define CMP_extended 0b11101011001
#define CMP_immediate 0b11110001
#define ANDS_shifted_register 0b11101010
#define EOR_shifted_register 0b11001010
#define ORR_shifted_register 0b10101010
#define B 0b000101
#define B_cond 0b01010100
#define BR 0b1101011000011111000000
#define LSL_immediate 0b1101001101
#define LSR_immediate 0b1101001101 
#define STUR 0b11111000000
#define STURB 0b00111000000
#define STURH 0b01111000000
#define LDUR 0b11111000010
#define LDURH 0b01111000010
#define LDURB 0b00111000010
#define MOVZ 0b110100101
#define ADD_extended 0b10001011000 
#define ADD_immediate 0b10010001
#define MUL 0b10011011000
#define CBZ 0b10110100
#define CBNZ 0b10110101

#define MEMORY_BASE 0x10000000 // Dirección base de memoria
#define MEMORY_SIZE 0x00100000 // 1 MB

uint8_t MEMORY[MEMORY_SIZE]; // Memoria de 1 MB 

// Define una estructura para almacenar los opcodes y sus longitudes
typedef struct {
    uint32_t opcode;
    int length; 
} OpcodeEntry;

void utils(uint32_t instruction, uint32_t *Rd, uint32_t *Rn, uint32_t *Rm, 
    uint32_t *Ra, uint32_t *Rt, uint32_t *shift, uint32_t *immr, 
    uint32_t *cond, uint32_t *opt, uint32_t *imms) {
    *Rd = (instruction >> 0) & 0x1F; //en cada caso se obtiene los primeros cuatro bits (registro destino) (0-4)
    *Rn = (instruction >> 5) & 0x1F; //en cada caso se obtiene los siguientes cuatro bits (registro origen) (5-9)
    *Rm = (instruction >> 16) & 0x1F; //en cada caso se obtiene los ultimos cuatro bits (registro origen) (16-20)
    *Ra = (instruction >> 10) & 0x1F; //en cada caso se obtiene los 5 bits inferiores (10-14) // ESTE ES EN EL CASO DE MUL
    *Rt = (instruction >> 0) & 0x1F; //en cada caso se obtiene los 5 bits inferiores (0-4) // ESTE ES EN EL CASO DE CBZ Y CBNZ
    *shift = (instruction >> 22) & 0x1; //en cada caso se obtiene el bit 22 (shift) (22 - 23)
    *immr = (instruction >> 16) & 0x3F; //en cada caso se obtiene los 6 bits inferiores (16-21) es con R porque es lo de la derecha e ozq
    *cond = (instruction >> 0) & 0xF; //en cada caso se obtiene los 4 bits inferiores (0-3)
    *opt = (instruction >> 13) & 0x2;  //en cada caso se obtiene los 4 bits inferiores (13 -15)
    *imms = (instruction >> 10) & 0x3F; //en cada caso se obtiene los 6 bits inferiores (10-15) es con R porque es lo de la derecha e ozq
}


void update_flags(int64_t result) {
    NEXT_STATE.FLAG_Z = 0;
    NEXT_STATE.FLAG_N = 0;

    if (result == 0) {
        NEXT_STATE.FLAG_Z = 1; 
        printf("Z flag set\n");
    } else {
        NEXT_STATE.FLAG_Z = 0; 
        printf("Z flag clear\n");
    }

    if (result < 0) {
        NEXT_STATE.FLAG_N = 1; 
        printf("N flag set\n");
    } else {
        NEXT_STATE.FLAG_N = 0; 
        printf("N flag clear\n");
    }
}


// Define un array con los opcodes conocidos
OpcodeEntry opcode_dict[] = {
    {ADDS_extended, 11},
    {ADDS_immediate, 8},
    {SUBS_extended_register, 11},
    {SUBS_immediate, 8},
    {HLT, 11},
    {CMP_extended, 11},
    {CMP_immediate, 8},
    {ANDS_shifted_register, 8},
    {EOR_shifted_register, 8},
    {ORR_shifted_register, 8},
    {B, 6},
    {BR, 22}, 
    {B_cond, 8},
    {LDURB, 11},
    {MOVZ, 9},
    {ADD_extended, 11},
    {ADD_immediate, 8},
    {LSL_immediate, 10},
    {LSR_immediate, 10},
    {STUR, 11},
    {STURB, 11},
    {STURH, 11},
    {LDUR, 11},
    {LDURH, 11},
    {LDURB, 11},
    {MUL, 11},
    {CBZ, 8},
    {CBNZ, 8}
};
const int opcode_dict_size = sizeof(opcode_dict) / sizeof(opcode_dict[0]);

//Funcion para obtener el opcode mediante el uso de un diccionario
uint32_t get_opcode(uint32_t instruction){
    
    for(int i = 0; i < opcode_dict_size; i++){
        uint32_t extracted_opcode = instruction >> (32 - opcode_dict[i].length);
        if(extracted_opcode == opcode_dict[i].opcode){
            return opcode_dict[i].opcode;
        }
    }
}

// Función para determinar el tipo de instrucción

const char* identify_instruction(uint32_t instruction) {
    uint32_t opcode = get_opcode(instruction);

    if (opcode == ADDS_extended) {
        return "ADDS_extended";
    } else if (opcode == ADDS_immediate) {
        return "ADDS_immediate";
    } else if (opcode == SUBS_extended_register) {
        return "SUBS_extended_register";
    } else if (opcode == SUBS_immediate) {
        return "SUBS_immediate";
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
    } else if (opcode == BR) {
        return "BR";
    } else if (opcode == B_cond) {
        return "B_cond";
    } else if (opcode == LDURB) {
        return "LDRUB";
    } else if (opcode == MOVZ) {
        return "MOVZ";
    } else if (opcode == ADD_extended) {
        return "ADD_extended";
    } else if (opcode == LSL_immediate) {
        return "LSL_inmmediate";
    } else if (opcode == LSR_immediate) {
        return "LSR_immediate";
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

int64_t sign_extend(int64_t value, int bits) {
    int64_t mask = 1LL << (bits - 1);
    return (value ^ mask) - mask;
}

void execute_instruction( uint32_t ins){
    uint32_t opcode = get_opcode(ins);
    uint32_t Rd, Rn, Rm, Ra, Rt, shift, immr, cond, opt, imms;
    utils(ins, &Rd, &Rn, &Rm, &Ra, &Rt, &shift, &immr, &cond, &opt, &imms);

    uint32_t imm12 = (ins >> 10) & 0xFFF;  
    printf("Rd: %x\n", Rd);
    printf("Rn: %x\n", Rn);

    
    if (Rd >= 32 || Rn >= 32 || Rm >= 32) {
        printf("Error: Registro fuera de rango (Rd: %u, Rn: %u, Rm: %u) ❌\n", Rd, Rn, Rm);
        return;
    }
    else{
        printf("Registros dentro de rango ✅\n");
    }
 //Itera sobre los opcodes conocidos, cuando hay match, ejecuta la instrucción correspondiente

    switch(opcode){
        case ADDS_extended:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] + CURRENT_STATE.REGS[Rm]; 
            printf("Next state: %lx\n", NEXT_STATE.REGS[Rd]);
            update_flags(NEXT_STATE.REGS[Rd]); 
            break;
        case ADDS_immediate:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] + imm12; 
            printf("Next state: %lx\n", NEXT_STATE.REGS[Rd]);
            update_flags(NEXT_STATE.REGS[Rd]); 
            if (shift == 1){
                imm12 = imm12 <<= 12;
            }
            break;
        case SUBS_extended_register:
            if (Rd == 31) { // si Rd es XZR, es CMP
                printf("CMP extended register\n");
                NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] - CURRENT_STATE.REGS[Rm];
                printf("curr: %lx\n", NEXT_STATE.REGS[Rd]);
                update_flags(NEXT_STATE.REGS[Rd]); 
                break;
            }
        
            else{
                NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] - CURRENT_STATE.REGS[Rm]; 
                update_flags(NEXT_STATE.REGS[Rd]); 
                break;
            }
        case SUBS_immediate:
            if(Rd == 31){ //CMP imm
                printf("CMP immediate\n");
                printf("curr: %lx\n", CURRENT_STATE.REGS[Rn] - imm12);
                update_flags(CURRENT_STATE.REGS[Rn] - imm12); 
                break;
            }
            else{ 
                NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] - imm12; 
                update_flags(NEXT_STATE.REGS[Rd]); 
                if (shift == 1){
                    imm12 = imm12 <<= 12;
                }
                break;
            }
        case HLT:
            printf("Se detiene la ejecución\n");
            RUN_BIT = 0;
            break;
        case ANDS_shifted_register:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] & CURRENT_STATE.REGS[Rm];
            update_flags(NEXT_STATE.REGS[Rd]);
            break;
        case EOR_shifted_register:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] ^ CURRENT_STATE.REGS[Rm];
            break;
        case ORR_shifted_register:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] | CURRENT_STATE.REGS[Rm];
            break;

        case B:
            int32_t imm26 = (ins >> 0) & 0x03FFFFFF;
            if (imm26 & 0x02000000) {
                imm26 |= 0xFC000000;
            }
            int32_t target_address = CURRENT_STATE.PC + (imm26 << 2);
            NEXT_STATE.PC = target_address;
            break; 
        case BR:
            NEXT_STATE.PC = CURRENT_STATE.REGS[Rn];
            break; 
        case B_cond: 
            uint32_t imm19_bcond = (ins >> 5) & 0x7FFFF; 
            int condition = cond; 
        
            int offset = sign_extend(imm19_bcond, 19) << 2; 

        
            switch (condition) {
                case 0b0000: // EQ (Z == 1)
                    if (CURRENT_STATE.FLAG_Z == 1){
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        printf("nest stai pc: %lx\n", NEXT_STATE.PC);

                    } else{
                        NEXT_STATE.PC += 4;
                    }
                    break;
                case 0b0001: // NE (Z == 0)
                    if (CURRENT_STATE.FLAG_Z == 0){
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        printf("nest stai pc: %lx\n", NEXT_STATE.PC);
                    }else{
                        NEXT_STATE.PC += 4;
                    }
                    break;
                case 0b1010: // GE (N == 0)
                    if (CURRENT_STATE.FLAG_N == 0 || CURRENT_STATE.FLAG_Z == 1){
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                    }else{
                        NEXT_STATE.PC += 4;
                    }
                    break;
                case 0b1011: // LT (N == 1)
                    if (CURRENT_STATE.FLAG_N == 1 && CURRENT_STATE.FLAG_Z == 0){
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        printf("nest stai pc: %lx\n", NEXT_STATE.PC);
                    }else{
                        NEXT_STATE.PC += 4;
                    }
                    break;
                case 0b1100: // GT (Z == 0 && N == 0)
                    if (CURRENT_STATE.FLAG_Z == 0 && CURRENT_STATE.FLAG_N == 0){
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        printf("nest stai pc: %lx\n", NEXT_STATE.PC);
                    }else{
                        NEXT_STATE.PC += 4;
                    }
                    break;
                case 0b1101: // LE (Z == 1 || N == 1)
                    if (CURRENT_STATE.FLAG_Z == 1 || CURRENT_STATE.FLAG_N == 1){
                        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                        printf("nest stai pc: %lx\n", NEXT_STATE.PC);
                    }else{
                        NEXT_STATE.PC += 4;
                    }
                    break;
                default:
                    NEXT_STATE.PC += 4;
                    break;
            }
        case LSL_immediate:
            uint32_t immr = (ins >> 16) & 0x3F;
            uint32_t imms = (ins >> 10) & 0x3F;  
            imm12 = (imm12 <<= 12);
            if (imms != 0b111111 && (imms + 1) == immr) {  // Este es el caso de LSL
                opcode = LSL_immediate;
                imm12 = 64 - immr;
                NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] << imm12;
                
            }
            else{
                opcode =LSR_immediate; // Este es el caso de LSR
                imm12 = (ins >> 16) & 0x3F;
                NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] >> imm12;
            }
            
            break;
            
        case MUL:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] * CURRENT_STATE.REGS[Rm];
            break;
        case ADD_extended:
            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] + CURRENT_STATE.REGS[Rm];
            break;
        case ADD_immediate:
            if (shift == 01) {
                imm12 <<= 12; 
            }

            NEXT_STATE.REGS[Rd] = CURRENT_STATE.REGS[Rn] + imm12;
            break;
        case MOVZ:
            uint32_t imm16 = (ins >> 5) & 0xFFFF;
            uint32_t lsl = (ins >> 21) & 0x3; 
            NEXT_STATE.REGS[Rd] = imm16 << (lsl* 16);
            printf("MOVZ: Moved %u to X%u\n", imm16, Rd);
            break;
        case LDUR:
            printf("current state: %lx\n", CURRENT_STATE.REGS[Rn]);
            printf("imm12: %x\n", imm12);
            printf("rn: %x\n", Rn);

            int16_t imm9_ldur = (ins >> 12) & 0b111111111; 
            uint64_t offset_ldur = sign_extend(imm9_ldur, 9); 
            uint64_t add = CURRENT_STATE.REGS[Rn] + offset_ldur; 

            uint32_t value_1 = mem_read_32(add);
            uint32_t value_2 = mem_read_32(add + 4); 

            uint64_t value_ldur = value_1 | ((uint64_t)value_2 << 32); 
            NEXT_STATE.REGS[Rt] = value_ldur; 
            break;

        case LDURB:

            int16_t imm9_ldurb = (ins >> 12) & 0b111111111; 
            uint64_t offset_ldurb = sign_extend(imm9_ldurb, 9); 
            uint64_t add_ldurb = CURRENT_STATE.REGS[Rn] + offset_ldurb; 

            uint64_t read_ldurb = mem_read_32(add_ldurb); 
            uint8_t value_ldurb = read_ldurb & 0b11111111; 

            NEXT_STATE.REGS[Rt] = value_ldurb;
            break;


        case LDURH:
            int16_t imm9_ldurh = (ins >> 12) & 0b111111111; 
            uint64_t offset_ldurh = sign_extend(imm9_ldurh, 9); 
            uint64_t add_ldurh = CURRENT_STATE.REGS[Rn] + offset_ldurh; 

            uint64_t read_ldurh = mem_read_32(add_ldurh); 
            uint8_t value_ldurh = read_ldurh & 0b1111111111111111;

            NEXT_STATE.REGS[Rt] = value_ldurh; 
            break;

        case CBZ:
            if (CURRENT_STATE.REGS[Rd] == 0) {
                int64_t imm;
                int32_t imm19 = (ins >> 5) & 0b1111111111111111111;  
                imm = ((int32_t)(imm19 << 13)) >>11;

                NEXT_STATE.PC = CURRENT_STATE.PC + imm;
            
            } 
            else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case CBNZ:
            if (CURRENT_STATE.REGS[Rd] != 0) {


                int64_t imm;
                int32_t imm19 = (ins >> 5) & 0b1111111111111111111;    
                imm = ((int32_t)(imm19 << 13)) >>11;
                NEXT_STATE.PC = CURRENT_STATE.PC + imm;
            
             
            }
            else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        case STUR:

            uint16_t imm9_stur = (ins >> 12) & 0b111111111; 
            uint64_t offset___ = sign_extend(imm9_stur, 9); 
            uint64_t addr_stur = CURRENT_STATE.REGS[Rn] + offset___; 
            uint64_t val_stur = CURRENT_STATE.REGS[Rt]; 

            mem_write_32(addr_stur, val_stur & 0b11111111111111111111111111111111); 
            mem_write_32(addr_stur + 4, (val_stur >> 32));
            
            break;

        case STURB:
            uint16_t imm9 = (ins >> 12) & 0b111111111; 
            uint64_t offset_ = sign_extend(imm9, 9); 
            uint64_t addr_sturb = CURRENT_STATE.REGS[Rn] + offset_; 
            uint64_t val_b = CURRENT_STATE.REGS[Rt]; 
            
            uint64_t adress_aligned = addr_sturb & ~0b11; 

            uint64_t byte_position = addr_sturb & 0b11; 
            uint32_t word = mem_read_32(adress_aligned); 

            uint8_t value_sturb = val_b & 0b11111111;
            uint32_t mask = ~(0b11111111 << (byte_position * 8));
            uint32_t palabrita = (word & mask)|(value_sturb << (byte_position*8));
            mem_write_32(adress_aligned, palabrita); 
            break;

        case STURH:
            uint32_t Rd_sturh = (ins >> 0) & 0x1F; 
            uint32_t Rn_sturh = (ins >> 5) & 0x1F; 


            uint16_t imm9_sturh = (ins >> 12) & 0b111111111; 
            uint64_t offset__ = sign_extend(imm9_sturh, 9); 
            uint64_t addr_sturh = CURRENT_STATE.REGS[Rn_sturh] + offset__; 
            uint64_t val_h = CURRENT_STATE.REGS[Rd_sturh] & 0b1111111111111111; 

            uint64_t adress_aligned_sturh = addr_sturh & ~0b11; 

            uint64_t byte_position_sturh = addr_sturh & 0b11; 
            uint32_t word_h = mem_read_32(adress_aligned_sturh) ; 

            if(byte_position_sturh == 3){
                uint64_t mask_sturh = ~(0b11111111 << 24);
                uint8_t value_sturh = (val_h & 0b11111111) << 24 ;
                
                uint32_t palabrita_sturh = (word_h & mask_sturh)|(value_sturh); 
                mem_write_32(adress_aligned_sturh, palabrita_sturh); 

                uint32_t palabrita_sturh_2 = mem_read_32(adress_aligned_sturh + 4); 
                uint32_t mask_sturh_2 = ~(0b11111111);
                uint8_t value_sturh_2 = (val_h >> 8) & 0b11111111 ;

                uint32_t palabrita_sturh_2_mask = (palabrita_sturh_2 & mask_sturh)|(value_sturh_2); 
                mem_write_32(adress_aligned_sturh + 4, palabrita_sturh_2_mask); 

            }
            else{
                uint32_t mask_sturh = ~(0b1111111111111111 << (byte_position_sturh * 8));
                uint32_t value_sturh = (val_h << (byte_position_sturh * 8));

                uint32_t palabrita_sturh = (word_h & mask_sturh)|(value_sturh); 
                mem_write_32(adress_aligned_sturh, palabrita_sturh); 

               
            }

        }     
}


void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = get_opcode(instruction);
    printf("--------- \n");
    printf("instruction: %x\n", instruction);
    printf("---------- \n");
    printf("identificador: %s\n", identify_instruction(instruction));
    identify_instruction(opcode);
    execute_instruction(instruction);
    
    if (opcode == B || opcode == B_cond){
        return;
    } else if ((opcode == BR) ) {
        return;
    } else if (opcode == CBZ || opcode == CBNZ) {
        return;
    }

    
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    NEXT_STATE.REGS[31] = 0;
}
