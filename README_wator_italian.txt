********************************************************************************
Istruzioni per l'esecuzione del codice 

	Preparazione degli eseguibili
	
	- Posizionarsi nella cartella contenente il Makefile ed eseguire
	  da terminale il comando make (o, in alternativa, make setup). 
	  Il risultato dell'operazione sarà la creazione di una libreria 
	  e dei due eseguibili "wator" e "visualizer". 
	
	Esecuzione del codice
	
	- Eseguire il programma digitando da terminale il comando
	
		./wator file [-n nwork] [-v chronon] [-f dumpfile]
	  
	  dove file è il file da cui caricare il pianeta su cui lavorare
	  (opzione OBBLIGATORIA), mentre gli altri flag sono opzionali e
	  permettono di modificare a piacere le impostazioni iniziali della
	  simulazione. Nell'ordine, n rappresenta il numero di worker, v
	  stabilisce ogni quanti chronon si debba visualizzare la situazione 
	  del pianeta e f rappresenta il file su cui stamparla (se non 
	  specificato la stampa avviene su stdout).
	  
	Terminazione
	
	- Per porre fine alla simulazione, è possibile mandare un segnale 
	  SIGINT o SIGTERM da terminale al processo oppure aspettare che esso
	  termini di compiere un numero di iterazioni prefissato (10000).
	  
	Checkpointing
	
	- Per "fotografare" la situazione del pianeta ad un certo istante 
	  durante la simulazione, è necessario inviare un segnale SIGUSR1 al
	  processo wator. Il pianeta verrà stampato nel file "wator.check".
	  
	File di configurazione
	
	- Per la scelta del file da cui caricare il pianeta è consigliato usare
	  "planet.dat". Inizialmente vuoto, tale file può essere riempito 
	  copiando il contenuto di alcuni file di esempio, denominati 
	  
	  	planet1.dat planet2.dat planet3.dat
	  
	- Per settare invece i parametri vitali degli abitanti del pianeta 
	  wator, modificare il file wator.conf copiando eventualmente il
	  contenuto dei due file di esempio
	  
	  	wator.conf.1 wator.conf.2
	  
	Controllo della validità di un file 
	
	- Prima di eseguire il processo wator è opportuno controllare la validità 
	  del file che viene incluso nel comando digitato da terminale.
	  Per fare ciò è possibile utilizzare lo script bash "watorscript".
	  Procedere digitando il comando
	  
	  	./watorscript [-s] [-f] [file] [--help]
	  	
  	  dove -s e -f sono due opzioni facoltative utili a contare il numero
  	  di pesci o squali presenti nel pianeta (al massimo uno delle due può
  	  essere specificata), mentre file è l'effettivo file di cui è necessario
  	  controllare la correttezza (parametro obbligatorio). L'esito
  	  dell'operazione sarà la stampa di un messaggio "OK" in caso positivo
  	  (eventualmente sarà presente anche un numero) o di un messaggio di 
  	  errore in caso negativo. 
  	  L'opzione --help illustra un corretto utilizzo dello script.
	  
********************************************************************************