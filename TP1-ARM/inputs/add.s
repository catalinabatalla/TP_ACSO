.section .text
.global _start

_start:
    // Usar MOVZ para cargar inmediatos en registros
    MOVZ X1, #10          // X1 = 10
    MOVZ X2, #20          // X2 = 20
    ADD X3, X1, X2        // X3 = X1 + X2 → X3 = 30
    HLT #0                // Se detiene la ejecución
