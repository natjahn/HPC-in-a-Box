/**********************************/
/* Purpose: Sums all consecutive integers between a minimum and maximum value (inclusive)
  Author: Natalie Jahn
  Date last modified: Sep 14, 2019
  Notes: - Two command line arguments (min and max values)
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main ( int argc, char *argv[] );

int main (int argc, char *argv[]) {

  int i;  //loop counter
  int err;    //mpi initialization error variable
  int master = 0;   //denoted rank of master node
  int rank;   //rank of current process (0 to numProc-1)
  int numProcs; //number of processes
  long long int sum;    //sums integers in allocated range in each process

  //min and max values from user input
  long long int min;
  long long int max;
  int range;  //max-min - number of integers to be added

  //each process will add integers between procMin and procMax (inclusive)
  int procMin;
  int procMax;
  int procRange;  //number of integers between procMin and procMax

  long long int totalSum;   //each sum from each process is added to this

  double exec_time = 0.0; //program execution time

  MPI_Status status;

  clock_t start = clock();


/* Initialize MPI */
  err = MPI_Init(&argc, &argv);

  if (err != 0) {
    //error with MPI_INIT
    printf("\nMPI_Init fatal error.\n");
    exit(1);
  }

/* Get number of processes */
  MPI_Comm_size (MPI_COMM_WORLD, &numProcs);

/* Get rank of current proccess */
  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

//Master process says hello
   if (rank == master) {

     if (argc != 3) {
       printf("Program needs minimum and maximum integer values in command line!\n");
       return(0);
     }

     //make sure max > min
      if (atoi(argv[1]) >= atoi(argv[2])) {
       printf("Minimum value must be smaller than maximum value!\n");
       return 0;
      }

      printf("This program will add every integer between %s and %s using %d processes.\n\n", argv[1], argv[2], numProcs);

  }

    //get min and max values from command line arguments
    min = atoll(argv[1]);
    max = atoll(argv[2]);

    range = max - min + 1; //number of numbers (inclusive) between max and min

    procRange = range / numProcs; //number of numbers in subset of range proccess will be counting



/* each process takes a fraction of integers between min and max and sums them */

   procMin = (procRange * rank) + min;
   procMax = procMin + procRange - 1;

   //if last process, adds sum of its allocated min to the overall max
   if (rank == numProcs-1) {
     sum = 0.0;
     for (i = procMin; i <= max; i++) {
       sum = sum + i;
     }
   }
   else {
   //every other process adds between their allocated max and min
     sum = 0.0;
     for (i = procMin; i <= procMax; i++) {
       sum = sum + i;
     }
   }

   printf("\nProcess %d:\n", rank);
   printf("My sum contribution is %lld\n", sum);

   //add all sums from the individual processes
   MPI_Reduce(&sum, &totalSum, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);


/* print total sum (could be included in else statement above?) */
   if (rank == master) {
     printf("\n");
     printf("Master process:\n");
     printf("The total sum is %lld\n\n", totalSum);
   }


/* Terminate MPI */
  MPI_Finalize();
  if (rank == master) {
    printf("Master - end execution\n");
    clock_t end = clock();
    exec_time += (double)(start - end) / CLOCKS_PER_SEC;
    printf("\n--- %f seconds for program execution ---\n", exec_time);
  }

  return 0;

}
