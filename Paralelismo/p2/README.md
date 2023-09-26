# Práctica 2 #
En esta práctica se pide realizar una extensión de la práctica 1 hecha anteriormente.

## Enunciado (Parte 1) ##
Para ello se deben usar las funciones MPI (Message Passing Interface), en las que el proceso 0 debe 
hacer la entrada/salida (scanf/printf) y debe distribuir *n* y *L* al resto de procesos (con MPI_Send).
**Ahora con operación colectiva MPI!**

Nota: *n* es la longitud de la cadena y *L* la letra ser buscada.

La carga de trabajo se debe repartir con un bucle for con "paso" i+=numprocs en vez de i++.

Para terminar el proceso 0 debe recoger el número de aparicioenes detectada en cada proceso (con MPI_Recv).
**Ahora con operación colectiva MPI!**

Nota: MPI_Send y MPI_Recv no separan como deben la cadena, por lo tanto es probable que si
tienes un número de procesos que no divida (en números enteros ) a la cadena puede dar problemas.

## Enunciado (Parte 2) ##

Se pide implementar una función colectiva en árbol binomial, implementación que
denominaremos MPI BinomialColectiva, que utilizaremos *SOLO* en la distribución de n y L.

Para la implementación de Bcast con árbol binomial (MPI BinomialBcast): Mismos parámetros que MPI Bcast 
(consultar página man de MPI Bcast para obtener cabecera), asumiendo por simplicidad que el root es el 0.
En el paso “i” los procesos con myrank < 2⁽ᶦ⁻¹⁾ se comunican con el proceso myrank + 2⁽ᶦ⁻¹⁾.

Posteriormente implementaremos una función propia de colectiva *SOLO* para la recolección de count,
inicialmente utilizando las mismas operaciones de Send/Recv que en la implementación sin
colectivas (bucle for de Recv), implementación que denominaremos MPI FlattreeColectiva.

Nota: asumir que la operación a realizar será una suma. 

El resto de parámetros de la cabecera deben ser los mismos que los de la colectiva estándar de MPI (incluido controlar el error).
