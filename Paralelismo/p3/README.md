# Práctica 3 #
En esta práctica se va a implementar un programa en paralelo que reciba dos cadenas de ADN y compruebe la similaridad entre ellas.

Se debe hacer la medida de lo similares que son la secuencia i del primer conjunto y la secuencia i del segundo conjunto de ADN.

## Enunciado ##
Para ello se dividen las matrices entre p procesos, cada uno con rows = M/p filas
(por simplicidad, es mejor empezar con el caso en que consideraremos que el
número de procesos es múltiplo de M: M mod p = 0). Cada tarea se encargará de calcular 
M/p elementos del vector resultado. Luego se puede dar el paso a que se contemple todos
los procesos posibles.

Se debe realizar una implementación SPMD.En el que la inicializacioón de la matriz la hace
el proceso 0. Todos los mensajes se deben enviar y recibir con operaciones colectivas estándar de MPI.

La E/S (printf) del programa la debe hacer el proceso 0 y se debe imprimir por separado tiempo de 
comunicaciones y tiempo computación de cada proceso.
