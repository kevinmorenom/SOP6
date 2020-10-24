#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>


#define INICIAL 900000000
#define FINAL 1000000000
#define NTHREADS 4
int SIZE=0;
int start,end;
int PrimeCounter[NTHREADS];
int isprime(int n);

struct args_struct{
    int SIZE;
    int thread[NTHREADS];
};

void *tfunc( void * args){
	int i,j;
    int nthread=*((int *)args);
    int inicio=nthread*(SIZE/NTHREADS)+nthread+start;
    int fin=(nthread+1)*(SIZE/NTHREADS)+nthread+start;
    if (fin>end)fin=end;
	for(i=inicio;i<=fin;i++){
        if(isprime(i)){
            PrimeCounter[nthread]++;
            printf("%d \n",i);
    }
    }
				   
}

int main (int argc,char *argv[]){
    long long start_ts;
    long long stop_ts;
    long long elapsedTime;
    long lElapsedTime;
    struct timeval ts;

    gettimeofday(&ts, NULL);
    start_ts = ts.tv_sec; //Tiempo inicial

     start=atoi(argv[1]);
     end=atoi(argv[2]);
    SIZE=(end-start);


    pthread_t tid[NTHREADS];
    int  args[NTHREADS];

    for(int i=0;i<NTHREADS;i++){
        args[i]=i;
        pthread_create(&tid[i],0,tfunc,&args[i]);
    }

    for(int i=0;i<NTHREADS;i++){
		
		pthread_join(tid[i],NULL);
	}
    int answer=0;
    for(int i=0;i<NTHREADS;i++){
		
		answer+=PrimeCounter[i];
	}
    gettimeofday(&ts, NULL);
    stop_ts = ts.tv_sec; //Tiempo final

    elapsedTime = stop_ts - start_ts;
    printf("Tiempo total, %lld segundos\n", elapsedTime);
    printf("Se encontraron %d primos",answer);
}

int isprime(int n)
{
	//int d=3;
	int prime=1; //asumimos que es primo
	int limit=sqrt(n);// su divisor tiene que estar dentro de su raiz cuadrada.
	
	if(n<2)//si es menor a dos no es primo
		prime=0;
	else if(n==2){//si es dos si es primo
        prime=1;
    }
	else if(n%2==0)// si es par no es primo
		prime=0;
	else
	{
		for(int j=2;j<=limit;j++){
			if(n%j==0)//si encontramos otro divisor exacto, no es primo
			{
				prime=0;
				break;
			}
		}
	}
	return(prime);
}