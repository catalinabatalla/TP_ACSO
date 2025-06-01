#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COMMANDS 200

bool is_quote_char(char c) {
    // Comillas ASCII para probar todos los casos
    if (c == '"' || c == '\''){
        return true;
    }
    if ((unsigned char)c == 0x93 || (unsigned char)c == 0x94 || // “ ”
        (unsigned char)c == 0x91 || (unsigned char)c == 0x92){   // ‘ ’
        return true;
    }
    return false;
}

char matching_quote(char open) {
    switch(open) {
        case '"': return '"';
        case '\'': return '\'';
        // reemplazar comillas tipográficas por comillas ASCII
        default: return open;
    }
}

char** parse_args(char* command) {
    char** args = malloc(100 * sizeof(char*));
    int argc = 0;
    int i = 0;
    int len = strlen(command);
    
    while (i < len) {
        // Saltar espacios iniciales
        while (i < len && (command[i] == ' ' || command[i] == '\t')) i++;
        if (i >= len) break;

        char* start;
        char quote = 0;

        if (command[i] == '"' || command[i] == '\'') {
            quote = command[i++];
            start = &command[i];
            while (i < len && command[i] != quote) i++;
        } else {
            start = &command[i];
            while (i < len && command[i] != ' ' && command[i] != '\t') i++;
        }

        int length = &command[i] - start;
        char* arg = malloc(length + 1);
        strncpy(arg, start, length);
        arg[length] = '\0';
        args[argc++] = arg;

        if (quote && command[i] == quote) i++;  // Saltar comilla de cierre
    }

    args[argc] = NULL;
    return args;
}


// Reemplaza comillas tipográficas UTF-8 (3 bytes) por comillas ASCII dobles (")
void change_quotes(char* str) {
    char* p = str;
    char* dst = str;

    while (*p) {
        if ((unsigned char)p[0] == 0xE2 && (unsigned char)p[1] == 0x80) {
            if ((unsigned char)p[2] == 0x9C || (unsigned char)p[2] == 0x9D) {
                // “ o ”
                *dst++ = '"';
                p += 3;
                continue;
            }
            if ((unsigned char)p[2] == 0x98 || (unsigned char)p[2] == 0x99) {
                // ‘ o ’
                *dst++ = '\'';
                p += 3;
                continue;
            }
        }

        // Sino tiene comillas raras copia directo el carácter
        *dst++ = *p++;
    }
    *dst = '\0';
}

int main() {
    char command[256];
    char* commands[MAX_COMMANDS];
    int command_count;

    while (1) {
        if (isatty(STDIN_FILENO)) { //para correr los tests de Rama
            printf("Shell> ");
            fflush(stdout);
        }

        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;  // Salir si hay error o EOF
        }

        change_quotes(command);
        command[strcspn(command, "\n")] = '\0';

        // Salir si el comando es "quit" o "q" o "exit"
        if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0 || strcmp(command, "exit") == 0) {
            printf("Bye!\n");
            break;
        }
        // Parseo por pipes 
        command_count = 0;
        char* token = strtok(command, "|");
        while (token != NULL && command_count < MAX_COMMANDS) {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        int prev_fd = -1;
        int pipefd[2];

        for (int i = 0; i < command_count; i++) {
            // Crear pipe si no es el último comando
            if (i < command_count - 1) {
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    exit(1);
                }
            }

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                // Hijo

                // Si hay input de pipe anterior
                if (i > 0) {
                    dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }

                // Si no es el último, redirige stdout al pipe
                if (i < command_count - 1) {
                    close(pipefd[0]); // Cerrar
                    dup2(pipefd[1], STDOUT_FILENO);
                    close(pipefd[1]);
                }

                // Parsear argumentos y ejecutar
                char** args = parse_args(commands[i]);
                execvp(args[0], args);

                perror("execvp");
                exit(1);
            } else {
                // Padre
                if (i > 0) close(prev_fd); // Cerramos anterior read end
                if (i < command_count - 1) {
                    close(pipefd[1]); // Cerramos write end
                    prev_fd = pipefd[0]; // El read end pasa al próximo
                }
            }
        }

        // Esperar a todos los hijos
        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }
    }

    return 0;
}