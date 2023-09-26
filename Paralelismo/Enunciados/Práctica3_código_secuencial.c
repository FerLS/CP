#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define DEBUG 0

/* Translation of the DNA bases
   A -> 0
   C -> 1
   G -> 2
   T -> 3
   N -> 4*/


#define M  1000000 // Number of sequences
#define N  200  // Number of bases per sequence

// The distance between two bases
int base_distance(int base1, int base2){

  if((base1 == 4) || (base2 == 4)){
    return 3;
  }

  if(base1 == base2) {
    return 0;
  }

  if((base1 == 0) && (base2 == 3)) {
    return 1;
  }

  if((base2 == 0) && (base1 == 3)) {
    return 1;
  }

  if((base1 == 1) && (base2 == 2)) {
    return 1;
  }

  if((base2 == 2) && (base1 == 1)) {
    return 1;
  }

  return 2;
}

int main(int argc, char *argv[] ) {

  int i, j;
  int *data1, *data2;
  int *result;
  struct timeval  tv1, tv2;

  data1 = (int *) malloc(M*N*sizeof(int));
  data2 = (int *) malloc(M*N*sizeof(int));
  result = (int *) malloc(M*sizeof(int));

  /* Initialize Matrices */
  for(i=0;i<M;i++) {
    for(j=0;j<N;j++) {
      data1[i*N+j] = (i+j)%5;
      data2[i*N+j] = 4-data1[i*N+j];
    }
  }

  gettimeofday(&tv1, NULL);

  for(i=0;i<M;i++) {
    result[i]=0;
    for(j=0;j<N;j++) {
      result[i] += base_distance(data1[i*N+j], data2[i*N+j]);
    }
  }

  gettimeofday(&tv2, NULL);
    
  int microseconds = (tv2.tv_usec - tv1.tv_usec)+ 1000000 * (tv2.tv_sec - tv1.tv_sec);

  /*Display result */
  if (DEBUG){
    for(i=0;i<M;i++) {
      printf(" %d \t ",result[i]);
    }
  } else {
    printf ("Time (seconds) = %lf\n", (double) microseconds/1E6);
  }    

  free(data1); free(data2); free(result);

  return 0;
}
