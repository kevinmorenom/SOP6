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
#include <sys/msg.h>
#include <string.h>
#include "listas.h"

#define NPROC 4 //Numero de procesos

typedef struct {
	long msg_type;		// Tipo de mensaje, debe ser long
	char mensaje[100];	// Contenido
} MSGTYPE; 

int msgqid; //cola o buzón de mensajes

int SIZE=0; //cantidad de numeros en el rango
int start,end;//inicio y fin de rango


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


void *emisor(void *args)
{
    int nthread=*((int *)args); //N´umero de proceso actual
    int inicio=nthread*(SIZE/NPROC)+nthread+start;//limite inferior
    int fin=(nthread+1)*(SIZE/NPROC)+nthread+start;//limite superior
    if (fin>end)fin=end;//ajustar el limite superior
	MSGTYPE m;// instancia de estructura mensaje
    for(int n=inicio;n<=fin;n++)
    {	
		if(isprime(n))//verificacion de primaridad
        {
			m.msg_type=1;//tipo de mensaje, Valor entero positivo
			sprintf(m.mensaje,"%d",n); //Definir mensaje en la estructura
			msgsnd(msgqid,&m,sizeof(MSGTYPE)-sizeof(long),0);//enviar mensaje (buzon,mensaje,tamaño,msgflag)
			printf("Mensaje enviado por %d primo:%d\n",nthread,n);
			sleep(1);
		}
    }
	m.msg_type=1;
	strcpy(m.mensaje,"Fin");//Mandamos mensaje de que el proceso termino
	msgsnd(msgqid,&m,sizeof(MSGTYPE)-sizeof(long),0);
    exit(0);

}


void *receptor()
{
	int primos=0;//contador de primos
	int finales=0;//Contador de procesos terminados
	 
	ptrLista lista = NULL;//apuntador a lista
	ptrNodo nodo = NULL;//apuntador a nodo
	
    MSGTYPE m;	// Donde voy a recibir el mensaje
	int retval;// valor de retorno
	
	do
	{
		retval=msgrcv(msgqid,&m,sizeof(MSGTYPE)-sizeof(long),1,0);//Recibir mensaje. recieve bloqueante.
		if(strcmp(m.mensaje,"Fin")==0){
			finales++;//aumentar contador de procesos terminados
		}
		else{
			primos++;//aumentar contador de primos
			insertar_orden(atoi(m.mensaje),&lista,nodo);//insertar primo en lista enlazada
			printf("\t\t\t\t\tRecibido: %s\n",m.mensaje);

		}
		sleep(1);
	}
	while(finales!=4); // Hasta que se reciban 4 "Fin"
	printf("---------------------------\n");
	printf("Se encontraron %d primos\n",primos);
	printf("---------------------------\n");
	printf("Los números primos son:\n");
	nodos_lista(lista);
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
	// Crear un buzón o cola de mensajes
	msgqid=msgget(0x1234,0666|IPC_CREAT);

	//Creacion de procesos emisores 
	for(int i=0;i<4;i++)
	{
		params[i]=i;
		p=fork();
		if(p==0){
			emisor(&params[i]); // mandar a llamar la funcion en cada proceso hijo
		}
	}

	//creación de proceso receptor
	int cons=fork();
	if(cons==0){
		receptor();
	}
	
	// esperar a todos los procesos
	for(int i=0;i<5;i++)
	{
		ret=wait(NULL); 
	}

	//eliminar buzón o cola de mensajes
	msgctl(msgqid,IPC_RMID,NULL);
	
    gettimeofday(&ts, NULL);
    stop_ts = ts.tv_sec; //Tiempo final
    elapsedTime = stop_ts - start_ts;
    printf("Tiempo total, %lld segundos\n", elapsedTime);
     


}

