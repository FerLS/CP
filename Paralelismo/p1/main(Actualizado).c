#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>

/*
 * Para ejecutarlo hay que tener instalada la librería OpenMPI.
 * Cuando lo tengas para compilarlo: mpicc main.c -o p1,
 * y para ejecutarlo: mpirun -np x --oversubscribe -/p1 y M
 * Siendo x el número de procesos, y la longitud de la cadena y
 * M la letra a buscar en la cadena.
 * Usa el oversubscribe xq a veces puede ser que no tenga suficientes
 * nodos para ejecutarse, asi se sobreescriben y puedes usar mas.
 */

// Función que inicializa la cadena de longitud n.
void inicializaCadena(char *cadena, int n){
    int i;
    for(i=0; i<n/2; i++){
        cadena[i] = 'A';
    }
    for(i=n/2; i<3*n/4; i++){
        cadena[i] = 'C';
    }
    for(i=3*n/4; i<9*n/10; i++){
        cadena[i] = 'G';
    }
    for(i=9*n/10; i<n; i++){
        cadena[i] = 'T';
    }
}

int main(int argc, char **argv) {
    // Si no tenemos 3 argumentos salimos del programa.
    if(argc != 3){
        printf("Número incorrecto de parámetros\nLa sintaxis debe ser: program n L\n"
               "  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n"
               "  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
        exit(1);
    }

    //Variables necesarias.
    // Para el MPI y contar número de letras.
    int rank, size, i, n, count, local_count = 0;
    // Letra a contar.
    char L;
    // Cadena.
    char *str;

    // Iniciamos el MPI (Message Passing Interface).
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) { // Si el proceso es el 0, se envían las variables al resto de procesos.

        L = *argv[2];
        n = atoi(argv[1]);

        str = malloc(n * sizeof(char));
        inicializaCadena(str, n);
        str[n]='\0';
        printf("Letra buscada: %c\n",L);
        printf("Cadena: %s\n",str);

        for (i = 1; i < size; i++) {
            MPI_Send( &L, 1, MPI_CHAR, i, 0, MPI_COMM_WORLD);
            MPI_Send(&n,1,MPI_INT,i,0,MPI_COMM_WORLD);
            MPI_Send(str,n,MPI_CHAR,i,0,MPI_COMM_WORLD);
        }

    }else{
        // Recibir n, L y la cadena.
        MPI_Recv(&L, 1, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        str=malloc(n*sizeof(char));
        MPI_Recv(str, n, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Contar las repeticiones de la letra en la cadena desde rank hasta n haciendo el paso.
    for (i = rank; i < n; i+=size) {
        if (str[i] == L) {
            local_count++;
        }
    }

    // Envía el resultado al proceso 0, y si es el proceso 0, se reciben todas las soluciones de los demás procesos.
    if (rank != 0) {
        MPI_Send(&local_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        // Esta línea es importante ya que si no se hace se pierde la cuenta hecha por el proceso 0.
        count = local_count;
        for (i = 1; i < size; i++) {
            MPI_Recv(&local_count, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            count += local_count;
        }
        printf("La letra '%c' aparece %d veces en la cadena.\n", L, count);
    }

    // Se liberan todos los malloc y se cierra el MPI.
    MPI_Finalize();
    free(str);

    return 0;
}
