#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

#define BLOCK_SIZE 512

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;

    if (inode_iget(fs, inumber, &in) == -1) {
        return -1;
    }

    int filesize = inode_getsize(&in);
    
    int offsetfile = blockNum * BLOCK_SIZE;
    if (offsetfile >= filesize) {
        return 0;
    }

    int bytes = BLOCK_SIZE;
    if (offsetfile + bytes > filesize) {
        bytes = filesize - offsetfile;
    }

    int blockNumInDisk = inode_indexlookup(fs, &in, blockNum);
    if (blockNumInDisk == -1) {
        return -1;
    }
    if (diskimg_readsector(fs->dfd, blockNumInDisk, buf) == -1) {
        return -1;
    }
    return bytes;
}
