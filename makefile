all: P6_SEM

P6_SEM: P6_SEM.c
	gcc -o P6_SEM P6_SEM.c -lm
	