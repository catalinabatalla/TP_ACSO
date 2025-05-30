#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 200
#define MAX_CMDS 10

void free_args(char **args);


void remove_newline(char *str) {
    char *nl = strchr(str, '\n');
    if (nl) *nl = '\0';
}

int parse_pipeline(char *line, char *commands[]) {
    int count = 0;
    char *token = strtok(line, "|");
    while (token != NULL) {

        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ') *end-- = '\0';

        if (*token == '\0') {
            return -1; // Error de sintaxis: comando vacío
        }

        commands[count++] = token;
        token = strtok(NULL, "|");
    }
    return count;
}

char** parse_args_quoted(char *cmd) {
    char **args = malloc((MAX_ARGS + 1) * sizeof(char *));
    int argc = 0;
    char *p = cmd;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        if (argc > MAX_ARGS) {
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
            if (*p == quote) p++; // Saltar comilla final
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

void free_args(char **args) {
    for (int i = 0; args[i]; i++) free(args[i]);
    free(args);
}

void ejecutar_pipeline(char *commands[], int num_cmds) {
    int pipefd[2], in_fd = 0;

    for (int i = 0; i < num_cmds; i++) {
        pipe(pipefd);
        pid_t pid = fork();

        if (pid == 0) {
            if (in_fd != 0) {
                dup2(in_fd, 0);
                close(in_fd);
            }

            if (i < num_cmds - 1) {
                dup2(pipefd[1], 1);
                close(pipefd[1]);
            }

            close(pipefd[0]);

            char **args = parse_args_quoted(commands[i]);
            if (execvp(args[0], args) == -1) {
                printf("command not found\n");
                exit(1);
            }
        }

        // Proceso padre
        wait(NULL);
        close(pipefd[1]);
        if (in_fd != 0) close(in_fd);
        in_fd = pipefd[0];
    }
}

int main() {
    char line[MAX_LINE];

    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;

        remove_newline(line);

        if (strcmp(line, "exit") == 0) break;

        char *trim = line;
        while (*trim == ' ') trim++;
        if (*trim == '|') {
            printf("Syntax error\n");
            continue;
        }

        int len = strlen(line);
        while (len > 0 && line[len - 1] == ' ') len--;
        line[len] = '\0';
        if (len > 0 && line[len - 1] == '|') {
            printf("Syntax error\n");
            continue;
        }

        char *commands[MAX_CMDS];
        char line_copy[MAX_LINE];
        strcpy(line_copy, line);

        if (strstr(line, "||") != NULL) {
            printf("Syntax error\n");
            continue;
        }


        int num_cmds = parse_pipeline(line_copy, commands);
        if (num_cmds == -1) {
            printf("Syntax error\n");
            continue;
        }

        ejecutar_pipeline(commands, num_cmds);
    }

    return 0;
}
