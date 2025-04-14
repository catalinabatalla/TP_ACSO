; /** defines bool y puntero **/
%define NULL 0
%define TRUE 1
%define FALSE 0

section .data

section .text

global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

; FUNCIONES auxiliares que pueden llegar a necesitar:
extern malloc
extern free
extern str_concat


string_proc_list_create_asm:
    PUSH RBP
    MOV RBP, RSP

    ; por el struct se que la lista tiene first y last (2 punteros) que equivale a 2*8 = 16
    MOV RDI, 16
    CALL malloc ; malloc espera el tamaño en RDI (1era variable disponible)

    CMP RAX, 0 ; el malloc se guarda en RAX
    JE .return_null ; (jump if equal) entonces si es = 0 (nulo) -> devuelve null

    MOV QWORD [RAX], 0 ; list -> first = NULL (0)
    MOV QWORD [RAX + 8], 0 ; list -> last = NULL (0) (le agrego 8 (offset) porque me muevo de pos)

    JMP .end ; devuelvo el puntero a la lista (salta al end)

;defino las funciones que usé
.return_null:
    MOV RAX, 0; se sale 

.end:
    POP RBP ;hago este pop (alineo)
    RET ;devuelvo !


string_proc_node_create_asm:
    PUSH RBP
    MOV RBP, RSP

    ; RDI = type (1era variable) (1 byte) (pero la puedo usar para otra cosa)
    ; RSI = hash (2nda variable) (8 byte) (es un puntero)

    ; por el struct del nodo entonces tenemos que 
    CMP RSI, 0 ;si hash esta vacio termino
    JE .return_null 

    MOV RDI, 32 ; tiene 2 punteros (8), un uint8 (8) y un char (1) pero tiene que ser multiplo de 8 (32)
    CALL malloc
    CMP RAX, 0 ;veo si falla el MALLOC 
    JE .return_null ;devuelve null

    MOV QWORD [RAX + 16], 0 ; si nodo -> next = NULL (0)
    MOV QWORD [RAX + 8], 0 ; si nodo -> prev = NULL (0) 
    MOV BYTE [RAX], DIL ; este es el type, dil es la version uint8 (byte porq es 1 byte)
    MOV QWORD [RAX + 24], RSI ;este es el hash

    JMP .end
.return_null:
    MOV RAX, 0;

.end:
    POP RBP ;hago este pop (alineo)
    RET ;devuelvo !

string_proc_list_add_node_asm:
    ; RDI = lista (16 bytes) (lista que tiene 16 x los 2 punteros)
    ; RSI = type (1 byte) (char)
    ; RDX = hash (8 bytes) (puntero)

    PUSH RBP
    MOV RBP, RSP

    MOV R8, RDI ; dsp lo pisamos entonces lo guardo en R8 = list

    CMP RDI, 0 ; if list == NULL
    JE .end ; no devuelve NULL pq esta funcion devuelve tipo void

    CMP RDX, 0 ; if hash == NULL
    JE .end 

    MOV RDI, RSI ; muevo type a la primera variable (lo que me devuelve la funcion que voy a llamar ahora)
    MOV RSI, RDX ; muevo hash a la segunda variable

    CALL string_proc_node_create_asm
    CMP RAX, 0; resultavo RAX si es NULL termina
    JE .end

    MOV RCX, [R8] ; rcx = list -> first
    MOV RDX, [R8 + 8] ; ahora puedo usar rdx porque lo moví a rsi rdx = list -> last
    
    MOV R10, RCX
    OR R10, RDX; este OR compara bit a bit -> si el resultado es 0 -> termina
    CMP R10, 0 
    JNE .append_to_end

    MOV [R8], RAX; list-> first = to_add
    MOV [R8 + 8], RAX; list-> last = to_add

    JMP .end

.append_to_end:
    MOV R9, [R8 + 8]; current = list -> last
    MOV [R9 + 16], RAX;  current -> next = to_add
    MOV [RAX + 8], R9;  to_add -> previous = current
    MOV [R8 + 8], RAX;

.end:
    POP RBP
    RET

string_proc_list_concat_asm:
    PUSH RBP
    MOV RBP, RSP 

    ; RDI = list
    ; RSI = type
    ; RDX = hash

    ;guardamos los callee- save
    PUSH RBX
    PUSH R12
    PUSH R13
    PUSH R14

    MOV R12, RDI; R12 = list
    MOV R13, RSI ; R13 = type
    MOV R14, RDX; R14 = hash

    MOV RDI, 1; result = malloc(1)
    CALL malloc
    MOV RBX, RAX; RBX = result 
    MOV BYTE [RBX], 0 ; result[0] = '\0'

    ; RAX = current
    MOV RAX, [R12 + 8]; current = list -> first

.loop:
    CMP RAX, 0 ; while (current != NULL)
    JE .post_loop
    MOVZX RCX, BYTE [RAX + 16] ; RCX = current -> type
    CMP CL, R13B; cl el byte bajo del registro rcx y r13b es el byte bajo de r13 porque es uint8_t
    JNE .next_node

    MOV RDI, RBX; result 
    MOV RSI, [RAX + 24] ; current = hash
    CALL str_concat ;

    MOV RDI, RBX
    CALL free ;

    MOV RBX, RAX; result = temp

.next_node:
    MOV RAX, [RAX + 8] ; current = current -> next
    JMP .loop
    
.post_loop:
    MOV RDI, R14
    MOV RSI, RBX
    CALL str_concat;

    MOV RDI, RBX
    CALL free;
    
    POP R14
    POP R13
    POP R12
    POP RBX

    POP RBP
    RET








