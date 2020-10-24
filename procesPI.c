//Autores:
// Jorge Alejandro Dong Llauger IS714046
// Kevin Antonio Moreno Melgoza IS714714
//Fecha: 15/09/2020
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include <sys/time.h>
#include "semaphores.h"


#define VELPROD 1000000	// Microsegundos
#define VELCONS 5000000
#define TAMBUFFER 7
#define INICIAL 900000000
#define FINAL 1000000000
#define NPROC 4
int SIZE=0;
int start,end;
int PrimeCounter[NPROC];
int isprime(int n);
enum {EXMUT,BARRERA};
int barrera;
SEM_ID semarr;
enum {E_MAX,N_BLOK,S_EXMUT};  // Semáforos 0,1 y 2

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

void sembarrier(int semid)
{
	struct sembuf s;
	int n;
	
	// semwait(semid,EXMUT);
	s.sem_num=EXMUT;
	s.sem_op=-1;
	s.sem_flg=0;
	semop(semid,&s,1);
	
	n=semctl(semid,BARRERA,GETNCNT,0); //Cuántos procesos hay bloqueados en barrera?

	s.sem_num=EXMUT;
	s.sem_op=1;
	s.sem_flg=0;
	semop(semid,&s,1);

	
	if(n<NPROC-1)
	{
		// WAIT
		s.sem_num=BARRERA; // número de semáforo en el arreglo
		s.sem_op=-1;
		s.sem_flg=0;
	
		semop(semid,&s,1);
	}
	else
	{
		s.sem_num=BARRERA; // número de semáforo en el arreglo
		s.sem_op=3;
		s.sem_flg=0;

		semop(semid,&s,1);		
	}
	return;
}

void *productor(void *args,double shm_id)
{
	double* buffer = (double*)shmat(shm_id,0,0); // apunta al inicio de la memoria
	if(buffer == (double*) -1){
		printf("shmat error\n");
	    exit(1);
	}

	int i,j;
    int nthread=*((int *)args);
    int inicio=nthread*(SIZE/NPROC)+nthread+start;
    int fin=(nthread+1)*(SIZE/NPROC)+nthread+start;
    if (fin>end)fin=end;
    int n,temp;
    // printf("Inicia productor %d\n",nthread);
    for(n=inicio;n<=fin;n++)
    {	
		if(isprime(n))
        {
			semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
			semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítica
			temp=buffer[5];
			buffer[temp]=n;
			if(buffer[5]!=4)
			{buffer[5]++;}
			else
			{buffer[5]=0;}
			// printf("Productor %d produce %d\n",nthread,n);
        	// printf("Buffer: %f %f %f %f %f %f %f \n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6]);

        	usleep(1000000);

			// usleep(rand()%5);

			semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
			semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas
		}

    }
	sembarrier(barrera);
	semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
	semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítica
	temp=buffer[5];
	buffer[temp]=-1;
	semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
	semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas
    exit(0);

}


void *consumidor(void *arg,double shm_id)
{
    int n,next2read,consume;
	double* buffer = (double*)shmat(shm_id,0,0); // apunta al inicio de la memoria
	if(buffer == (double*) -1){
		printf("shmat error\n");
	    exit(1);
	}
    printf("Inicia Consumidor\n");
	semwait(semarr,N_BLOK);	// Si el buffer está vacío, se bloquea
    semwait(semarr,S_EXMUT);	// Asegura el buffer como sección crítica
	int temp=buffer[6];
	next2read=buffer[temp];	
    while(next2read!=-1)
    {
        // printf("Buffer: %f %f %f %f %f %f %f \n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6]);
		
		//  for(n=0;n<buffer[5];n++)
		//  {
			if(next2read!=0){
        	printf("Consumidor consume %d\n",next2read);
			consume++;
			next2read=0;
			if(buffer[6]!=4)
			{
				buffer[6]++;
			}
			else
			{
				buffer[6]=0;
			}
			temp=buffer[6];
			next2read=buffer[temp];

			}
		//  }
        // usleep(rand()%VELCONS);
		printf("2read: %d\n",next2read);
        semsignal(semarr,S_EXMUT);	// Libera la SC el buffer
        semsignal(semarr,E_MAX);	// Si el productor está bloqueado porque el buffer estaba lleno, lo desbloquea
        usleep(1000000);

        // usleep(rand()%VELCONS);

    }
    semsignal(semarr,E_MAX);	// Si el productor está bloqueado porque el buffer estaba lleno, lo desbloquea
	printf("%d primos\n",consume);
	exit(0);
}



int main(int argc,char *argv[]){
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

	int params[NPROC];
	int p,ret;
	double shm_id;  //id de espacio en memoria 
	shm_id = shmget(IPC_PRIVATE, TAMBUFFER*sizeof(double), IPC_CREAT | 0666);// crea espacio en memoria. (espacio desocupado,tamaño a reservar,permisos de escritua y lectura)
	if (shm_id < 0) { // en caso de error son shmget
	     printf("shmget error\n");
	     exit(1);
	}

	barrera=createsemarray(0x1234,2);
	initsem(barrera,EXMUT,1);
	initsem(barrera,BARRERA,0);

	semarr=createsemarray((key_t) 9234,3);
    initsem(semarr,E_MAX,5);
    initsem(semarr,N_BLOK,0);
    initsem(semarr,S_EXMUT,1);

	//apuntador a double, apunta al espacio en memoria
	double* addr = (double*)shmat(shm_id,0,0); 
	for(int i=0;i<4;i++)
	{
		params[i]=i;
		p=fork();
		if(p==0){
			productor(&params[i],shm_id); // mandar a llamar la funcion en cada proceso hijo
		}
	}

	int cons=fork();
	if(cons==0){
		consumidor(&params[0],shm_id);
	}
	
	for(int i=0;i<5;i++)
	{
		ret=wait(NULL); // esperar a todos los procesos
	}

    printf("Final Buffer: %f %f %f %f %f %f %f\n",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5],addr[6]);

	erasesem(barrera);
	erasesem(semarr);
	
    gettimeofday(&ts, NULL);
    stop_ts = ts.tv_sec; //Tiempo final
    elapsedTime = stop_ts - start_ts;
    printf("Tiempo total, %lld segundos\n", elapsedTime);
     


}

