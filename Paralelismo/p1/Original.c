#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

int main(int argc, char *argv[])
{
  if(argc != 3){
    printf("Numero incorrecto de parametros\nLa sintaxis debe ser: program n L\n  program es el nombre del ejecutable\n  n es el tamaño de la cadena a generar\n  L es la letra de la que se quiere contar apariciones (A, C, G o T)\n");
    exit(1); 
  }
    
  int i, n, count=0;
  char *cadena;
  char L;

  n = atoi(argv[1]);
  L = *argv[2];
  
  cadena = (char *) malloc(n*sizeof(char));
  inicializaCadena(cadena, n);
  
  for(i=0; i<n; i++){
    if(cadena[i] == L){
      count++;
    }
  }
  
  printf("El numero de apariciones de la letra %c es %d\n", L, count);
  free(cadena);
  exit(0);
}
