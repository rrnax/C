#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#define MSG_SIZE 128

union semun {			//unia do semfaora
	int val;
	struct semid_ds *buf_sem;
	ushort *array;
} sem_arg;

struct skarga {			//struktura dotyczaca wpsiu
	int size;		//size zawier maksymalna ilosc wpsiow
	char name[32];
	char post[MSG_SIZE];
} *request;

key_t key;			
int memory, sem;		//id dzielonej i semaforu
struct shmid_ds info;
int max;			//wielkośc ksiegi
struct sembuf sb;		//struktura do zarzadania semaforem

static void sig_handler(int);	//Obsługa sygnałów

int main(int argc, char *argv[])
{
	int j;
	max = atoi(argv[2]);
	
	if(argc != 3)		
	{
		printf("./serwer.c [nazwa pliku] [rozmiar ksiegi]\n");
		exit(1);
	}
	
	printf("[Serwer]: ksiega skarg i wnioskow (WARIANT B)\n");
	printf("[Serwer]: tworze klucz na podstawie pliku %s... ", argv[1]);
	
	if((key = ftok(argv[1], 1)) == -1)		//wytworzenie klucza
	{
		perror("[Serwer]: nie udalo sie stworzyc klucza...\n");
		exit(1);
	}
	
	printf("OK (klucz: %d)\n", key);
	printf("[Serwer]: tworze segment pamieci dzielonej dla ksiegi na %d wpsiow po 128b każdy... ", atoi(argv[2]));

	if((memory = shmget(key, atoi(argv[2]) * sizeof(struct skarga), 0666 | IPC_CREAT | IPC_EXCL)) == -1)	//wytworzenie segmentu pamieci
	{
		perror("[Serwer]: blad segmentu...\n");
		exit(1);
	}
	
	shmctl(memory, IPC_STAT, &info);	//kontorola pamieci dzielonej 
	printf("OK (id: %d, rozmiar: %zub)\n", memory, info.shm_segsz);
	printf("[Serwer]: dolaczam pamiec wspolna... ");
	
	if((request = (struct skarga*)shmat(memory, (void *)0, 0)) == (struct skarga *)(-1))	//dolaczenie pamieci do struktury ksiegi
	{
		perror("[Serwer]: shmat nie dziala...\n");
		exit(1);
	}
	
	for(j = 0; j < atoi(argv[2]); j++)	//wypelnienie w celu ustalania ilosci wpisow
	{
		strcpy(request[j].name, "x");
		request[j].size = max;
	}
	printf("OK (adres: %lX)\n", (long int)request);

	printf("[Serwer]: Tworzenie semafora...");
	if((sem = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) == -1)    //wytworzenie semafora
        {
                perror("[Serwer]: blad semafora...\n");
                exit(1);
        }
	printf(" stworzony id: %d\nustawianie wartosci...", sem);
	sem_arg.val = 1;
	if(semctl(sem, 0, SETVAL, sem_arg) == -1)		//ustawienie wartosci poczatkowej semafora
	{
		perror("[Serwer]: blad semctl...\n");
		exit(1);
	}
	printf(" ustawiono\n");
	sb.sem_num = 0;
	sb.sem_op =  1;		//ustawiam na odblokowany
	sb.sem_flg = 0;


	if (signal(SIGTSTP, sig_handler) == SIG_ERR)	perror("cos nie wyszlo");	//sygnaly
	if (signal(SIGINT, sig_handler) == SIG_ERR)	perror("cos nie wyszlo");

	while(1)
	{
		printf("[Serwer]: Ctrl^Z -> stan ksiegi\n");
       		printf("[Serwer]: Ctrl^C -> zakoncz program\n");
		pause();
	}
	
}

static void sig_handler(int signum)
{
	int i, num = 0;
	if(signum == SIGTSTP)		//wypisanie zawartosic ksiegi
	{	
		printf("\n[Serwer]: oczekiwanie na dostep...\n");
		sb.sem_op = -1;		//blokuje
		if(semop(sem, &sb, 1) == -1)
		{
			perror("[Serwer]: blad operacji na semaforze...");
			exit(1);
		}


		printf("\n*************** Ksiega skarg i wnioskow ***************\n");
		for(i = 0; i < max; i++)
		{
			if(strcmp(request[i].name, "x") != 0) num++;
		}
		if(num == 0)
		{
			printf("PUSTO\n");
		} else {
			for(i = 0; i < num; i++)
			{
				printf("[%s]: %s\n", request[i].name, request[i].post);
			}
		}

		sb.sem_op = 1;		//odblokowuje
		if(semop(sem, &sb, 1) == -1)	
                {
                        perror("[Serwer]: blad operacji na semaforze...");
                        exit(1);
                }
		

	} else if(signum == SIGINT) {		//zakonczenie i odlaczenie pamieci
		printf("\n[Serwer]: dostalem polecenie zkonczenia... \n");
		printf("odlaczanie pamieci dzielonej... %s\n", (shmdt(request) == 0) ? "zakonczone" : "blad odlaczania!");
		printf("usuwanie pamieci dzielonej... %s\n", (shmctl(memory, IPC_RMID, 0) == 0) ? "zakonczone" : "blda usuwania pamieci!");
		printf("usuwanie semafora... %s\n", (semctl(sem, 0, IPC_RMID, sem_arg) == 0) ? "zakonczone" : "blda usuwania semafora!");
		exit(1);
	}
}


