.section .text
.global _start

_start:
    // Cargar la dirección de "target" en X1.
    // Dependiendo del ensamblador, se puede usar MOVZ con desplazamiento (LSL) para armar la dirección.
    // Por ejemplo, si se asume que la dirección de target es 0x400050, se puede hacer:
    MOVZ X1, #0x4050        // X1 = 0x4050 (nota: en la práctica se debe construir la dirección completa)
    // Si es necesario, se podría usar ADD o MOVK para completar la dirección.
    
    BR X1                   // Salta a la dirección contenida en X1 (branch to register)
    
    HLT #0                  // Si BR falla, se detiene la ejecución

target:
    ADDS X2, X0, #42        // X2 = 0 + 42 → X2 = 42 (0x2A)
    HLT #0                  // Se detiene la ejecución
