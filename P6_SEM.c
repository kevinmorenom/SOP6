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
#include "listas.h"


#define TAMBUFFER 7 //5 espacios para primos, 1 referencia de lectura, 1 referencia de escritura
#define NPROC 4 //numero de procesos

int SIZE=0;//cantidad de números en el rango
int start,end;//limites del rango
enum {EXMUT,BARRERA};//enumeración
int barrera;//arreglo de semaforos
SEM_ID semarr;//arreglo de semaforos
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

void sembarrier(int semid)//barrera de semaforos
{
	struct sembuf s;
	int n;
	
	// WAIT A EXMUT;
	s.sem_num=EXMUT;
	s.sem_op=-1;
	s.sem_flg=0;
	semop(semid,&s,1);
	
	n=semctl(semid,BARRERA,GETNCNT,0); //Cuántos procesos hay bloqueados en barrera?

	// SIGNAL A EXMUT;
	s.sem_num=EXMUT;
	s.sem_op=1;
	s.sem_flg=0;
	semop(semid,&s,1);

	
	if(n<NPROC-1)//si todavía no estan todos los procesos -1 bloqueados, bloqueo el proceso
	{
		// WAIT
		s.sem_num=BARRERA; // número de semáforo en el arreglo
		s.sem_op=-1;
		s.sem_flg=0;
	
		semop(semid,&s,1);
	}
	else //si solo falto yo de bloquear, desbloqueo todos
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

    int nthread=*((int *)args); //Numero de proceso
    int inicio=nthread*(SIZE/NPROC)+nthread+start;//limite inferior
    int fin=(nthread+1)*(SIZE/NPROC)+nthread+start;//limite superior
    if (fin>end)fin=end;//ajuste de limite superior
    int n,temp;
    for(n=inicio;n<=fin;n++)
    {	
		if(isprime(n))//si es primo
        {
			semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
			semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítica
			temp=buffer[5];//referencia de escritura
			buffer[temp]=n;//escribir en el buffer
			if(buffer[5]!=4)//actualizar referencia de escritura (buffer redondo)
			{
				buffer[5]++;
			}
			else
			{
				buffer[5]=0;
			}
			printf("Productor %d produce %d\n",nthread,n);
        	usleep(1000000);
			semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
			semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas
		}

    }
	sembarrier(barrera);//Una vez que termina el proceso entra a la barrera
	//SECCION CRÍTICA
	semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
	semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítica
		temp=buffer[5];
		buffer[temp]=-1;//escribe terminador en buffer
	semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
	semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas
	//FIN SECCION CRÍTICA
    
	exit(0);

}


void *consumidor(void *arg,double shm_id)
{
	ptrLista lista = NULL; //apuntador a lista
	ptrNodo nodo = NULL; //apuntador a nodo
    int n,next2read; //referencia de lectura
	int consume=0;//contador deconsumo
	double* buffer = (double*)shmat(shm_id,0,0); // apunta al inicio de la memoria
	if(buffer == (double*) -1){
		printf("shmat error\n");
	    exit(1);
	}
	semwait(semarr,N_BLOK);	// Si el buffer está vacío, se bloquea
    semwait(semarr,S_EXMUT);	// Asegura el buffer como sección crítica
	int temp=buffer[6];//referencia de lectura
	next2read=buffer[temp];	//dato leído
    while(next2read!=-1)///si el dato no es el terminador, consumimos
    {
			if(next2read!=0){ //aseguramos que no consumamos 0
        	printf("\t\t\t\tConsumidor consume %d\n",next2read);
			insertar_orden(next2read,&lista,nodo);//insertamos en orden en la lista enlazada
			consume++; //aumentamos contador
			next2read=0;
			if(buffer[6]!=4) //actualizamos referencia de lectura (buffer redondo)
			{
				buffer[6]++;
			}
			else
			{
				buffer[6]=0;
			}
			temp=buffer[6];
			next2read=buffer[temp];//siguiente dato a leer

			}
        semsignal(semarr,S_EXMUT);	// Libera la SC el buffer
        semsignal(semarr,E_MAX);	// Si el productor está bloqueado porque el buffer estaba lleno, lo desbloquea
        usleep(1000000);

    }
    semsignal(semarr,E_MAX);	// Si el productor está bloqueado porque el buffer estaba lleno, lo desbloquea
	printf("---------------------------\n");
	printf("Se encontraron %d primos\n",consume);
	printf("---------------------------\n");
	printf("Los números primos son:\n");

	nodos_lista(lista);//Mostramos la lista de números primos encontrados
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

	 start=atoi(argv[1]); //limite inferior de rango
     end=atoi(argv[2]); //limite superior de rango
     SIZE=(end-start);// cantidad de numeros en el rango

	int params[NPROC];
	int p,ret;
	double shm_id;  //id de espacio en memoria 
	shm_id = shmget(IPC_PRIVATE, TAMBUFFER*sizeof(double), IPC_CREAT | 0666);// crea espacio en memoria. (espacio desocupado,tamaño a reservar,permisos de escritua y lectura)
	if (shm_id < 0) { // en caso de error son shmget
	     printf("shmget error\n");
	     exit(1);
	}

	barrera=createsemarray(0x1234,2);//inicializacion de semáforos
	initsem(barrera,EXMUT,1);
	initsem(barrera,BARRERA,0);

	semarr=createsemarray((key_t) 9234,3);//inicialización de semáforos
    initsem(semarr,E_MAX,5); //valor inicial 5
    initsem(semarr,N_BLOK,0);
    initsem(semarr,S_EXMUT,1);

	//apuntador a double, apunta al espacio en memoria
	double* addr = (double*)shmat(shm_id,0,0); 
	//Creación de procesos productores
	for(int i=0;i<4;i++)
	{
		params[i]=i;
		p=fork();
		if(p==0){
			productor(&params[i],shm_id); // mandar a llamar la funcion en cada proceso hijo
		}
	}
	//creación de proceso consumidor
	int cons=fork();
	if(cons==0){
		consumidor(&params[0],shm_id);
	}
	
	for(int i=0;i<5;i++)
	{
		ret=wait(NULL); // esperar a todos los procesos (4 productores, 1 consumidor)
	}

	//Eliminación de semáforos
	erasesem(barrera);
	erasesem(semarr);

	erasesem(E_MAX);
	erasesem(N_BLOK);
	erasesem(S_EXMUT);
	
    gettimeofday(&ts, NULL);
    stop_ts = ts.tv_sec; //Tiempo final
    elapsedTime = stop_ts - start_ts;
    printf("Tiempo total, %lld segundos\n", elapsedTime);
     


}

