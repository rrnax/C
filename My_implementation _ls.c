#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <limits.h>
#define BUFS 1024


typedef struct data{		//Struktura która przechowuje nam wszystkie wydobyte i prztworzone informacje
	char name[256];
	char mod_data[64];	//Data ostatniej modyfikacji
	int file_size;
	long unsigned int hard_links;
	int uid;
	int gid;
	char user_name[64];
	char group_name[64];
	char file_type_premision[32];
}data;

void print_2_lines(char *file_name)		//Funkcja wypisujaca poczatek zawartosci pliku
{
	FILE *ft;	//Wskaznik na plik
	char buf[BUFS];
	ft = fopen (file_name, "r");
   	if (ft == NULL)
	{	
		perror ("Error opening file");
	} else {
		printf("Poczatek zawartosci: \n");
		if(fgets (buf , BUFS , ft) != NULL )
       		{
			printf("%s", buf);	//Wypisanie pierwszej linijki
		}
		if(fgets (buf , BUFS , ft) != NULL )
                {	
                        printf("%s", buf);	//Wypisanie drugiej linijki
                }
	}
}

void user_and_group(int u, int g, char *u_name, char *g_name)		//Zamiana z uid i gid na nazwy grupy i uzytkownika
{
	struct passwd *usinfo;
	struct group *grinfo;
	if((usinfo = getpwuid(u)) == NULL)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
	if((grinfo = getgrgid(g)) == NULL)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
	strcpy(u_name, usinfo->pw_name);	//Skopiowanie do stuktury nazwy uzytkownika i grupy;
	strcpy(g_name, grinfo->gr_name);	
}	

int hard_link(char *file_name)		//Funkcja wydobywajaca ilosc twardych linkow
{
	struct stat st_pl;
        if(lstat(file_name, &st_pl) == -1)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
	return st_pl.st_nlink;
}

int uid_u(char *file_name)          //Funkcja wydobywajaca uid
{
        struct stat st_pl;
        if(lstat(file_name, &st_pl) == -1)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
        return st_pl.st_uid;
}

int gid_g(char *file_name)          //Funkcja wydobywajaca gid
{
        struct stat st_pl;
        if(lstat(file_name, &st_pl) == -1)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
        return st_pl.st_gid;
}



void filesize(char *file_name, int *result)		//Funkcja wydobywajaca ze stat wielkosc
{
	struct stat st_pl;
	if(lstat(file_name, &st_pl) == -1)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
	*result = st_pl.st_size;
}

void modify_time(char *file_name, char *result)		//Funkcja odczytujaca data ostatniej modyfikacj
{
	struct stat st_pl;
	time_t now;		//Czas terazniejszy
	struct tm modtime, actual;	// modtiem(ostatnia modyfikacja) actual(aktualna data)
	char buf[64];
	time(&now);
	actual = *localtime(&now);
	if(lstat(file_name, &st_pl) == -1)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
	modtime = *localtime(&st_pl.st_mtime);
	if(actual.tm_year == modtime.tm_year)	//Warunek od ktorego zalezy format wypisywanej daty w trybie pierwszym
	{
		strftime(buf, sizeof(buf), "%m-%d %H:%M", &modtime );
		strcpy(result, buf);
	} else {
		strftime(buf, sizeof(buf), "%Y-%m-%d", &modtime );
		strcpy(result, buf);
	}
}

void print_time(struct tm some_time)		// Odpowiednie formatowanie wypisu dat do trybu drugiego
{
	char buf[64];
	switch(some_time.tm_mon)
	{
		case 0: strftime(buf, sizeof(buf), "%d styczenia %Y roku o %X", &some_time); break;
		case 1: strftime(buf, sizeof(buf), "%d lutego %Y roku o %X", &some_time); break;
		case 2: strftime(buf, sizeof(buf), "%d marca %Y roku o %X", &some_time); break;
		case 3: strftime(buf, sizeof(buf), "%d kwietnia %Y roku o %X", &some_time); break;
		case 4: strftime(buf, sizeof(buf), "%d maja %Y roku o %X", &some_time); break;
		case 5: strftime(buf, sizeof(buf), "%d czerwca %Y roku o %X", &some_time); break;
		case 6: strftime(buf, sizeof(buf), "%d lipca %Y roku o %X", &some_time); break;
		case 7: strftime(buf, sizeof(buf), "%d sierpnia %Y roku o %X", &some_time); break;
		case 8: strftime(buf, sizeof(buf), "%d wrzesnia %Y roku o %X", &some_time); break;
		case 9: strftime(buf, sizeof(buf), "%d pazdziernika %Y roku o %X", &some_time); break;
		case 10: strftime(buf, sizeof(buf), "%d listopada %Y roku o %X", &some_time); break;
		case 11: strftime(buf, sizeof(buf), "%d grudnia %Y roku o %X", &some_time); break;
	}
	printf(" %s\n", buf);
}

void file_time(char *file_name)         //Funkcja odczytujaca i wypisujaca czasy pliku
{
        struct stat st_pl;
        struct tm modtime, acstime, chntime; //modtime = modyfikacji, asctime = dostepu, chntime = zmiany	(czasy)
        if(lstat(file_name, &st_pl) == -1)
        {
                fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
        modtime = *localtime(&st_pl.st_mtime);
        acstime = *localtime(&st_pl.st_atime);
     	chntime = *localtime(&st_pl.st_ctime);	
   	printf("\nOstatnio używany:	 ");
	print_time(acstime);
        printf("Ostatnio modyfikowany:	 ");
        print_time(modtime);
        printf("Ostatnio zmieniany stan: ");
        print_time(chntime);
}


void fileandpermision(char *file_name, char *fandp)		//Funkcja odczytujaca typ i prawa pliku
{
	struct stat st_pl;
	if(lstat(file_name, &st_pl) == -1)
	{
		fprintf(stderr, "stat: errno: %d : %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	        switch (st_pl.st_mode & S_IFMT)		//Rodzaj pliku
	{
       		case S_IFDIR:  fandp[0] = 'd';     break;
        	case S_IFLNK:  fandp[0] = 'l';     break;
        	case S_IFREG:  fandp[0] = '-';     break;
        	default:       fandp[0] = 'o';     break;		//Przez o oznaczylem wszystkie inny pliki  
        }
	if (st_pl.st_mode & S_IRUSR)		//Prawa wlasciciela
	{
		fandp[1] = 'r';
	} else {
		fandp[1] = '-';
	}

	if (st_pl.st_mode & S_IWUSR)
        {
                fandp[2] = 'w';
        } else {
                fandp[2] = '-';
        }

	if (st_pl.st_mode & S_IXUSR)
        {
                fandp[3] = 'x';
        } else {
                fandp[3] = '-';
        }
	
        if (st_pl.st_mode & S_IRGRP)		//Prawa grupy
        {
                fandp[4] = 'r';
        } else {
                fandp[4] = '-';
        }

        if (st_pl.st_mode & S_IWGRP)
        {
                fandp[5] = 'w';
        } else {
                fandp[5] = '-';
        }

        if (st_pl.st_mode & S_IXGRP)
        {
                fandp[6] = 'x';
        } else {
                fandp[6] = '-';
       	}

        if (st_pl.st_mode & S_IROTH)		//Prawa innych
        {
                fandp[7] = 'r';
        } else {
                fandp[7] = '-';
        }

        if (st_pl.st_mode & S_IWOTH)
        {
                fandp[8] = 'w';
        } else {
                fandp[8] = '-';
        }

        if (st_pl.st_mode & S_IXOTH)
        {
                fandp[9] = 'x';
        } else {
                fandp[9] = '-';
        }
/*	 if (st_pl.st_mode & S_ISVTX)		Opcjonalny sticky bit
        {
                fandp[10] = 't';
        } else {
                fandp[10] = 'T';
        }		*/

}

void column_size(data *all, int all_size, int *max)		// Funkcja okreslajaca nadluszsza liczbe znakow w kolumnie (odpowiada za szerokosc kolumn)
{
	int c;
	char buf[128], buf2[128];
	sprintf(buf, "%d", all[0].file_size);
	sprintf(buf2, "%ld", all[0].hard_links);
	max[0] = strlen(all[0].user_name);	
	max[1] = strlen(all[0].group_name);
	max[2] = strlen(buf);			
	max[3] = strlen(buf2);
	max[4] = strlen(all[0].mod_data);
	for(c = 1; c < all_size; c++)
	{
		sprintf(buf, "%d", all[c].file_size);
		sprintf(buf2, "%ld", all[c].hard_links);
		if(max[0] < strlen(all[c].user_name))	//kolumna nazw uzytkownikow
		{
			max[0] = strlen(all[c].user_name);
		}
		if(max[1] < strlen(all[c].group_name))	//kolumna nazw grup
                {
                        max[1] = strlen(all[c].group_name);
                }
		if(max[2] < strlen(buf))		//kolumna rozmiaru pliku
                {
                        max[2] = strlen(buf);
                }
		if(max[3] < strlen(buf2))		//kolumna liczby dowiazan
                {
                        max[3] = strlen(buf2);
                }
		if(max[4] < strlen(all[c].mod_data))	//kolumna daty ostatniej modyfikacji
                {
                        max[4] = strlen(all[c].mod_data);
                }
	}
}

void print_column(int c_size, int cell)		//funkcja uzupelniajaca szerokosc kolumn c_size =  maksymalna szerokosc kolumny , cell = szerokosc pojedynczej komorki
{
	int p;
	for(p = 0; p < c_size - cell; p++)
	{
		printf(" ");
	}	
}


int main(int argc, char *argv[])
{
	struct dirent **content;	//content przetrzymuje przez chwile nazwy plikow w katalogu
	int i, j, k, max_c[5];		//j, k, uzywane do petli for, max_c przetrzymuje maksymalnom szerokosc danej kolumny 
	data *info;			//struktura przetrzymujaca fszystkie info o pliku
	char buf_path[PATH_MAX], buf_path_link[PATH_MAX], *rep, empty[128], empty2[128]; //pomocnicze bufory
	if(argc == 1)		// Tryb pierwszy
	{
		i = scandir(".", &content, NULL, alphasort);		//Zmienna  [i] odpowiadajaca ilosci plikow w przeszukiwanym miejscu
		if (i == -1) 
		{
               		perror("scandir");
            		exit(EXIT_FAILURE);
           	}
		info =(data*) malloc((i+3) * sizeof(data));
		for(j = 0; j < i; j++){
              		strcpy(info[j].name, content[j]->d_name);	// Przpisanie w posortowanej kolejnosc nazw plikow do sktruktury w ktorej groamdzimy informacje o plikach
               		free(content[j]);
           	}
		free(content);
		for(j = 0; j < i; ++j)			//Przniesienie informacji do struktury za pomoca funkcji napisanych powyzej
		{
			filesize(info[j].name, &info[j].file_size);
			info[j].hard_links = hard_link(info[j].name);
			info[j].uid = uid_u(info[j].name);
			info[j].gid = gid_g(info[j].name);
			fileandpermision(info[j].name, info[j].file_type_premision);
			modify_time(info[j].name, info[j].mod_data);
			user_and_group(info[j].uid, info[j].gid, info[j].user_name, info[j].group_name);
		}
		column_size(info, i, max_c);	// Okreslenie maksymalnych szerokosci kolumn
		for(j = 0; j < i; ++j)		//Wypisanie informacji o plikach
		{
			for(k = 0; k < 10; k++)
                        {
                                printf("%c", info[j].file_type_premision[k]);
                        }
			print_column(max_c[3], sprintf(empty2,"%ld",info[j].hard_links));
                        printf(" %ld ", info[j].hard_links);
			print_column(max_c[0], strlen(info[j].user_name));
			printf("%s ", info[j].user_name);
			print_column(max_c[1], strlen(info[j].group_name));
			printf(" %s ", info[j].group_name);
			print_column(max_c[2], sprintf(empty,"%d",info[j].file_size));
			printf("%d", info[j].file_size);
			print_column(max_c[4], strlen(info[j].mod_data));
			printf(" %s", info[j].mod_data);
			printf(" %s", info[j].name);
                        if(info[j].file_type_premision[0] == 'l')
                        {
                                rep = realpath(info[j].name, buf_path);
                                if (rep == NULL)
                                {
                                         perror("realpath");
                                         exit(EXIT_FAILURE);
                                } else {
                                        printf(" -> %s", buf_path);
                                }
                        }
                        printf("\n");
		}
		free(info);
	} else if(argc == 2) {		// Tryb drugi
		info =(data*) malloc(sizeof(data));
		strcpy(info->name, argv[1]);	
		rep = realpath(info->name, buf_path);
                if (rep == NULL)
                {
                        perror("realpath");
                        exit(EXIT_FAILURE);
                }
		fileandpermision(info->name, info->file_type_premision);		//Prawa i typ pilku za pomoca funkcji
		filesize(info->name, &info->file_size);			//Wielkosc pliku
		if((info->file_type_premision[0]) == '-')		//Zwykly plik wypisanie o nim informacji
		{
			printf("Informacje o %s:\nTyp pliku: \tzwykly plik\nSciezka: \t%s\nRozmiar: \t%d bajtow\nUprawnienia: \t", info->name, buf_path, info->file_size);
			for(j = 1; j <= 9; j++)
			{
				printf("%c",info->file_type_premision[j]);
			}
			file_time(info->name);				//wypis czasow pliku za pomoca funkcji
			print_2_lines(info->name);			//Wypis zawarosci z apomoca funkcji
		}
		if((info->file_type_premision[0]) == 'd')				// Katalog wypisanie o nim informacji
                {
                	printf("Informacje o %s:\nTyp pliku: \tkatalog\nSciezka: \t%s\nRozmiar: \t%d bajtow\nUprawnienia: \t", info->name,buf_path, info->file_size);
                        for(j = 1; j <= 9; j++)
                        {
                                printf("%c",info->file_type_premision[j]);
                        }
                        file_time(info->name);				//wypis czasow pliku za pomoca funkcji
		}
		if((info->file_type_premision[0]) == 'l')			//Link symboliczny wypisanie o nim informacji
                {
			getcwd(buf_path_link, PATH_MAX);
                        printf("Informacje o %s:\nTyp pliku: \tlink symboliczny\nSciezka: \t%s/%s\nWskazuje na: \t%s\nRozmiar: \t%d bajtow\nUprawnienia: \t", info->name, buf_path_link, info->name, buf_path, info->file_size);
                        for(j = 1; j <= 9; j++)
                        {
                                printf("%c",info->file_type_premision[j]);
                        }
                        file_time(info->name);				//wypis czasow pliku za pomoca funkcji
                }
		if((info->file_type_premision[0]) == 'o')		//Inny pliki wypisanie o nim informacji
                {
                	printf("Informacje o %s:\nTyp pliku: \tinny plik\nSciezka: \t%s\nRozmiar: \t%d bajtow\nUprawnienia: \t", info->name, buf_path, info->file_size);
                        for(j = 1; j <= 9; j++)
                        {
                                printf("%c",info->file_type_premision[j]);
                        }
                        file_time(info->name);			//wypis czasow pliku za pomoca funkcji

		}

	} else {		//Obsluga bledu za duzej ilosci parametrow
		fprintf(stderr, "Too many arguments.\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
