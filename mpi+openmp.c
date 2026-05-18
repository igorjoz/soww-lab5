#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <omp.h>
#define RANGESIZE 10000
#define DATA 0
#define RESULT 1
#define FINISH 2

long is_prime(long n)
{
  if (n <= 1)
    return 0;
  if (n <= 3)
    return 1;
  if (n % 2 == 0 || n % 3 == 0)
    return 0;
  for (long i = 5; i * i <= n; i = i + 6)
    if (n % i == 0 || n % (i + 2) == 0)
      return 0;
  return 1;
}

long CountTwinPrimes(long a, long b)
{
  long sum = 0;

  if (a <= 3 && b >= 3)
  {
    sum++;
  }

  long start = a;
  if (start < 5)
  {
    start = 5;
  }
  while (start % 6 != 5)
  {
    start++;
  }

#pragma omp parallel for reduction(+ : sum) schedule(dynamic, 256)
  for (long i = start; i <= b; i += 6)
  {
    if (is_prime(i) && is_prime(i + 2))
    {
      sum++;
    }
  }
  return sum;
}

int main(int argc, char **argv)
{

  Args ins__args;
  parseArgs(&ins__args, &argc, argv);

  // set number of threads
  omp_set_num_threads(ins__args.n_thr);

  // program input argument
  long inputArgument = ins__args.arg;
  struct timeval ins__tstart, ins__tstop;

  int threadsupport;
  int myrank, proccount;

  long a = 1, b = inputArgument;
  long range[2];
  long result = 0, resulttemp;
  int sentcount = 0;
  int i;
  MPI_Status status;

  // Initialize MPI with desired support for multithreading -- state your desired support level

  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &threadsupport);

  if (threadsupport < MPI_THREAD_FUNNELED)
  {
    printf("\nThe implementation does not support MPI_THREAD_FUNNELED, it supports level %d\n", threadsupport);
    MPI_Finalize();
    return -1;
  }

  // obtain my rank
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  // and the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &proccount);

  if (!myrank)
    gettimeofday(&ins__tstart, NULL);
  // run your computations here (including MPI communication and OpenMP stuff)
  if (myrank == 0)
  {
    range[0] = a;

    // first distribute some ranges to all slaves
    for (i = 1; i < proccount; i++)
    {
      range[1] = range[0] + RANGESIZE - 1;
      if (range[1] > b)
        range[1] = b;

      // send it to process i
      MPI_Send(range, 2, MPI_LONG, i, DATA, MPI_COMM_WORLD);
      sentcount++;

      if (range[1] >= b)
      {
        // If we reached b, just set range[0] > b so we don't send real data next
        range[0] = b + 1;
      }
      else
      {
        range[0] = range[1] + 1;
      }
    }

    while (range[0] <= b)
    {
      // distribute remaining subranges to the processes which have completed their parts
      MPI_Recv(&resulttemp, 1, MPI_LONG, MPI_ANY_SOURCE, RESULT,
               MPI_COMM_WORLD, &status);
      result += resulttemp;

      // check the sender and send some more data
      range[1] = range[0] + RANGESIZE - 1;
      if (range[1] > b)
        range[1] = b;

      MPI_Send(range, 2, MPI_LONG, status.MPI_SOURCE, DATA,
               MPI_COMM_WORLD);
      range[0] = range[1] + 1;
    }

    // now receive results from the processes you sent tasks to
    for (i = 0; i < sentcount; i++)
    {
      MPI_Recv(&resulttemp, 1, MPI_LONG, MPI_ANY_SOURCE, RESULT,
               MPI_COMM_WORLD, &status);
      result += resulttemp;
    }
    // shut down the slaves
    for (i = 1; i < proccount; i++)
    {
      MPI_Send(NULL, 0, MPI_LONG, i, FINISH, MPI_COMM_WORLD);
    }
    // now display the result
    printf("\nHi, I am process 0, the result is %ld\n", result);
  }
  else
  { // slave
    // this is easy - just receive data and do the work
    do
    {
      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

      if (status.MPI_TAG == DATA)
      {
        MPI_Recv(range, 2, MPI_LONG, 0, DATA, MPI_COMM_WORLD,
                 &status);
        // compute my part
        resulttemp = CountTwinPrimes(range[0], range[1]);
        // send the result back
        MPI_Send(&resulttemp, 1, MPI_LONG, 0, RESULT,
                 MPI_COMM_WORLD);
      }
    } while (status.MPI_TAG != FINISH);
  }

  // synchronize/finalize your computations

  if (!myrank)
  {
    gettimeofday(&ins__tstop, NULL);
    ins__printtime(&ins__tstart, &ins__tstop, ins__args.marker);
  }

  MPI_Finalize();
}
