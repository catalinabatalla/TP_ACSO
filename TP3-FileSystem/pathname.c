
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (pathname == NULL || pathname[0] != '/') {
        fprintf(stderr, "pathname_lookup: path must be absolute\n");
        return -1;
    }

    // Copia local del path porque vamos a modificarla con strtok
    char pathcopy[1024];
    strncpy(pathcopy, pathname, sizeof(pathcopy));
    pathcopy[sizeof(pathcopy) - 1] = '\0';

    int current_inumber = 1; // raíz siempre es el inodo 1

    char *token = strtok(pathcopy, "/");
    while (token != NULL) {
        struct direntv6 entry;
        if (directory_findname(fs, token, current_inumber, &entry) < 0) {
            return -1; // no se encontró el componente
        }
        current_inumber = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    return current_inumber;
}