/** \file Wator.c
    \author Leonardo Cariaggi (503140)
    
     Si dichiara che il contenuto di questo file e' in ogni sua parte opera
     originale dell' autore.  
*/

#define _GNU_SOURCE
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "wator.h"
#include "myFunctions.h"
#define ALT altezza
#define LAR larghezza
#define SYSCALL(r,c,e) if((r=c)==-1) { perror(e);exit(errno); }
#define UNIX_PATH_MAX 108
#define SOCKNAME "./visual.sck"
#define ERRORMESSAGE(x) { printf(x);exit(1); }
#define RETURN(e, x); errno=e; return x;
#define N 256

/** controlla se un certo file esiste nella cartella corrente ed è un regular file
   \param filename nome del file da controllare

   \retval 1 se il file esiste
   \retval 0 se il file non esiste oppure non è un regular file
 */
int doesFileExist(const char *filename);

/** applica le regole a una specifica fetta di pianeta
   \param pw struttura la cui fetta di pianeta deve essere processata
   \param sl fetta in cui saranno applicate le regole

   \retval -1 se si è verificato un errore
   \retval 0 se tutto è andato bene
   
   \comment all'interno della funzione si fa uso delle variabili globali borderMutex e oldMatrix
 */
int ApplyRules(wator_t *pw, Slice sl);

void* workerroutine(void *arg);
void* collectorroutine(void *arg);
void* dispatcherroutine(void *arg);

/** funzioni di cleanup eseguite al momento dell'uscita
 */
void cleanWorkersId(void);
void cleanWator(void);
void cleanList(void);

/** handler per la gestione dei segnali SIGUSR1, SIGTERM/SIGINT e SIGUSR2
 */
void usr1Handler(int sig);
void termHandler(int sig);
void usr2Handler(int sig);

/** variabili globali
 */
int *workersid, sliceCounter=0, workCounter=0, currentChronon=0, visualcycle=100, iterazioniCompiute=0, workDone=0, workCollected=0, workers=2, altezza, larghezza;
volatile sig_atomic_t checkpointFlag=0, termFlag=0, usr2Flag=0;
pthread_t dispatchertid;

pthread_mutex_t borderMutex = PTHREAD_MUTEX_INITIALIZER, //mutex usata dagli worker quando si trovano in prossimità del bordo della fetta su cui lavorano
queueMutex = PTHREAD_MUTEX_INITIALIZER, //mutex utilizzata per accedere alla lista globale delle fette di pianeta
readytosendMutex = PTHREAD_MUTEX_INITIALIZER, //mutex "di appoggio" alla variabile di condizione sulla quale collector si sospende
workcounterMutex = PTHREAD_MUTEX_INITIALIZER, //mutex usata dagli worker per incrementare la variabile che tiene traccia del numero di fette processate
readytoproduceMutex = PTHREAD_MUTEX_INITIALIZER; //mutex "di appoggio" alla variabile di condizione sulla quale dispatcher si sospende

pthread_cond_t emptyqueueCond = PTHREAD_COND_INITIALIZER, //CV su cui gli worker si sospendono quando non hanno la possibiltà di prelevare una fetta dalla lista
readytosendCond = PTHREAD_COND_INITIALIZER, //CV sulla quale collector si sospende in attesa che gli worker abbiano finito il lavoro
readytoproduceCond = PTHREAD_COND_INITIALIZER; //CV sulla quale dispatcher si sospende in attesa che collector abbia inviato il pianeta a visualizer

//lista delle fette che compongono il pianeta
SliceList l = NULL;

wator_t *pw;
cell_t **oldMatrix;

int watorPid, pid;
/*
void printlist(SliceList li)
{
	if(li == NULL) return;
	printf("fetta: (%d, %d) [%d x %d]\n", (li->slice).origineX, (li->slice).origineY, (li->slice).altezza, (li->slice).larghezza);
	printlist(li->next);
}
*/
int main(int argc, char *argv[])
{
	watorPid=getpid();
	
	char *options = "v:n:f:"; //OPZIONI ACCETTATE
	char currentoption, dumpfile[N] = "stdout", *strtolbuffer, fileplan[N];
	int gotv=0, gotf=0, gotn=0, err, r; 
	struct sigaction s;
	memset(&s, 0, sizeof(s));
	s.sa_handler = usr1Handler;
	SYSCALL(r,sigaction(SIGUSR1,&s,NULL),"sigaction1");// SETTO L'HANDLER PER IL CHECKPOINT
	
	sigset_t terminationset;
	SYSCALL(r,sigemptyset(&terminationset),"sigemptyset");
	SYSCALL(r,sigaddset(&terminationset, SIGINT),"sigaddset"); //MASCHERO SIGINT E SIGTERM DURANTE IL termHandler
	SYSCALL(r,sigaddset(&terminationset, SIGTERM),"sigaddset");
	s.sa_mask = terminationset;
	
	
	s.sa_handler = termHandler;
	SYSCALL(r,sigaction(SIGINT,&s,NULL),"sigaction2");
	SYSCALL(r,sigaction(SIGTERM,&s,NULL),"sigaction3");

	if(argc < 2)
		ERRORMESSAGE("Too few arguments\n");
	

	strcpy(fileplan, argv[1]);
	
	optind = 2; //AL PRIMO POSTO C'È SEMPRE IL FILE (optind è in unistd.h)
	//optarg CONTIENE IL VALORE DELLE OPZIONI
	
	if (!doesFileExist(fileplan)) //CONTROLLO IL PRIMO ARGOMENTO
		ERRORMESSAGE("File does not exist or it is not a regular file\n");
	
	while ((currentoption = getopt(argc, argv, options)) != -1)
	{	
		switch(currentoption)
		{
			case 'v':				
				visualcycle = strtol(optarg, &strtolbuffer, 10);
				if(gotv == 1 || *strtolbuffer != '\0' || visualcycle <=0)
					exit(1);
				gotv=1;
				break;
			case 'n':				
				workers = strtol(optarg, &strtolbuffer, 10);
				if(gotn == 1 || *strtolbuffer != '\0' || workers <=0)
					exit(1);
				gotn=1;
				break;
			case 'f':
				if(gotf == 1)
					exit(1);
				gotf=1;
				strcpy(dumpfile, optarg);
				break;
			default:
				printf("%c not recognized\n", currentoption);
				exit(1);
				break;
		}
	}

	sigset_t tempset;
	
	SYSCALL(r,sigemptyset(&tempset),"sigemptyset");
	SYSCALL(r,sigaddset(&tempset, SIGUSR1),"sigaddset"); //NON VOGLIO RICEVERE SIGUSR1
	SYSCALL(r,pthread_sigmask(SIG_SETMASK, &tempset, NULL),"sigmask");
	
	
	pw = new_wator(fileplan);
	
	if(pw == NULL) ERRORMESSAGE("Planet file was not in a correct format\n");
	
	atexit(cleanList);
	atexit(cleanWator); 				//ATEXIT
	
	/** Formule con le quali si calcola il valore equo della dimensione delle fette di pianeta in base al numero di worker
	 */
	altezza = (int)ceil((double)pw->plan->nrow / workers);
	larghezza = (int)ceil((double)pw->plan->ncol / workers);
	
	sliceCounter = (int) ( ceil(pw->plan->nrow / ALT) * ceil(pw->plan->ncol / LAR) );
	SYSCALL(r,sigemptyset(&tempset),"sigemptyset");
	SYSCALL(r,pthread_sigmask(SIG_SETMASK, &tempset, NULL),"sigmask");
	
	if(gotn) pw->nwork = workers;

	switch( pid=fork() )
	{
		case 0:
			execl("./visualizer", "visualizer", dumpfile, (char*)NULL);
			break;
		case -1:
			perror("fork");
			exit(1);
			break;
		default:break;
	}
	
	//********************************************
	//SONO IL PROCESSO WATOR, ADESSO CREO I THREAD
	
	workersid = (int*) malloc(workers*sizeof(int));
	if(workersid == NULL)
		exit(1);
	
	//CREAZIONE DEGLI WORKER
	for (int i = 0; i < workers; i++) 
	{
		workersid[i] = i;
		pthread_t tid;
		pthread_attr_t att;
		pthread_attr_init(&att);
		pthread_attr_setdetachstate(&att, PTHREAD_CREATE_DETACHED);
		if ( (err = pthread_create(&tid, &att, &workerroutine, (void*)&workersid[i])) != 0 )
			{ errno = err; perror("pthread_create"); exit(1); }
			
		pthread_attr_destroy(&att);
	}

	atexit(cleanWorkersId); 				//ATEXIT
	
	//CREA DISPATCHER	
	if ( (err = pthread_create(&dispatchertid, NULL, &dispatcherroutine, (void*)0)) != 0 )
		{ errno = err; perror("pthread_create"); exit(1); }
	pthread_detach(dispatchertid);
	
	//CREA COLLECTOR
	pthread_t collectortid;
	if ( (err = pthread_create(&collectortid, NULL, &collectorroutine, (void*)0)) != 0 )
		{ errno = err; perror("pthread_create"); exit(1); }
	pthread_detach(dispatchertid);
	
	//MI METTO IN ATTESA DEI SIGUSR1, SIGINT E SIGTERM
	sigset_t set;	
	SYSCALL(r,sigfillset(&set),"sigfillset");
	SYSCALL(r,sigdelset(&set, SIGUSR1),"sigdelset");
	SYSCALL(r,sigdelset(&set, SIGINT),"sigdelset");
	SYSCALL(r,sigdelset(&set, SIGTERM),"sigdelset");

	int fcloseRes;
	
	while(1)
	{		
		if(sigsuspend(&set) == -1 && errno != EINTR)
			{ perror("sigsuspend wator"); exit(errno); }
		if(checkpointFlag == 1)
		{
			checkpointFlag = 0;
			printf("[wator] Ho ricevuto SIGUSR1\n");

			FILE *cp = fopen("wator.check", "w");
			print_planet(cp, pw->plan);
			if( (fcloseRes=fclose(cp)) != 0 )
				printf("[wator] ERRORE IN FCLOSE\n");

		}
		if(termFlag == 1)
		{
			termFlag = 0;
			printf("[wator] Ho ricevuto SIGINT O SIGTERM\n");
			
			if(pthread_kill(collectortid, SIGUSR2)) 
				{ perror("invio di usr2 a collector"); exit(errno); }
			
			int visualizerexitstatus;
			SYSCALL(r,waitpid(pid, &visualizerexitstatus, 0),"waitpid");		
			printf("[wator] Visualizer è uscito con [%d]\n", WEXITSTATUS(visualizerexitstatus));

			exit(0);
		}
	}
	
	return 0;
}


int doesFileExist(const char *filename) 
{
    struct stat st;
    int result = stat(filename, &st);
    if (result == -1 && errno != ENOENT) //PROBLEMA
    {
    	perror("stat");
    	exit(errno);
    }
    return S_ISREG(st.st_mode);//result == 0 &&
}

int ApplyRules(wator_t *pw, Slice sl)
{
	int coorX, coorY, result, borderNearby, actualI, actualJ;
	
	for (int i = 0; i < sl.altezza; i++) // PRIMA TUTTI GLI SQUALI
	{
		for (int j = 0; j < sl.larghezza; j++)
		{
			actualI = i + sl.origineX;
			actualJ = j + sl.origineY;
			//fprintf(stderr, "applico su (%d, %d)\n", actualI, actualJ);
			if(pw->plan->w[actualI][actualJ] == oldMatrix[actualI][actualJ])
			{
				switch(pw->plan->w[actualI][actualJ])
				{
					case SHARK:
						borderNearby = (j==0 || j==1 || j==sl.larghezza-1 || j==sl.larghezza-2) || (i==0 || i==1 || i==sl.altezza-1 || i==sl.altezza-2);
						if(borderNearby)//ACQUISISCO borderMutex PERCHÉ SONO IN PROSSIMITÀ DI UN BORDO
						{
							if((pthread_mutex_lock(&borderMutex)) != 0)
								return -1;
						}

						if( (result = shark_rule1(pw, actualI, actualJ, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }
						
						if(result == MOVE || result == EAT){ //CHIAMO LA REGOLA NELLA CELLA IN CUI MI SONO MOSSO DOPO LA RULE 1
							if( (result = shark_rule2(pw, coorX, coorY, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }
						}
						else if( (result = shark_rule2(pw, actualI, actualJ, &coorX, &coorY)) == -1) 
						{ 
							RETURN(EINVAL, -1); 
						}//CHIAMO LA REGOLA NELLA CELLA ATTUALE
						
						if(borderNearby) pthread_mutex_unlock(&borderMutex);
					break;

					default: break;
				}
			}
		}
	}
	
	for (int i = 0; i < sl.altezza; i++) // POI TUTTI I PESCI
	{
		for (int j = 0; j < sl.larghezza; j++)
		{
			actualI = i + sl.origineX;
			actualJ = j + sl.origineY;
			if(pw->plan->w[actualI][actualJ] == oldMatrix[actualI][actualJ])
			{
				switch(pw->plan->w[actualI][actualJ])
				{
					case FISH:
						borderNearby = (j==0 || j==1 || j==sl.larghezza-1 || j==sl.larghezza-2) || (i==0 || i==1 || i==sl.altezza-1 || i==sl.altezza-2);
						if(borderNearby)//ACQUISISCO borderMutex PERCHÉ SONO IN PROSSIMITÀ DI UN BORDO
						{
							if((pthread_mutex_lock(&borderMutex)) != 0)
								return -1;
						}
						if( (result = fish_rule4(pw, actualI, actualJ, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }						
						if( (result = fish_rule3(pw, actualI, actualJ, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }
						
						if(borderNearby) pthread_mutex_unlock(&borderMutex);
					break;
					
					default: break;
				}
			}
		}
	}
	
	return 0;
}

void cleanWorkersId(void)
{
	free(workersid);
}
void cleanWator(void)
{
	free_wator(pw);
}
void cleanList(void)
{
	FreeList(&l);
}
void usr1Handler(int sig)
{
	if(sig == SIGUSR1)
		checkpointFlag = 1;
}

void usr2Handler(int sig)
{
	if(sig == SIGUSR2)
		usr2Flag = 1;
}

void termHandler(int sig)
{
	write(1,"[TERM]\n", 7);
	termFlag = 1;
}

void* workerroutine(void *arg)
{
	int r;
	sigset_t set;	
	SYSCALL(r,sigfillset(&set),"sigfillset");
	SYSCALL(r,pthread_sigmask(SIG_SETMASK, &set, NULL),"sigmask");	
	//CREAZIONE FILE
	int numeroWorker = *(int*)arg;
	int res;
	char threadno[15];
	sprintf(threadno, "%d", numeroWorker);
	char workerFileName[N] = "wator_worker_";
	strcat(workerFileName, threadno);
	FILE *wf = fopen(workerFileName, "w");
	fclose(wf);
	
	while(1)
	{
	/** SEZIONE CRITICA (PRELIEVO DI UNA FETTA DALLA CODA) */
		if((pthread_mutex_lock(&queueMutex)) != 0)
			pthread_exit((void*) &errno);
			
		Slice item;
		while( l == NULL )//PRELIEVO FETTA
		{
			//printf("worker %d: mi sospendo\n", numeroWorker);
			pthread_cond_wait(&emptyqueueCond, &queueMutex);
		}
		l = Extract(l, &item);
		//HO ACQUISITO LA FETTA
		pthread_mutex_unlock(&queueMutex);
		
		if( (res=ApplyRules(pw, item)) != 0 )
		{
			printf("[worker] esco per fallimento di ApplyRules\n");
			pthread_exit((void*) &errno);
		}
			
		if((pthread_mutex_lock(&workcounterMutex)) != 0)
			pthread_exit((void*) &errno);
		
		workCounter++;
		
		if(workCounter == sliceCounter)
		{
			workCounter = 0;
			workDone = 1;
			//printf("[lastworker] noi abbiamo finito il lavoro\n");
			pthread_cond_signal(&readytosendCond);
		}
		pthread_mutex_unlock(&workcounterMutex);
	}
		
	pthread_exit((void*)0);
}

void* dispatcherroutine(void *arg)
{
	int r;
	sigset_t set;
	SYSCALL(r,sigfillset(&set),"sigfillset");
	SYSCALL(r,pthread_sigmask(SIG_SETMASK, &set, NULL),"sigmask");

	while(1)
	{
		//printf("[dispatcher] FACCIO UNA COPIA DELLA MATRICE\n");
		oldMatrix = CopyMatrix(pw);
		//printf("[dispatcher] HO FATTO UNA COPIA DELLA MATRICE\n");
		if((pthread_mutex_lock(&queueMutex)) != 0)
				pthread_exit((void*) &errno);
		
		//POPOLO LA LISTA
		for (int i = 0; i < pw->plan->nrow; i += ALT)
		{
			for (int j = 0; j < pw->plan->ncol; j += LAR)
			{
				Slice sl;
				sl.origineX = i;
				sl.origineY = j;
				
				if(j + LAR-1 <= pw->plan->ncol)// controllo se la fetta esce fuori dal pianeta
					sl.larghezza = LAR;
				else 
					sl.larghezza = pw->plan->nrow - j;
				
				if(i + ALT-1 <= pw->plan->nrow)// controllo se la fetta esce fuori dal pianeta
					sl.altezza = ALT;
				else 
					sl.altezza = pw->plan->ncol - i;

				l = Add(l, sl);
			}
		}

		pthread_mutex_unlock(&queueMutex);
		pthread_cond_broadcast(&emptyqueueCond);
		
		pthread_mutex_lock(&readytoproduceMutex);
		//printf("[dispatcher] MI SOSPENDO IN ATTESA DELLA SIGNAL\n");
		
		while(!workCollected)
			pthread_cond_wait(&readytoproduceCond, &readytoproduceMutex);
		
		workCollected = 0;
		pthread_mutex_unlock(&readytoproduceMutex);			
		
		free(oldMatrix);

	}// <---- TORNO A POPOLARE LA LISTA	
	
	pthread_exit((void*)0);
}

void* collectorroutine(void *arg)//PUÒ SOLO RICEVERE SIGUSR2
{
	int r, ultimaIterazione=0, scritti=0;
	sigset_t set;
	
	SYSCALL(r,sigfillset(&set),"sigemptyset");
	SYSCALL(r,sigdelset(&set, SIGUSR2),"sigaddset");
	SYSCALL(r,pthread_sigmask(SIG_SETMASK, &set, NULL),"sigmask");	
	struct sigaction s;
	memset(&s, 0, sizeof(s));		
	s.sa_handler = usr2Handler;
	SYSCALL(r,sigaction(SIGUSR2,&s,NULL),"sigaction");

	struct sockaddr_un sa;
	strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
	sa.sun_family = AF_UNIX;			
	
	while(currentChronon < pw->chronon)
	{	
		char bufrighe[N], bufcolonne[N];
		int sfd;
		
		if((pthread_mutex_lock(&readytosendMutex)) != 0)
			return (void*)(-1);
		
		while(!workDone)
			pthread_cond_wait(&readytosendCond, &readytosendMutex);
			
		pthread_mutex_unlock(&readytosendMutex);
		
		workDone = 0;
		workCollected = 1;
		pw->nf = fish_count(pw->plan);
		pw->ns = shark_count(pw->plan);
		//printf("[collector] sono stato svegliato su readytosendCond\n");
		if( (currentChronon % visualcycle == 0) || usr2Flag == 1)
		{
			SYSCALL(sfd,socket(AF_UNIX, SOCK_STREAM, 0),"creazione socket");
			while (connect(sfd,(struct sockaddr*)&sa,sizeof(sa)) == -1 ) //CONNESSIONE ALLA SOCKET
			{
				if ( errno == ENOENT ) sleep(1); else { perror("[collector] connect error, errno="); exit(EXIT_FAILURE); }
			}
			
			if(usr2Flag == 1) 
			{
				//printf("[collector] ho ricevuto SIGUSR2\n");
				usr2Flag = 0;
				ultimaIterazione = 1;
				sprintf(bufrighe, "%ds", pw->plan->nrow); //NUMERO DI MESSAGGI CHE IL VISUALIZER SI DEVE ASPETTARE (compreso 's')		
				SYSCALL(scritti,write(sfd, bufrighe, N),"write buf");
			}
			else
			{
				sprintf(bufrighe, "%d", pw->plan->nrow); //NUMERO DI MESSAGGI CHE IL VISUALIZER SI DEVE ASPETTARE (senza 's')		
				SYSCALL(scritti,write(sfd, bufrighe, N),"write buf");
			}
			sprintf(bufcolonne, "%d", pw->plan->ncol); //NUMERO DI COLONNE	
			SYSCALL(scritti,write(sfd, bufcolonne, N),"write buf");

			for (int i = 0; i < pw->plan->nrow; i++)
			{
				SYSCALL(scritti,write(sfd, pw->plan->w[i], pw->plan->ncol * sizeof(cell_t)),"write riga pianeta");
			}
			SYSCALL(r,close(sfd),"[collector] close della socket");
			
		}
		
		if(currentChronon == pw->chronon - 2) //LA SIMULAZIONE HA QUASI RAGGIUNTO IL TERMINE
		{
			kill(watorPid, SIGTERM);
		}
		
		if(!ultimaIterazione)//DISPATCHER PUÒ DI NUOVO PRODURRE
		{
			iterazioniCompiute++;
			//printf("[collector] iterazioni compiute++\n");
			
			pthread_cond_signal(&readytoproduceCond);
		}
		//else printf("[collector] ultimaIterazione = %d\n", ultimaIterazione);
		
		currentChronon++;
		if(ultimaIterazione) break;
		
		
	}// <---- TORNO ALL'INIZIO DEL WHILE
		

	pthread_exit((void*)0);
}







