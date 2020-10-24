all: P6_SEM P6_SMS

P6_SEM: P6_SEM.c
	gcc -o P6_SEM P6_SEM.c -lm

P6_SMS: P6_SMS.c
	gcc -o P6_SMS P6_SMS.c -lm

