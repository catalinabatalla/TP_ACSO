#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"
#include <string.h>

#define INODE_START_SECTOR 2
#define INDIR_ADDRESS 7
#define POINTERS_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(short))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1; // Número de inodo inválido
    }

    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode); // 512/sizeof(struct inode) = 16
    int maxinode = fs->superblock.s_isize * inodes_per_block; 
    if (inumber > maxinode || inumber < 1) {
        return -1; // Número de inodo fuera de rango
    }

    int inode_index = inumber - 1;

    int sector_num = INODE_START_SECTOR + (inode_index / inodes_per_block);

    unsigned char buf[DISKIMG_SECTOR_SIZE];

    int err = diskimg_readsector(fs->dfd, sector_num, buf);
    if (err != DISKIMG_SECTOR_SIZE) {
        return -1; // Error al leer el sector
    }

    int offset = inode_index % inodes_per_block;

    memcpy(inp, buf + (offset) * sizeof(struct inode), sizeof(struct inode));
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {

    int amount_pointers_per_block = DISKIMG_SECTOR_SIZE / sizeof(short);

    if (blockNum < 0) {
        return -1; // Número de bloque inválido
    }
    if (!(inp->i_mode & IALLOC)) {
        return -1; // Inodo no asignado
    }


    //Caso 1: Archivo no grande (donde tiene direcciones directas en i_addr)
    if (!(inp ->i_mode & ILARG)) {
        if (blockNum >= 8) return -1;
        if (inp->i_addr[blockNum] == 0) return -1; // Bloque no asignado
        return inp->i_addr[blockNum];
    }
    else {
        // Caso 2: Archivo grande (donde tiene direcciones indirectas en i_addr)
        int limit = INDIR_ADDRESS * amount_pointers_per_block;
        if (blockNum < limit) {
            // Simple
            int blockIndex = blockNum / amount_pointers_per_block;
            int offset = blockNum % amount_pointers_per_block;

            if (blockIndex >= INDIR_ADDRESS) {
                return -1; // Error: índice de bloque fuera de rango
            }

            int indBlockNum = inp->i_addr[blockIndex];
            if (indBlockNum == 0) {
                return -1; // Bloque no asignado
            }
            unsigned short buf[amount_pointers_per_block];
            int err = diskimg_readsector(fs->dfd, indBlockNum, buf);
            if (err != DISKIMG_SECTOR_SIZE) {
                return -1; // Error al leer el sector
            }
            int blockPointer = buf[offset];
            if (blockPointer == 0) {
                return -1; // Bloque no asignado
            }
            return blockPointer; // Retorna el bloque apuntado
        }
        else{
            // Doble indirecto
            int dobleBlockNum = blockNum - limit;
            int indBlockNum = inp->i_addr[INDIR_ADDRESS];
            if (indBlockNum == 0) {
                return -1; // Bloque no asignado
            }
            unsigned short buf[amount_pointers_per_block];
            int err = diskimg_readsector(fs->dfd, indBlockNum, buf);
            if (err != DISKIMG_SECTOR_SIZE) {
                return -1; // Error al leer el sector
            }
            int firstBlockIndex = dobleBlockNum / amount_pointers_per_block;
            int firstBlockOffset = dobleBlockNum % amount_pointers_per_block;

            if (firstBlockIndex >= amount_pointers_per_block) {
                return -1; // Error: índice de bloque fuera de rango
            }
            int secondBlockNum = buf[firstBlockIndex];
            if (secondBlockNum == 0) {
                return -1; // Bloque no asignado
            }
            unsigned short buf2[amount_pointers_per_block];
            err = diskimg_readsector(fs->dfd, secondBlockNum, buf2);
            if (err != DISKIMG_SECTOR_SIZE) {
                return -1; // Error al leer el sector
            }
            int blockPointer = buf2[firstBlockOffset];
            if (blockPointer == 0) {
                return -1; // Bloque no asignado
            }
            return blockPointer; // Retorna el bloque apuntado
        }

    }
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1); 
}
