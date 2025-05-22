#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>

#define MAX_COMMANDS 200

bool is_quote_char(char c) {
    // Comillas ASCII
    if (c == '"' || c == '\''){
        return true;
    }
    // Comillas tipográficas 
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
        // En UTF-8 las comillas tipográficas son multibyte
        // Para ejemplo simple, sólo soporte ASCII
        default: return open;
    }
}

char** parse_args(char* command) {
    char** args = malloc(100 * sizeof(char*));
    int i = 0;
    int len = strlen(command);
    int pos = 0;

    while (pos < len) {
        // Saltar espacios
        while (pos < len && command[pos] == ' ') pos++;
        if (pos >= len) break;

        char quote_char = 0;
        if (command[pos] == '"' || command[pos] == '\'') {
            quote_char = command[pos];
            pos++;
        }

        int start = pos;
        int end = pos;
        if (quote_char) {
            // Leer hasta cierre de comillas
            while (end < len && command[end] != quote_char) end++;
            if (end >= len) {
                // No encontró comillas de cierre, se toma hasta fin
                end = len;
            }
            // Extraer string entre comillas
            int arg_len = end - start;
            char* arg = malloc(arg_len + 1);
            strncpy(arg, command + start, arg_len);
            arg[arg_len] = '\0';
            args[i++] = arg;
            pos = end + 1;  // avanzar después de cierre
        } else {
            // Leer hasta siguiente espacio
            while (end < len && command[end] != ' ') end++;
            int arg_len = end - start;
            char* arg = malloc(arg_len + 1);
            strncpy(arg, command + start, arg_len);
            arg[arg_len] = '\0';
            args[i++] = arg;
            pos = end;
        }
    }
    args[i] = NULL;
    return args;
}

// Reemplaza comillas tipográficas UTF-8 (3 bytes) por comillas ASCII dobles (")
void change_quotes(char* str) {
    char* p = str;
    char* dst = str;

    while (*p) {
        // Comillas tipográficas 3 bytes UTF-8
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

        // Sino copia el carácter tal cual
        *dst++ = *p++;
    }
    *dst = '\0';
}

int main() {
    char command[256];
    char* commands[MAX_COMMANDS];
    int command_count;

    while (1) {
        printf("Shell> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\n");
            break;  // EOF con Ctrl+D
        }

        change_quotes(command);
        command[strcspn(command, "\n")] = '\0';

        // Salir si el comando es "quit"
        if (strcmp(command, "quit") == 0 || strcmp(command, "q") == 0) {
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
                    close(pipefd[0]); // Cerramos lectura
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
