#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#define RANGESIZE 1
#define DATA 0
#define RESULT 1
#define FINISH 2

double
f (double x)
{
    return sin (x) * sin (x) / x;
}

double
SimpleIntegration (double a, double b, double precision)
{
    long j = (b-a)/precision;
    int i=0;
    double sum = 0;
    #pragma omp parallel for private(i) reduction(+:sum)
    for (i = 0; i < j; i ++)
    sum += f (a+i*precision) * precision;
    return sum;
}

int main(int argc,char **argv) {

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  //set number of threads
  omp_set_num_threads(ins__args.n_thr);
  
  //program input argument
  long inputArgument = ins__args.arg; 
  double precision=1.0/inputArgument;
  struct timeval ins__tstart, ins__tstop;

  int threadsupport;
  int myrank,proccount;


  double a = 1, b = 100;
  double range[2];
  double result = 0, resulttemp;
  int sentcount = 0;
  int i;
  MPI_Status status;


  // Initialize MPI with desired support for multithreading -- state your desired support level

  MPI_Init_thread(&argc, &argv,MPI_THREAD_FUNNELED,&threadsupport); 

  if (threadsupport<MPI_THREAD_FUNNELED) {
    printf("\nThe implementation does not support MPI_THREAD_FUNNELED, it supports level %d\n",threadsupport);
    MPI_Finalize();
    return -1;
  }
  
  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD,&proccount);

  if (!myrank) 
    gettimeofday(&ins__tstart, NULL);
  // run your computations here (including MPI communication and OpenMP stuff)
if (myrank == 0)
    {
        range[0] = a;

  // first distribute some ranges to all slaves
  for (i = 1; i < proccount; i++)
  {
            range[1] = range[0] + RANGESIZE;

      // send it to process i
      MPI_Send (range, 2, MPI_DOUBLE, i, DATA, MPI_COMM_WORLD);
      sentcount++;
      range[0] = range[1];
  }
  do
  {
      // distribute remaining subranges to the processes which have completed their parts
      MPI_Recv (&resulttemp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT,
      MPI_COMM_WORLD, &status);
      result += resulttemp;

            // check the sender and send some more data
      range[1] = range[0] + RANGESIZE;
      if (range[1] > b)
              range[1] = b;

      MPI_Send (range, 2, MPI_DOUBLE, status.MPI_SOURCE, DATA,
            MPI_COMM_WORLD);
      range[0] = range[1];
  }

  while (range[1] < b);
  // now receive results from the processes
        for (i = 0; i < (proccount - 1); i++)
  {
      MPI_Recv (&resulttemp, 1, MPI_DOUBLE, MPI_ANY_SOURCE, RESULT,
      MPI_COMM_WORLD, &status);
      result += resulttemp;
  }
  // shut down the slaves
  for (i = 1; i < proccount; i++)
  {
            MPI_Send (NULL, 0, MPI_DOUBLE, i, FINISH, MPI_COMM_WORLD);
  }
        // now display the result
        printf ("\nHi, I am process 0, the result is %f\n", result);
    }
    else
    {       // slave
        // this is easy - just receive data and do the work
  do
  {
            MPI_Probe (0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (status.MPI_TAG == DATA)
      {
                MPI_Recv (range, 2, MPI_DOUBLE, 0, DATA, MPI_COMM_WORLD,
        &status);
    // compute my part
    resulttemp = SimpleIntegration (range[0], range[1], precision);
    // send the result back
    MPI_Send (&resulttemp, 1, MPI_DOUBLE, 0, RESULT,
        MPI_COMM_WORLD);
      }
  }
  while (status.MPI_TAG != FINISH);
    }


  // synchronize/finalize your computations

  if (!myrank) {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }
    
  MPI_Finalize();
  
}
