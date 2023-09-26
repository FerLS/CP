#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi/mpi.h>

#define DEBUG 2

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4
*/

#define M  100 // Number of sequences
#define N  100  // Number of bases per sequence

unsigned int g_seed = 0;

int fast_rand(void) {
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) % 5;
}

// The distance between two bases
int base_distance(int base1, int base2) {

    if ((base1 == 4) || (base2 == 4)) {
        return 3;
    }

    if (base1 == base2) {
        return 0;
    }

    if ((base1 == 0) && (base2 == 3)) {
        return 1;
    }

    if ((base2 == 0) && (base1 == 3)) {
        return 1;
    }

    if ((base1 == 1) && (base2 == 2)) {
        return 1;
    }

    return 2;
}

int main(int argc, char *argv[]) {

    //Variables necesarias.
    int i, j, numprocs, rank, rows;
    int *data1, *data2;
    int *result;
    struct timeval tc1, tc2, tt1, tt2;
    long comp_time, trans_time;

    // Iniciamos el MPI.
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Aquí establecemos la longitud de cada parte a enviar, se utiliza el pudding, haciendo que si el número de
    // procesos no divide a la cadena se rellena un  trozo de memoria con "basura" para que no salte un error.
    rows = M / numprocs;
    if (M % numprocs != 0) {
        rows++;
    }

    // Si el rango es 0 se declaran las matrices y se inicializan.
    if (rank == 0) {
        data1 = (int *) malloc(rows * numprocs * N * sizeof(int));
        data2 = (int *) malloc(rows * numprocs * N * sizeof(int));
        result = (int *) malloc(rows * numprocs * sizeof(int));
        /* Initialize Matrices */
        for (i = 0; i < M; i++) {
            for (j = 0; j < N; j++) {
                data1[i * N + j] = fast_rand();
                data2[i * N + j] = fast_rand();
            }
        }
    }

    // Para cada proceso se inicializa una matriz local en la que se va a almacenar el trozo de la cadena que le toca.
    int *local_data1 = malloc(rows * N * sizeof(int));
    int *local_data2 = malloc(rows * N * sizeof(int));
    int *local_result = malloc(rows * sizeof(int));

    // Se envían los trozos de la cadena a cada proceso con MPI_Scatter y se mide su tiempo con time.h.
    gettimeofday(&tt1, NULL);
    MPI_Scatter(data1, rows * N, MPI_INT, local_data1, rows * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(data2, rows * N, MPI_INT, local_data2, rows * N, MPI_INT, 0, MPI_COMM_WORLD);
    gettimeofday(&tt2, NULL);

    trans_time = (tt2.tv_usec - tt1.tv_usec) + 1000000 * (tt2.tv_sec - tt1.tv_sec);

    // Se realizan las operaciones para calcular la distancia de las dos cadenas de ADN y se guardan en un resultado, midiendo su tiempo.
    gettimeofday(&tc1, NULL);
    for (i = 0; i < rows; i++) {
        local_result[i] = 0;
        for (j = 0; j < N; j++) {
            local_result[i] += base_distance(local_data1[i * N + j], local_data2[i * N + j]);
        }
    }
    gettimeofday(&tc2, NULL);

    comp_time = (tc2.tv_usec - tc1.tv_usec) + 1000000 * (tc2.tv_sec - tc1.tv_sec);

    //Luego se envían los resultados al proceso 0 con MPI_Gather y se mide su tiempo con time.h.
    gettimeofday(&tt1, NULL);
    MPI_Gather(local_result, rows, MPI_INT, result, rows, MPI_INT, 0, MPI_COMM_WORLD);
    gettimeofday(&tt2, NULL);

    trans_time += (tt2.tv_usec - tt1.tv_usec) + 1000000 * (tt2.tv_sec - tt1.tv_sec);

    /* Display result */
    if (DEBUG == 1) {
        if (rank == 0) {
            int checksum = 0;
            for (i = 0; i < M; i++) {
                checksum += result[i];
            }
            printf("Checksum: %d\n ", checksum);
        }
    } else if (DEBUG == 2) {
        if (rank == 0) {
            for (i = 0; i < M; i++) {
                printf(" %d \t ", result[i]);
            }
        }
    } else {
        printf("Process(%d):\t Comp_Time: %lf (seconds)\t Trans_Time %lf (seconds) \n", rank, (double) comp_time / 1E6,
               (double) trans_time / 1E6);
    }

    // Luego se libera la memoria de cada matriz inicializada en el proceso 0.
    if (rank == 0) {
        free(data1);
        free(data2);
        free(result);
    }

    // Y se liberan las matrices de cada proceso.
    free(local_data1);
    free(local_data2);
    free(local_result);

    // Se cierra el MPI y se finaliza el programa.
    MPI_Finalize();
    return 0;
}
