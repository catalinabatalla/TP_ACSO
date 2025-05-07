#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

#define INODE_START_SECTOR 2
#define INDIR_ADDRESS 7
#define POINTERS_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(short))

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) {
        return -1; // Número de inodo inválido
    }

    int inodes_per_sector = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    
    int inode_index = inumber - 1;

    int sector_num = INODE_START_SECTOR + (inode_index / inodes_per_sector);
    int offset = inode_index % inodes_per_sector;

    char ino[DISKIMG_SECTOR_SIZE];

    int err = diskimg_readsector(fs->dfd, sector_num, ino);
    if (err == 1) {
        return -1; // Error al leer el sector
    }

    struct inode *inodes = (struct inode *)ino;
    *inp = inodes[offset];
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {

    //Caso 1: Archivo no grande (donde tiene direcciones directas en i_addr)
    if ((inp ->i_mode & ILARG) == 0) {
        return inp->i_addr[blockNum];
    }

    //Caso 2: Archivo grande (donde tiene direcciones indirectas en i_addr)
    //Caso 2a: Busco entre los bloques indirectos
    if (blockNum < 7 * POINTERS_PER_BLOCK) {
        int blockIndex = blockNum / POINTERS_PER_BLOCK;
        int offset = blockNum % POINTERS_PER_BLOCK;
        char buf[DISKIMG_SECTOR_SIZE];
        int err = diskimg_readsector(fs->dfd, inp->i_addr[blockIndex], buf);
        if (err == -1) {
            return -1; // Error al leer el sector
        }
        short *blockPointers = (short *)buf;   
        return blockPointers[offset];
    }

    //Caso 2b: Busco entre los bloques doblemente indirectos
    else{
        int dobleBlockNum = blockNum - INDIR_ADDRESS * POINTERS_PER_BLOCK;
        int fila = dobleBlockNum / POINTERS_PER_BLOCK;
        int offset = dobleBlockNum % POINTERS_PER_BLOCK;
        char buf[DISKIMG_SECTOR_SIZE];

        int err = diskimg_readsector(fs->dfd, inp->i_addr[INDIR_ADDRESS], buf);
        if (err == -1) {
            return -1; // Error al leer el sector
        }
        short *blockPointers = (short *)buf;
        int blockIndex = blockPointers[fila];
        int err2 = diskimg_readsector(fs->dfd, blockIndex, buf);
        if (err2 == -1) {
            return -1; // Error al leer el sector
        }
        short *blockPointers2 = (short *)buf;
        return blockPointers2[offset];

    }
}

int inode_getsize(struct inode *inp) {
    return ((inp->i_size0 << 16) | inp->i_size1); 
}
