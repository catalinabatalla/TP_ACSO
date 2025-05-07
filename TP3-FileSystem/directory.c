#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
      struct inode dir_inode;

      // Obtener el inodo del directorio
      if (inode_iget(fs, dirinumber, &dir_inode) < 0) {
          return -1;
      }

      //Verificar si el inodo es un directorio
      if (!(dir_inode.i_mode & IALLOC) || !(dir_inode.i_mode & IFMT) == IFDIR) {
          return -1; // No es un directorio
      }
  
      int filesize = inode_getsize(&dir_inode);
      int entrysize = sizeof(struct direntv6);
      int totalEntries = filesize / entrysize;
      int totalBytesRead = 0;
      int blockNum = 0;

      char buffer[DISKIMG_SECTOR_SIZE];
  
      while (totalBytesRead < filesize) {
          int bytesRead = file_getblock(fs, dirinumber, blockNum, buffer);
          if (bytesRead < 0) {
              return -1;
          }
  
          int entriesPerBlock = bytesRead / entrysize; 
          struct direntv6 *entries = (struct direntv6 *) buffer;
  
          for (int i = 0; i < entriesPerBlock && totalBytesRead < totalEntries; i++, totalBytesRead++) {
              if (entries[i].d_inumber != 0 && strncmp(entries[i].d_name, name, 14) == 0) {
                  *dirEnt = entries[i]; // copiar entrada encontrada
                  return 0;             // éxito
              }
          }

          blockNum++;
      }
  
      return -1; // no se encontró
  }