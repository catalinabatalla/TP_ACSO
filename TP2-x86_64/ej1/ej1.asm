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
extern strlen
extern strcpy
extern strdup


string_proc_list_create_asm:
    PUSH RBP
    MOV RBP, RSP

    ; por el struct se que la lista tiene first y last (2 punteros) que equivale a 2*8 = 16
    MOV RDI, 16
    CALL malloc ; malloc espera el tamaño en RDI (1era variable disponible)

    TEST RAX, RAX ; el malloc se guarda en RAX
    JE .return_null ; (jump if equal) entonces si es = 0 (nulo) -> devuelve null

    MOV QWORD [RAX], 0 ; list -> first = NULL (0)
    MOV QWORD [RAX + 8], 0 ; list -> last = NULL (0) (le agrego 8 (offset) porque me muevo de pos)

    JMP .end ; devuelvo el puntero a la lista (salta al end)

;defino las funciones que usé
.return_null:
    ; se sale directo

.end:
    POP RBP ;hago este pop (alineo)
    RET ;devuelvo !


string_proc_node_create_asm:
    PUSH RBP
    MOV RBP, RSP

    ; RDI = type (1era variable) (1 byte) (pero la puedo usar para otra cosa)
    ; RSI = hash (2nda variable) (8 byte) (es un puntero)

    MOV R8B, DIL ; guardamos el type (uint8) en R8B
    MOV R9, RSI; R9 = hash

    ; por el struct del nodo entonces tenemos que 
    TEST R9, R9 ;si hash esta vacio termino
    JE .return_null 

    MOV RDI, 32 ; tiene 2 punteros (8), un uint8 (8) y un char (1) pero tiene que ser multiplo de 8 (32)
    CALL malloc
    TEST RAX, RAX ;veo si falla el MALLOC 
    JE .return_null ;devuelve null

    ; RAX  = NODO NUEVO
    MOV     QWORD [RAX], 0          ; node->next = NULL
    MOV     QWORD [RAX + 8], 0      ; node->previous = NULL
    MOV     BYTE  [RAX + 16], R8B   ; node->type = type
    MOV     QWORD [RAX + 24], R9   ; node->hash = hash

    JMP .end
.return_null:
    MOV RAX, 0;

.end:
    POP RBP ;hago este pop (alineo)
    RET ;devuelvo !

global string_proc_list_add_node_asm

string_proc_list_add_node_asm:
    ; Verificamos si list == NULL || hash == NULL
    TEST    RDI, RDI
    JE      .return
    TEST    RDX, RDX
    JE      .return

    ; Guardamos list en el stack (lo vamos a necesitar después del CALL)
    PUSH    RDI                ; guardar 'list'

    ; Preparamos argumentos para string_proc_node_create_asm
    MOVZX   EDI, SIL           ; type → edi (zero-extend a 32 bits)
    MOV     RSI, RDX           ; hash → rsi
    CALL    string_proc_node_create_asm

    ; Restauramos list
    POP     RDI

    ; Verificamos si el nodo fue creado correctamente
    TEST    RAX, RAX
    JE      .return

    ; RAX = to_add
    ; RDI = list
    MOV     RCX, [RDI]         ; list->first
    MOV     RDX, [RDI + 8]     ; list->last

    ; Si la lista está vacía
    TEST    RCX, RCX
    JNZ     .not_empty
    TEST    RDX, RDX
    JNZ     .not_empty

    ; list->first = to_add
    MOV     [RDI], RAX

    ; list->last = to_add
    MOV     [RDI + 8], RAX

    JMP     .return

.not_empty:
    ; current = list->last
    MOV     RBX, [RDI + 8]         ; current = list->last

    ; current->next = to_add
    MOV     [RBX], RAX             ; current->next = to_add

    ; to_add->previous = current
    MOV     [RAX + 8], RBX         ; to_add->previous = current

    ; list->last = to_add
    MOV     [RDI + 8], RAX         ; list->last = to_add

.return:
    RET

    
string_proc_list_concat_asm:
    PUSH RBP
    MOV RBP, RSP

    ; RDI = list
    ; SIL = type (uint8)
    ; RDX = hash inicial (char*)

    ; Guardamos registros que vamos a usar
    PUSH RBX
    PUSH R12
    PUSH R13
    PUSH R14
    PUSH R15

    ; Guardamos argumentos en registros temporales
    MOV RBX, RDI          ; RBX = list
    MOV R12B, SIL         ; R12B = type
    MOV R13, RDX          ; R13 = hash inicial

    ; === strdup(hash) ===
    ; Creamos una copia del hash inicial (por si no es NULL)
    MOV RDI, R13
    CALL strdup
    MOV R14, RAX          ; R14 = result

    CMP R14, 0
    JE .end               ; Si strdup falla, salimos con NULL

    ; === current = list->first ===
    MOV R15, [RBX]        ; R15 = current = list->first

.loop:
    CMP R15, 0
    JE .end               ; Si current == NULL, terminamos

    ; current->type está en offset 16
    MOVZX EAX, BYTE [R15 + 16]
    CMP AL, R12B
    JNE .next             ; Si el tipo no coincide, saltamos

    ; current->hash está en offset 24
    MOV RSI, [R15 + 24]
    TEST RSI, RSI
    JE .next              ; Si el hash es NULL, saltamos

    ; Concatenamos result + current->hash
    MOV RDI, R14
    CALL str_concat
    CMP RAX, 0
    JE .concat_error      ; Si str_concat falla, salimos con error

    ; Liberamos el string viejo
    MOV RDI, R14
    MOV R14, RAX          ; R14 ahora apunta al nuevo string concatenado
    CALL free

.next:
    ; Avanzamos al siguiente nodo
    MOV R15, [R15]        ; current = current->next
    JMP .loop

.concat_error:
    ; Si str_concat falla, liberamos memoria y devolvemos NULL
    MOV RDI, R14
    CALL free
    XOR R14, R14          ; R14 = NULL

.end:
    MOV RAX, R14          ; return result

    ; Restauramos registros
    POP R15
    POP R14
    POP R13
    POP R12
    POP RBX
    POP RBP
    RET