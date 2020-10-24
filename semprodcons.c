#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <pthread.h> 
#include "semaphoresarr.h"

#define TAMBUFFER 5

#define VELPROD 1000000	// Microsegundos
#define VELCONS 5000000

void *productor(void *arg);
void *consumidor(void *arg);

char buffer[TAMBUFFER];
SEM_ID semarr;
enum {E_MAX,N_BLOK,S_EXMUT};  // Semáforos 0,1 y 2

int main()
{
    // Definición de variables
    int res;
    int n;
    pthread_t a_thread[2];     // Identificador del hilo
    void *thread_result;

	srand(getpid());
	
    // Creación del arreglo de semáforos
    semarr=createsemarray((key_t) 9234,3);

    initsem(semarr,E_MAX,TAMBUFFER);
    initsem(semarr,N_BLOK,0);
    initsem(semarr,S_EXMUT,1);

    printf("Semáforos creados\n");

    /* Aquí se crean los hilos */

    res = pthread_create(&(a_thread[0]), NULL, productor, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_create(&(a_thread[1]), NULL, consumidor, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }



    printf("Esperando que los hilos terminen...\n");
    for(n=0;n<=1;n++)
    {
        res=pthread_join(a_thread[n], &thread_result);
        if (res != 0)
        {
            perror("La unión del hilo ha fallado");
            exit(EXIT_FAILURE);
        }
    }


    // Borrar los semáforos

    erasesem(semarr);

    exit(EXIT_SUCCESS);
}


void *productor(void *arg)
{
    int n;

    printf("Inicia productor\n");
    for(n=0;n<100;n++)
    {
        semwait(semarr,E_MAX);	// Si se llena el buffer se bloquea
        semwait(semarr,S_EXMUT);	// Asegurar el buffer como sección crítoca

        buffer[n%TAMBUFFER]=65+n%26;
        printf("Productor produce %c\n",(char) 65+n%26);
        usleep(rand()%VELPROD);

        semsignal(semarr,S_EXMUT);	// Libera la sección crítica del buffer
        semsignal(semarr,N_BLOK);	// Si el consumidor está bloqueado porque el buffer está vacío, lo desbloqueas

        usleep(rand()%VELPROD);

    }
    pthread_exit(NULL);
}


void *consumidor(void *arg)
{
    int n;

    printf("Inicia Consumidor\n");
    for(n=0;n<100;n++)
    {
        semwait(semarr,N_BLOK);	// Si el buffer está vacío, se bloquea
        semwait(semarr,S_EXMUT);	// Asegura el buffer como sección crítica 

        printf("                   Consumidor consume %c\n",(char) buffer[n%TAMBUFFER]);
        usleep(rand()%VELCONS);

        semsignal(semarr,S_EXMUT);	// Libera la SC el buffer
        semsignal(semarr,E_MAX);	// Si el productor está bloqueado porque el buffer estaba lleno, lo desbloquea
        usleep(rand()%VELCONS);

    }
    pthread_exit(NULL);
}