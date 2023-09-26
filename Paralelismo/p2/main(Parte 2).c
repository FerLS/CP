#include <stdio.h>
#include <stdlib.h>
#include <mpi/mpi.h>
#include <math.h>

/*
 * En esta práctica nos piden que hagamos lo mismo que en
 * las prácticas anteriores pero esta ver hagamos nosotros
 * las funciones MPI gestionando los envíos y recibos. Para
 * ello implementamos MPI_BinomialColectiva, que hace lo mismo
 * que MPI_Bcast pero con forma de árbol binomial, y la función
 * MPI_FlattreeColectiva que hace lo mismo que MPI_Reduce. Se 
 * puede intercambiar las funciones propias por las originales 
 * de MPI y funciona exactamente igual en todos los casos.
 */

// Esta función reemplaza a MPI_Bcast con sus mismos argumentos exactos, suponemos que el root es el proceso 0.
int MPI_BinomialColectiva(void * buf, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int numprocs , rank,error;

    // Se inicia el MPI.
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Aqui vamos a usar una estructura de árbol binario tal como se describe en el enunciado.
    for(int i=1; pow(2,i-1) <= numprocs; i++){
        // Si el proceso está dentro del grupo de los que puede enviar y existe destinatario se envía el mensaje con MPI_Send.
        if(rank < pow(2,i-1) && rank+pow(2,i-1) < numprocs){
            error=MPI_Send(buf, count, datatype, rank+(int)pow(2,i-1), 0, comm);
            if(error!=MPI_SUCCESS){
                return error;
            }
        }

        // Y los destinatarios reciben el mensaje con MPI_Recv.
        if(rank >= pow(2,i-1) && rank < pow(2,i)){
            error=MPI_Recv(buf, count, datatype, rank-(int)pow(2,i-1), 0, comm, MPI_STATUS_IGNORE);
            if(error!=MPI_SUCCESS){
                return error;
            }
        }
    }
    // Si no hay ningún problema se manda un MPI_SUCCESS.
    return MPI_SUCCESS;
}

// Esta función sustituye a la función MPI_Reduce.
int MPI_FlattreeColectiva(void * buff, void *recvbuff, int count,MPI_Datatype datatype, int root, MPI_Comm comm){
    // Variables necesarias
    int numprocs,rank,error;
    int count1,total_count;

    //Realizamos el control de errores.

    if(datatype != MPI_INT){
        return MPI_ERR_TYPE;
    }

    if(comm != MPI_COMM_WORLD){
        return MPI_ERR_COMM;
    }

    if(count == 0){
        return MPI_ERR_COUNT;
    }

    if(buff == NULL){
        return MPI_ERR_BUFFER;
    }

    // Se inicia el MPI.
    MPI_Comm_size(comm,&numprocs);
    MPI_Comm_rank(comm,&rank);

    // Si el proceso es el 0 se reciben las sumas del resto de procesos.
    if(rank==root){
        // Aquí se pone la suma del proceso 0 para no saltarlo.
        total_count = *(int*) buff;
        // Se van recibiendo las sumas y se suman a totalcount.
        for(int i=0;i<numprocs;i++){
            if(i!=root){
                error=MPI_Recv(&count1,count,datatype,MPI_ANY_SOURCE,0,comm,MPI_STATUS_IGNORE);
                if(error!=MPI_SUCCESS){
                    return error;
                }
                total_count+= count1;
            }
        }
        *(int*) recvbuff = total_count;
    }else{
        // Si no es el proceso 0, envía su suma al proceso 0.
        error=MPI_Send(buff,1,datatype,root,0,comm);
        if(error!=MPI_SUCCESS){
            return error;
        }
    }
    // Y salimos.
    return MPI_SUCCESS;
}

// Función que inicializa la cadena.
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
    // MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_BinomialColectiva(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_BinomialColectiva(&L, 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Como el proceso 0 ya tiene la cadena, el resto solo tiene q reservar el espacio y recibirla del proceso 0.
    if (rank != 0) {
        cadena = malloc((n + 1) * sizeof(char));
    }
    // Y se manda la cadena.
    // MPI_Bcast(cadena, n+1, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_BinomialColectiva(cadena, n+1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Todos los procesos buscan la letra con el paso i+=numprocs.
    for (i = rank; i < n; i += numprocs) {
        if (cadena[i] == L) {
            count++;
        }
    }

    // Luego se unen todas las sumas en una sola variable mediante MPI_Reduce con op MPI_SUM.
    //MPI_Reduce(&count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_FlattreeColectiva(&count, &total_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Se muestra por pantalla el número de veces que aparece la letra L en la cadena.
    if (rank == 0) {
        printf("\nEl número de apariciones de la letra %c es %d\n", L, total_count);
    }

    // Finalmente se libera la memoria, se cierra el MPI y se acaba el programa.
    free(cadena);
    MPI_Finalize();
    return 0;
}
