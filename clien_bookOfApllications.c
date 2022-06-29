#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <string.h>
#define MSG_SIZE 128

struct skarga {			//struktura ksiegi
	int size;
	char name[32];
	char post[MSG_SIZE];
} *request;


key_t key;		//klucz
int memory, sem;	//id dzielonej i semaforu
struct sembuf sb;	//struktura ustawiania semforu

int main(int argc, char *argv[])
{
	char buf[MSG_SIZE];
	int i, num = 0, max;
	if(argc != 3)
	{
		printf("./klient.c [nazwa pliku] [nazwa uzytkownika]\n");
		exit(1);
	}
	
	printf("Witaj w kliencie ksiegi skarg i wpisow!\n");
	
	if((key = ftok(argv[1], 1)) == -1)		//Wytworzenie klucza
	{
		perror("[Klient]: nie udalo sie stworzyc klucza...\n");
		exit(1);
	}
	
	if((memory = shmget(key, 0, 0)) == -1)		//Podlaczenie sie do pamieci dzielonej
	{
		perror("[Klient]: blad segmentu...\n");
		exit(1);
	}
	
	if((request = (struct skarga*)shmat(memory, (void *)0, 0)) == (struct skarga *)(-1)) //dolaczenie pamieci do struktury ksiegi
	{
		perror("[Klient]: shmat nie dziala...\n");
		exit(1);
	}
	
	if((sem = semget(key, 0, 0)) == -1)		//dolaczenie semafora
	{
		perror("[Klient]: blad semafora...\n");
                exit(1);
	}
	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = 0;
	printf("[Klient]: oczekiwanie na dostep...\n");
	if(semop(sem, &sb, 1) == -1)			//zablokowanie semafora
	{
		perror("[Klient]: blad operacji na semaforze...");
		exit(1);
	}

	max = request[0].size;
	for(i = 0; i < max; i++)			//sprawdzenie ilosci zajetych wpisow
       	{
  		if(strcmp(request[i].name, "x") == 0) num++;
        }
	
	if(num == 0)					//Ksiega zajeta
	{
		printf("[Klient]: Ksiega jest zapelniona! Dowidzenia!\n");
		sb.sem_op = 1;
		if(semop(sem, &sb, 1) == -1)		//odblokowanie semafora
		{
			perror("[Klient]: blad operacji na semaforze...");
			exit(1);
		}
		exit(1);
	} else {					//Wolne miejsce w ksiedze
		printf("[Wolnych wpisów jest  %d na %d]\n", num, request[0].size);
		printf("[Klient]: Wprowadz wpis: \n> ");	//wpisanie wpisu do ksiegi
		fgets(buf, MSG_SIZE, stdin);
		strcpy(request[max-num].name, argv[2]);
		strcpy(request[max-num].post, buf);
		printf("[Klient]: Wpisano do księgi. Dowidzenia!\n");
		shmdt(request);
		sb.sem_op = 1;
		if(semop(sem, &sb, 1) == -1)		//odblokowanie semafora
		{
			perror("[Klient]: blad operacji na semaforze...");
			exit(1);
		}
		exit(0);
	}
}
