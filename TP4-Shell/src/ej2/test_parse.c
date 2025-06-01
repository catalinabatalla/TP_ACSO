#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 5  // Cambiá este valor para probar diferentes límites

// Prototipos (asumiendo tus funciones)
char **parse_args_quoted(char *cmd);
void free_args(char **args);

// Mock simple de free_args (igual al tuyo)
void free_args(char **args) {
    for (int i = 0; args[i]; i++) free(args[i]);
    free(args);
}

// Función que parsea argumentos con límite MAX_ARGS, como en tu shell
char** parse_args_quoted(char *cmd) {
    char **args = malloc((MAX_ARGS + 1) * sizeof(char *));
    int argc = 0;
    char *p = cmd;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        if (argc >= MAX_ARGS) {
            printf("Too many arguments\n");
            free_args(args);
            exit(1);
        }

        char *start;
        int len = 0;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            start = p;
            while (*p && *p != quote) p++, len++;
            args[argc] = strndup(start, len);
            if (*p == quote) p++;
        } else {
            start = p;
            while (*p && *p != ' ' && *p != '\t') p++, len++;
            args[argc] = strndup(start, len);
        }

        argc++;
    }

    args[argc] = NULL;
    return args;
}

// Test que imprime argumentos parseados
void test_parse_args(const char *input) {
    printf("Input: %s\n", input);
    // Necesitamos una copia modificable
    char *line = strdup(input);
    char **args = parse_args_quoted(line);

    printf("Parsed args (%d):\n", MAX_ARGS);
    for (int i = 0; args[i] != NULL; i++) {
        printf("  arg[%d] = '%s'\n", i, args[i]);
    }
    free_args(args);
    free(line);
}

int main() {
    printf("=== Test con menos o igual a MAX_ARGS argumentos ===\n");
    test_parse_args("uno dos 'tres cuatro' cinco");

    printf("\n=== Test con exactamente MAX_ARGS argumentos ===\n");
    test_parse_args("1 2 3 4 5");

    printf("\n=== Test con más de MAX_ARGS argumentos (debería fallar) ===\n");
    test_parse_args("1 2 3 4 5 6");

    return 0;
}
