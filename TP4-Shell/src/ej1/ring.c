#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if(argc != 4) {
        printf("Uso: anillo <n> <c> <s>\n");
        return 1;
    }
    int n = atoi(argv[1]);
    int c = atoi(argv[2]);
    int s = atoi(argv[3]);

    if(n < 3 || s < 0 || s >= n) {
        printf("Parametros invalidos\n");
        return 1;
    }

    int pipes[n][2];
    for(int i=0; i<n; i++)
        if(pipe(pipes[i]) == -1) { perror("pipe"); exit(1); }

    for(int i=0; i<n; i++) {
        if(fork() == 0) {
            // Cierro pipes que no uso
            for(int j=0; j<n; j++) {
                if(j == (i + n - 1) % n) {
                    close(pipes[j][1]);
                } else if(j == i) {
                    close(pipes[j][0]);
                } else {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            int val;
            int read_fd = pipes[(i + n - 1) % n][0];
            int write_fd = pipes[i][1];

            while(1) {
                ssize_t r = read(read_fd, &val, sizeof(int));
                if(r <= 0) {
                    printf("Proceso %d: EOF o error en lectura\n", i);
                    break;
                }

                printf("Proceso %d: recibió valor %d\n", i, val);

                if(i == s && val >= c + n) {
                    printf("Proceso %d (iniciador): Resultado final: %d\n", i, val);
                    // Ya no escribo más para cerrar el anillo
                    break;
                }

                val++;
                if(i == s && val >= c + n) {
					printf("Proceso %d (iniciador): Resultado final: %d\n", i, val);
					// No escribo más, cierro y salgo
					break;
				}

				printf("Proceso %d: incrementa a %d y envía\n", i, val);
				write(write_fd, &val, sizeof(int));
            }

            close(read_fd);
            close(write_fd);
            exit(0);
        }
    }

    // Padre cierra todos menos el pipe de escritura del proceso s
    for(int i=0; i<n; i++) {
        if(i == s) {
            close(pipes[i][0]);
        } else {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }

    printf("Padre: inicia escribiendo %d en proceso %d\n", c, s);
    write(pipes[s][1], &c, sizeof(int));
    close(pipes[s][1]); // Cierro para que los procesos detecten EOF cuando terminen

    for(int i=0; i<n; i++) wait(NULL);

    printf("Padre: todos los hijos terminaron\n");
    return 0;
}
