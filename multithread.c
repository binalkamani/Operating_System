#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "pthread.h"


/* Number of Threads */
#define num_thread 25     
/* Defining Array maximum size */
#define max_size 8000


/* Array of thread identifiers */
pthread_t tid[num_thread];
/* output array */
unsigned long int  matC[max_size][max_size];
/*input arrays*/
int matA[max_size][max_size];
int matB[max_size][max_size];

// ***********************************************************************
// Function for large scale multiplication that each thread will execute. 
// ***********************************************************************
static void* matrices_multiplication( void *arg )
{	
	int i, j, k, begin, end, count;
	
	/*Parameter passed is the thread number.  Since the parameter could be 
	anything, it is passed as void * and must be "converted".*/
	int thread_num = (int )(arg);

	/* Number of rows per thread to be calculated */
	count = (max_size/num_thread);
	
	/* Begin multiplication from this number of row */
	begin = (thread_num * count);
	/* End multiplication by this row */
	end = ((thread_num + 1) * count);
	
	/* Thread has been created and now it will start executing main part of this function */
	printf("Thread = %d is begining \n", thread_num);	
	
	/* Matrix multiplication */
	for( i = begin ; i < end; i++)
	{	
		for( j = 0; j < max_size; j++ )
		{
			for(k = 0; k < max_size; k++)
			{			
				matC[i][j] += (matA[i][k] * matB[k][j]);							
			}
		usleep (10);	// introducing delay of 10us between each operation
		}
	printf("Calculating row number = %d \n", i);				
	}
	
}

int main ()
{
	int total;
	int i,j, no_threads;
	
	/* To compute excution time of code */
	struct timeval start, stop;	
  	int random_number;
  	
	srand (time(NULL));
	
	gettimeofday(&start, NULL);
	
	/* Generating Matrices A and B by generating random numbers */
	for(i = 0; i<max_size; i++)
	{
		for(j = 0; j<max_size; j++)
		{
			matA[i][j] = rand()%10;
			matB[i][j] = rand()%10;			
		}		
	}
			
	
	/* Creating threads with little delay in between	*/
	
	for( total = 0; total < num_thread; total++) 
	{
		pthread_create( &(tid[total]), NULL, matrices_multiplication, (void *)total );
		usleep (100);		
	}


	for( i = 0; i < num_thread; i++ )
	{
		pthread_join( tid[i], NULL );
		printf("Thread %d is terminated \n", i);
	}
	
	gettimeofday(&stop, NULL);
	/* Run Time for this program*/
	printf ("Total time = %f seconds\n",
	     (double) (stop.tv_usec - start.tv_usec)/1000000 +
	     (double) (stop.tv_sec - start.tv_sec));
	
	
  return 0;
}
