#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>

/*
 * En esta parte hay que implementar la práctica 1 pero
 * con operaciones colectivas en vez de punto a punto. En
 * este caso, lo implementé con operaciones MPI_Bcast para
 * enviar n y L, que esta vez se leen por teclado (primero
 * la longitud de la cadena y luego la letra), y luego la
 * cadena después de ser inicializada. Luego se usa la
 * función MPI_Reduce con el campo op a MPI_SUM para hacer
 * la recolección de los datos que se muestran por pantalla.
*/

void inicializaCadena(char *cadena, int n) {
    int i;
    for (i = 0; i < n / 2; i++) {
        cadena[i] = 'A';
    }
    for (i = n / 2; i < 3 * n / 4; i++) {
        cadena[i] = 'C';
    }
    for (i = 3 * n / 4; i < 9 * n / 10; i++) {
        cadena[i] = 'G';
    }
    for (i = 9 * n / 10; i < n; i++) {
        cadena[i] = 'T';
    }
}

int main(int argc, char *argv[]) {
    // Variables necesarias.
    int i, n, count = 0, total_count = 0, numprocs, rank;
    char *cadena;
    char L;

    // Se inicia el MPI (Message Passing Interface).
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Si el proceso es el 0.
    if (rank == 0) {
        printf("Escribe la logitud de la cadena y luego la letra a buscar, con espacio entre medias:\n");
        // Se pide n y L por teclado y se inicializa la cadena.
        scanf("%d %c", &n,&L);

        cadena = malloc((n + 1) * sizeof(char));
        inicializaCadena(cadena, n);
        cadena[n] = '\0';

        printf("Letra a buscar: %c\nLongitud cadena: %d\nCadena generada: %s", L,n, cadena);
    }

    // Se envia lo recibido por teclado a cada proceso desde el root, que es el proceso 0.
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Como el proceso 0 ya tiene la cadena, el resto solo tiene q reservar el espacio y recibirla del proceso 0.
    if (rank != 0) {
        cadena = malloc((n + 1) * sizeof(char));
    }
    // Y se manda la cadena.
    MPI_Bcast(cadena, n+1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Todos los procesos buscan la letra con el paso i+=numprocs.
    for (i = rank; i < n; i += numprocs) {
        if (cadena[i] == L) {
            count++;
        }
    }

    // Luego se unen todas las sumas en una sola variable mediante MPI_Reduce con op MPI_SUM.
    MPI_Reduce(&count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Se muestra por pantalla el número de veces que aparece la letra L en la cadena.
    if (rank == 0) {
        printf("\nEl número de apariciones de la letra %c es %d\n", L, total_count);
    }

    // Finalmente se libera la memoria, se cierra el MPI y se acaba el programa.
    free(cadena);
    MPI_Finalize();
    return 0;
}
