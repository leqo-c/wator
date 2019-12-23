/** \file myFunctions.h
    \author Leonardo Cariaggi (503140)
    
     Si dichiara che il contenuto di questo file e' in ogni sua parte opera
     originale dell' autore.  
*/

#ifndef __MYFUNCTIONS__H
#define __MYFUNCTIONS__H

#include<stdio.h>
#include<stdlib.h>
#include"wator.h"

/** struttura che identifica una cella e le sue coordinate nel pianeta */
typedef struct neigh
{
	/** coordinata x della cella */
	int x;
	/** coordinata y della cella */
	int y;
	/** cella */
	cell_t cell;
} Neighbor;

/** struttura che identifica una fetta, la sua posizione e le sue dimensioni */
typedef struct slice
{
	/** coordinata x della cella origine */
	int origineX;
	/** coordinata y della cella origine */
	int origineY;
	/** larghezza della fetta */
	int larghezza;
	/** altezza della fetta */
	int altezza;
} Slice;

/** struttura che rappresenta una lista di fette */
typedef struct slicelistitem
{
	Slice slice;
	struct slicelistitem *next;
} SliceListItem;
typedef SliceListItem* SliceList;

/** aggiunge un elemento in coda alla lista
   \param l lista da modificare
   \param item elemento da aggiungere
   
   \retval l a cui è stato aggiunto l'elemento item in coda
 */
SliceList Add(SliceList l, Slice item);

/** rimuove l'elemento in testa alla lista
   \param l lista da modificare
   
   \retval NULL se l è NULL
   \retval l a cui è stato rimosso il valore in testa (memorizzato in *par) 
 */
SliceList Extract(SliceList l, Slice *par);

/** libera la lista passata come parametro
   \param l puntatore alla lista da liberare
 */
void FreeList(SliceList *l);

/** restituisce una copia del pianeta della struttura passata come parametro
   \param pw struttura di cui copio il pianeta
   
   \retval NULL se si è verificato un errore
   \retval una copia di pw->plan->w
 */
cell_t** CopyMatrix(wator_t *pw);

/** controlla se una stringa rispetta il formato "sd num", "sb num" oppure "fb num"
   \param buf stringa da controllare (ipotesi: buf ha almeno 4 caratteri)
   \param storeValue intero passato per riferimento in cui memorizzare l'eventuale valore intero contenuto in buf

   \retval "sb", "sd" o "fb" se tutto è andato bene
   \retval NULL se si è verificato un errore
 */
char* BufferIsValid(char *buf, int *storeValue);

/** controlla se un pianeta è ben formato (comprese anche le sue sottostrutture)
   \param planet pianeta da controllare

   \retval 1 se tutto è andato bene
   \retval 0 se si è verificato un errore
 */
int ValidatePlanet(planet_t *planet);


/** controlla se un wator è ben formato (comprese anche le sue sottostrutture)
   \param pw wator da controllare

   \retval 1 se tutto è andato bene
   \retval 0 se si è verificato un errore
 */
int ValidateWator(wator_t *pw);


/** esegue l'operazione di modulo considerando anche i casi nei quali l'intero su cui operare è negativo (module è supposto > 0)
   \param number numero sul quale eseguire l'operazione
   \param module modulo
   
   \retval number % module se number è positivo
   \retval number + module se number è negativo (si suppone che possa assumere soltanto il valore -1)
 */
int Module(int number, int module);

/** libera la matrice passata come input, comprese tutte le sue righe
   \param mat la matrice da liberare
   \param rows numero di righe della matrice
 */
void freeMatrix(cell_t **mat, int rows);

#endif
