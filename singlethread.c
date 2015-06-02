#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>


int count = 10000;
int i,j,k;
	
unsigned long int matC[10000][10000] = {0};
int matA[10000][10000] = {0};
int matB[10000][10000] = {0};
	
// ***********************************************************************
// Function for large scale multiplication that one thread will execute. 
// ***********************************************************************
	
void * thread1()
{
		
	for (i=0;i<count;i++){
        for(j=0;j<count;j++){
              for(k=0;k<count;k++){
                matC[i][j] = matC[i][j] + (matA[i][k] * matB[k][j]);
            }  
		usleep (10); 	// introducing delay of 10us between each operation
        } 
		if (i%200 == 0)
		printf("Calculating %d\n", i);	
    }
		/*
		printf("Matrix C BY Thread1: \n");
		printf("%d  %d  %d  %d\n", matC[0][0], matC[0][1], matC[0][2], matC[0][3]);
		printf("%d  %d  %d  %d\n", matC[99][99], matC[33][33], matC[66][66], matC[22][22]);
		*/
		return 0;
}

int main()
{

		/* Thread identifier */
		pthread_t tid1;

		srand ( time(NULL) );
		
		
		//Generating Matrices with random numbers from 0 to 9
 
		for(i=0; i<count; i++){
			for(j=0;j<count;j++){
			
			matA[i][j] = rand() % 10 ;	
			matB[i][j] = rand() % 10 ;
		
			}	
		}
		
		/* just for testing purpose
		
		printf("Matrix A : \n");
		printf("%d  %d  %d  %d\n", matA[0][0], matA[0][1], matA[0][2], matA[0][3]);
		printf("%d  %d  %d  %d\n", matA[1][0], matA[1][1], matA[1][2], matA[1][3]);
		printf("%d  %d  %d  %d\n", matA[2][0], matA[2][1], matA[2][2], matA[2][3]);
		printf("%d  %d  %d  %d\n", matA[3][0], matA[3][1], matA[3][2], matA[3][3]);
		
		printf("Matrix B : \n");
		printf("%d  %d  %d  %d\n", matB[0][0], matB[0][1], matB[0][2], matB[0][3]);
		printf("%d  %d  %d  %d\n", matB[1][0], matB[1][1], matB[1][2], matB[1][3]);
		printf("%d  %d  %d  %d\n", matB[2][0], matB[2][1], matB[2][2], matB[2][3]);
		printf("%d  %d  %d  %d\n", matB[3][0], matB[3][1], matB[3][2], matB[3][3]);
		*/		
		
		//Creating Thread
        pthread_create(&tid1,NULL,thread1,NULL);
		
		// Terminating Thread
        pthread_join(tid1,NULL);

        return 0;
}
