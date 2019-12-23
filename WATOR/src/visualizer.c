/** \file visualizer.c
    \author Leonardo Cariaggi (503140)
    
     Si dichiara che il contenuto di questo file e' in ogni sua parte opera
     originale dell' autore.  
*/

#define _GNU_SOURCE
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/select.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/wait.h>
#include "wator.h"
#define SYSCALL(r,c,e) if((r=c)==-1) { perror(e);exit(errno); }
#define UNIX_PATH_MAX 108
#define SOCKNAME "./visual.sck"
#define ERRORMESSAGE(x) { printf(x);exit(1); }
#define N 100

/** variabili globali
 */
planet_t *plan;
FILE *dump;

/** funzione di test eseguita al momento dell'uscita
 */
void visualizerExit(void);

int main(int argc, char *argv[])
{
	atexit(visualizerExit);
	unlink(SOCKNAME);
	int sfd, fd_c, x, r, ultimaIterazione=0;
	char bufrighe[256], bufcolonne[256], *nonIntegerPart;	
	
	/** Se il file passato come parametro ha nome "stdout", allora le stampe di visualizer saranno effettuate sullo stdout.
		Questo causa un'unica restrizione sul nome dei file su cui l'utente desidera stampare, ossia che differisca da "stdout".	
	 */
	if(strcmp(argv[1], "stdout") == 0)
		dump = stdout;
	
	sigset_t set;
	
	SYSCALL(r,sigemptyset(&set),"sigemptyset");
	SYSCALL(r,sigaddset(&set, SIGINT),"sigaddset"); //NON VOGLIO RICEVERE SIGINT
	SYSCALL(r,pthread_sigmask(SIG_SETMASK, &set, NULL),"sigmask");
	
	struct sockaddr_un sa;
	strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
	sa.sun_family = AF_UNIX;

	SYSCALL(sfd,socket(AF_UNIX, SOCK_STREAM, 0),"creazione socket");
	SYSCALL(x,bind(sfd, (struct sockaddr *)&sa, sizeof(sa)),"bind");
	SYSCALL(x,listen(sfd, SOMAXCONN),"listen");
	
	/************************************************************************
	**********LIFECYCLE DEL VISUALIZER**************************************/
	while(1)
	{			
		SYSCALL(fd_c,accept(sfd, NULL, 0),"accept");
	
		SYSCALL(r,read(fd_c, bufrighe, 256),"read"); //LETTURA NUMERO RIGHE
		int righe = strtol(bufrighe, &nonIntegerPart, 10);
		if(*nonIntegerPart == 's') {//SUPPONGO DI POTER INVIARE SOLO [numero] OPPURE [numero + s] (CONDIZIONE DI TERMINAZIONE)
			ultimaIterazione = 1;
			//printf("[**visualizer**] stampo per l'ultima volta\n");
		}
		//else printf("[**visualizer**] stampo ma non è l'ultima iterazione\n");
		SYSCALL(r,read(fd_c, bufcolonne, 256),"read"); //LETTURA NUMERO COLONNE
		int colonne = strtol(bufcolonne, &nonIntegerPart, 10);	
	
		plan = new_planet(righe, colonne);
		if(plan == NULL) exit(1);

		for (int i = 0; i < righe; i++) //LETTURA RIGA PER RIGA DALLA SOCKET
		{
			SYSCALL(r,read(fd_c, plan->w[i], colonne * sizeof(cell_t)),"read");
		}
	
		SYSCALL(r,close(fd_c),"[visualizer] close della socket");
		
		if(strcmp(argv[1], "stdout") != 0)
		{
			dump = fopen(argv[1], "w");
			print_planet(dump, plan);
			fclose(dump);
		}
		
		else print_planet(dump, plan);
	
		
		if(ultimaIterazione == 1)
			break;
	}
	/*********FINE LIFECYCLE DEL VISUALIZER**********************************
	************************************************************************/	
	unlink(SOCKNAME);
	return 0;
}

void visualizerExit()
{
	write(1, "[VIS] esco\n", 11);
}


