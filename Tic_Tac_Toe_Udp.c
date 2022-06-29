#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>

#define PORT "7777"			//Port
#define MAX 1024

struct invite{			//struktura do wyslania nicku oraz potwierdzenia polaczenia
	char nick[MAX];
	char stat[MAX];
}partner_invite;

int sfd[2];			//deskryptory socketow
struct addrinfo hints_my, hints_partner, *servinfo, *clientinfo, *iterator, *iterator2;			//potrzebene struktury do obsługi getaddrinof
char my_nick[MAX], partner_nick[MAX];			//nicki jednego i drugiego gracza
char t[9];			//tablica do gry
int turn, loopend, result, remis;			//zmienne potrzben do zlicania czegoś lub obsługi błędów
int wynik[2];

void fail_exit(char *);			//funkcja obsługi błędów
void set_server(char *);		// funkcja ustawien socketow i adresow
int con(int, int);			//funkcja polaczeniowa miedzy
void wypisztablice();			//funkcja do planszy
int zmientablice(char, char);			//funkcja wprowadzjaca zmiany w tablicy
void gra(int);			//funkcja zawierajaca całą gre
void winif(char, char *);			//funkcja do sprawdzania gry
void wypelnijtablice();			//funkcja wypełnaijaca tablice

int main(int argc, char *argv[])
{
	if(argc == 3)			//Nick albo domyslna nazwa
	{
		strcpy(my_nick, argv[2]);
	} else if(argc == 2){
		strcpy(my_nick, "NN");
	} else if(argc > 3 || argc < 2){
		printf("Usage: ./gra hostname nick\n");
	}

	set_server(argv[1]);			//Ustawienie adresow
	while (1)			//petla do rozpoczynania kolejnych rozgrywek
	{
		wypelnijtablice();
		printf("Rozpoczynam gre z %s. Napisz <koniec> by zakonczyc.\n", inet_ntoa(*(struct in_addr*)iterator2->ai_addr));
		turn = con(sfd[0], sfd[1]);			//polaczenie pomiedzy partnerami
		remis = 0;
		wynik[1] = 0;
		wynik[0] = 0;
		while(loopend != 1)			//petla rozgrywki
		{
			gra(turn);
			if(remis == 9)			//sama w sobie gra
			{
				wypisztablice();
				printf("[Remis, gramy dalej!]\n");
				wypelnijtablice();
				remis = 0;
			}
		}
	}
	return 0;
}

void set_server(char *addr)			//ustawienia połączenia
{

	memset(&hints_my, 0, sizeof(hints_my));			//zerujemy
	memset(&hints_partner, 0, sizeof(hints_partner));
	hints_my.ai_family = AF_INET;			//ustawiamy podpowiedzi dla getaddrinfo dla tego programu
	hints_my.ai_socktype = SOCK_DGRAM;
	hints_my.ai_flags = AI_PASSIVE;
	hints_partner.ai_family = AF_INET;			//dla partnera
	hints_partner.ai_socktype = SOCK_DGRAM;
	if((result = getaddrinfo(NULL, PORT, &hints_my, &servinfo)) != 0)			//Ustawiamy automatycznie za pomocą getaddrinfo dla tego programu
	{
		fail_exit("trouble with: getaddrinfo...");
	}
	if((result = getaddrinfo(addr, PORT, &hints_partner, &clientinfo)) != 0)			// i partnera
	{
		fail_exit("trouble with: getaddrinfo...");
	}
	for(iterator = servinfo; iterator != NULL; iterator = iterator->ai_next)			//ten program
	{
		if((sfd[0] = socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol)) == -1)
		{
			perror("listner: socket");
			continue;
		}
		if(bind(sfd[0], iterator->ai_addr, iterator->ai_addrlen) == -1)
		{
			perror("listner: bind");
			continue;
		}
		break;
	}
	if(iterator == NULL)
	{
		fail_exit("listner: faild to bind socket");
	}
	for(iterator2 = clientinfo; iterator2 != NULL; iterator2 = iterator2->ai_next)			//partner
	{
		if((sfd[1] = socket(iterator2->ai_family, iterator2->ai_socktype, iterator2->ai_protocol)) == -1)
		{
			perror("listner: socket");
			continue;
		}
		break;
	}
	if(iterator2 == NULL)
	{
		fail_exit("listner: faild socket");
	}
	freeaddrinfo(servinfo);			//czyscimy
	freeaddrinfo(clientinfo);
}

int con(int sock1, int sock2)			//polaczenie miedzy graczami
{
	strcpy(partner_invite.nick, my_nick);
	strcpy(partner_invite.stat, "join");
	if((sendto(sock2, &partner_invite, sizeof(struct invite), 0, iterator2->ai_addr, iterator2->ai_addrlen)) == -1)			//wyslanie nicku przez partnera
  {
  	fail_exit("trouble with: sendto");
	}
	printf("[Propozycja gry wysłana]\n");
	if((recvfrom(sock1, &partner_invite, sizeof(struct invite), 0, NULL, NULL) == -1))			//pobranie nicku
  {
    fail_exit("trouble with: recvfrom");
	}
	strcpy(partner_nick,partner_invite.nick);
	if(strcmp("join", partner_invite.stat)==0)			//warunke na rozdzielenie kolejnosci
	{
			printf("[%s (%s) dołączył do gry]\n", partner_nick, inet_ntoa(*(struct in_addr*)iterator2->ai_addr));
			strcpy(partner_invite.nick, my_nick);
			strcpy(partner_invite.stat, "got");
			if((sendto(sock2, &partner_invite, sizeof(struct invite), 0,  iterator2->ai_addr, iterator2->ai_addrlen) == -1))	//wyslanie nicku przez ten porgram
		  {
		  	fail_exit("trouble with: sendto");
			}
			return 1;
	 } else if(strcmp("got", partner_invite.stat) == 0){
		 	return 2;
	 }
	 return 0;
}

void wypisztablice(){
	printf("\n%c|%c|%c\n%c|%c|%c\n%c|%c|%c\n\n",t[0],t[1],t[2],t[3],t[4],t[5],t[6],t[7],t[8]);
}

int zmientablice(char position, char k){			//zmienianie wartosci pól
	switch(position){
		case 'a':
		{
			if(t[0]!='a') return -2;
			t[0]=k;
      break;
		}
    case 'b':
		{
      if(t[1]!='b') return -2;
			t[1]=k;
    	break;
		}
    case 'c':
		{
      if(t[2]!='c') return -2;
			t[2]=k;
      break;
		}
    case 'd':
		{
      if(t[3]!='d') return -2;
			t[3]=k;
    	break;
		}
    case 'e':
		{
    	if(t[4]!='e') return -2;
			t[4]=k;
      break;
		}
    case 'f':
		{
			if(t[5]!='f') return -2;
			t[5]=k;
      break;
		}
    case 'g':
		{
      if(t[6]!='g') return -2;
			t[6]=k;
      break;
		}
    case 'h':
    {
			if(t[7]!='h') return -2;
			t[7]=k;
      break;
		}
    case 'i':
		{
      if(t[8]!='i') return -2;
			t[8]=k;
      break;
		}
    default:
		{
    	return -2;
		}
	}
	return 0;
}

void gra(int who)			//całość gry
{
	char buff[MAX], choice;
	bzero(buff, MAX);
	int check;
	if(who == 1)		//zawodnik z kolejnoscia 1
	{
		wypisztablice();			//wypisanie i porbeanie
		printf("[Wybierz pole] ");
		scanf("%s", buff);
		if(strcmp(buff, "<koniec>") == 0)			//sprawdzenie
		{
			if((sendto(sfd[1], buff, sizeof(buff), 0,  iterator2->ai_addr, iterator2->ai_addrlen)) == -1)
		  {
		  	fail_exit("trouble with: sendto");
			}
			exit(0);
		} else {
			if(strcmp(buff, "<wynik>") == 0)			//sprawdzenie
			{
				printf("%s %d : %d %s\n", my_nick, wynik[0], wynik[1], partner_nick);
				scanf("%s", buff);
			}
			sscanf(buff, "%c", &choice);
			check = zmientablice(choice, 'X');			//zamiana
			while(check == -2)			//warunek na dostepne pole
			{
				printf("[tego pola nie mozesz wybrac, wybierz pole] ");
				scanf("%s", buff);
				if(strcmp(buff, "<koniec>") == 0)
				{
					if((sendto(sfd[1], buff, sizeof(buff), 0,  iterator2->ai_addr, iterator2->ai_addrlen)) == -1)
				  {
				  	fail_exit("trouble with: sendto");
					}
					exit(0);
				}
				if(strcmp(buff, "<wynik>") == 0)			//sprawdzenie
				{
					printf("%s %d : %d %s\n", my_nick, wynik[0], wynik[1], partner_nick);
					scanf("%s", buff);
				}
				sscanf(buff, "%c", &choice);
				check = zmientablice(choice, 'X');
			}
			if((sendto(sfd[1], buff, sizeof(buff), 0,  iterator2->ai_addr, iterator2->ai_addrlen)) == -1) 			//przekazanie zmiany
		  {
		  	fail_exit("trouble with: sendto");
			}
			winif('X', "[Wygrana! Kolejna rozgrywka, poczekaj na swoja kolej]");
			remis++;
			turn = 2;
		}
	} else if(who == 2){		//zawodnik z kolejnoscia 2
		if((recvfrom(sfd[0], buff, sizeof(buff), 0, NULL, NULL) == -1))			//odebranie zmiany
	  {
	    fail_exit("trouble with: recvfrom");
		}
		if(strcmp(buff, "<koniec>") == 0)
		{
				printf("[%s (%s) zakończył rozgrywke, możesz poczekac na następnego gracza]\n", partner_nick, inet_ntoa(*(struct in_addr*)iterator2->ai_addr));
				wypelnijtablice();
				loopend = 1;
		} else {			//zmiana u partnera
			printf("[%s (%s) wybrał pole %s]\n", partner_nick, inet_ntoa(*(struct in_addr*)iterator2->ai_addr), buff);
			sscanf(buff, "%c", &choice);
			zmientablice(choice, 'O');
			winif('O', "[Przegrana! Zagraj jeszcze raz]");
			turn = 1;
			remis++;
		}
	}

}

void winif(char sign, char *role)			//warunek wygrania
{
	int i;
	for(i = 0; i < 3; i++)
        {
            if ((t[i] == t[i+3]) && (t[i+3] == t[i+6]) && (t[i]==sign))
            {
                printf("%s\n", role);
								wypelnijtablice();
								remis = 0;
								if(sign == 'X')
								{
									wynik[0]++;
								} else if(sign == 'O')
								{
									wynik[1]++;
								}
            }

            if ((t[i*3] == t[i*3+1]) && (t[i*3+1] == t[i*3+2]) && (t[i*3] == sign))

            {
                printf("%s\n",role);
								wypelnijtablice();
								remis = 0;
								if(sign == 'X')
								{
									wynik[0]++;
								} else if(sign == 'O')
								{
									wynik[1]++;
								}
            }
        }
        if (((t[0] == t[4] && t[4] == t[8]) || (t[2] == t[4] && t[4] == t[6])) && t[4] == sign)
        {
            printf("%s\n", role);
						wypelnijtablice();
						remis = 0;
						if(sign == 'X')
						{
							wynik[0]++;
						} else if(sign == 'O')
						{
							wynik[1]++;
						}
        }
}

void wypelnijtablice(){
	t[0]='a';
	t[1]='b';
	t[2]='c';
	t[3]='d';
	t[4]='e';
	t[5]='f';
	t[6]='g';
	t[7]='h';
	t[8]='i';
}


void fail_exit(char *er)			//obsługa błędów
{
	perror(er);
	exit(1);
}
