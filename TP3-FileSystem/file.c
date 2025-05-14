#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;

    if (blockNum < 0) {
        return -1; // Número de bloque inválido
    }

    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }

    uint32_t filesize = ((uint32_t)in.i_size0 << 16) | in.i_size1;
    uint32_t offsetfile = (uint32_t)blockNum * DISKIMG_SECTOR_SIZE;

    int blockNumInDisk = inode_indexlookup(fs, &in, blockNum); // si el bloque esta fuera del archivo
    if (blockNumInDisk < 0) {
        return -1;
    }

    if (offsetfile >= filesize) {
        return 0;
    }

    int bytes = filesize - offsetfile;
    if (bytes > DISKIMG_SECTOR_SIZE) {
        bytes = DISKIMG_SECTOR_SIZE;
    }

    if (diskimg_readsector(fs->dfd, blockNumInDisk, buf) != DISKIMG_SECTOR_SIZE) {
        return -1;
    }
    return bytes;
}
