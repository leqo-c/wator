/** \file myFunctions.c
    \author Leonardo Cariaggi (503140)
    
     Si dichiara che il contenuto di questo file e' in ogni sua parte opera
     originale dell' autore.  
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "wator.h"
#include "myFunctions.h"

#define RETURN(e, x); errno=e; return x;

SliceList Add(SliceList l, Slice item)
{
	SliceList ris, tmp;
	if(l == NULL)
	{
		ris = (SliceList) malloc(sizeof(SliceListItem));
		ris->slice = item;
		ris->next = NULL;
	}
	else
	{
		tmp = l;	
		while(tmp->next != NULL)
		{
			tmp = tmp->next;
		}			
		tmp->next = (SliceList) malloc(sizeof(SliceListItem));
		tmp->next->slice = item;
		tmp->next->next = NULL;
		ris = l;
	}
	
	return ris;
}

SliceList Extract(SliceList l, Slice *par)
{
	if(l == NULL)
	{
		return NULL;
	}
	else
	{
		SliceList aus = l;
		*par = l->slice;
		l = l->next;
		free(aus);
	}
	
	return l;
}

void FreeList(SliceList *l)
{
	SliceList aus;
	while (*l != NULL)
	{
		aus = *l;
		*l = (*l)->next;
		free(aus);
	}
}

cell_t** CopyMatrix(wator_t *pw)
{
	cell_t **oldMatrix = (cell_t**) malloc(pw->plan->nrow * sizeof(cell_t*));//array di puntatori a righe
	if(oldMatrix == NULL) 
	{
		free(oldMatrix);
		errno = ENOMEM; // NOT ENOUGH SPACE
		return NULL;
	}
	for (int i = 0; i < pw->plan->nrow; i++)
	{
		oldMatrix[i] = (cell_t*) malloc(pw->plan->ncol * sizeof(cell_t));
		
		if (oldMatrix[i] == NULL) {
			freeMatrix(oldMatrix, pw->plan->nrow);
			return NULL;		
		}
		
		for (int j = 0; j < pw->plan->ncol; j++)
			oldMatrix[i][j] = pw->plan->w[i][j];
		
	}
	return oldMatrix;
}

char* BufferIsValid(char *buf, int *storeValue)
{
	// IPOTESI: buf HA ALMENO 4 CARATTERI	
	// CONTROLLO SE buf RISPETTA IL FORMATO "sd num"
	
	
	char primeTre[3];//PRIMI TRE CARATTERI DELLA STRINGA buf
	
	strncpy(primeTre, buf, 3);
	char *toReturn;

	if (strcmp(primeTre, "sd ") == 0)
	{
		toReturn = "sd";
	}
	else if (strcmp(primeTre, "sb ") == 0)
	{
		toReturn = "sb";
	}
	else if (strcmp(primeTre, "fb ") == 0)
	{
		toReturn = "fb";
	}
	else 
	{
		RETURN(ERANGE, NULL);// MAL FORMATTATO		
	}
	
	char *integerValueString = buf + 3;// SUPPONGO CHE IL VALORE INTERO PRESENTE NELLA STRINGA COMINCI DAL QUARTO CARATTERE (formato: "sd num")
	int integerValue;
	char *errorBuffer = ""; //BUFFER D'APPOGGIO PER VERIFICARE IL BUON ESITO DELLA FUNZIONE strtol
	
	integerValue = strtol(integerValueString, &errorBuffer, 10);
	
	// CONTROLLO SE LA CONVERSIONE È ANDATA BENE
	if (errorBuffer[0] == '\n' || errorBuffer[0] == '\0')
	{
		if (integerValue >= 0)
		{
			*storeValue = integerValue;
			return toReturn;
		}
		else 
			return NULL; //NON ACCETTO VALORI NEGATIVI PER sd, sb o fb
	}
	else
	{
		return NULL;
	} 
}


int ValidatePlanet(planet_t *planet)
{
	if(planet == NULL) return 0;
	if(planet->w == NULL) return 0;
	if(planet->btime == NULL) return 0;
	if(planet->dtime == NULL) return 0;
	if(planet->nrow <= 0) return 0;
	if(planet->ncol <= 0) return 0;
	
	for(int i = 0; i < planet->nrow; i++) //CONTROLLA LA VALIDITÀ DI W, BTIME E DTIME
	{
		if(planet->w[i] == NULL) return 0;
		if(planet->btime[i] == NULL) return 0;
		if(planet->dtime[i] == NULL) return 0;
		for(int j = 0; j < planet->ncol; j++)
		{
			if( cell_to_char(planet->w[i][j]) == '?') return 0; 
			if(planet->btime[i][j] < 0) return 0;
			if(planet->dtime[i][j] < 0) return 0;
		}
			
	}
	
	return 1;
}

int ValidateWator(wator_t *pw)
{
	if(pw == NULL) return 0;
	if(pw->sd <= 0) return 0;
	if(pw->sb <= 0) return 0;
	if(pw->fb <= 0) return 0;
	if(pw->nf < 0) return 0;
	if(pw->ns < 0) return 0;
	
	//if(pw->nwork <= 0) return 0; //PER ADESSO NON CONTROLLO I VALORI DI QUESTI DUE CAMPI
	//if(pw->chronon <= 0) return 0;
	
	if( !ValidatePlanet(pw->plan) ) return 0;
	
	return 1;
}

int Module(int number, int module) // OPERAZIONE DI MODULO (COMPRENDENTE ANCHE CASI IN CUI IL NUMERO È NEGATIVO)
{
	if (number >= 0)
		return number%module;
	
	else return number + module;
}

void freeMatrix(cell_t **mat, int rows)
{
	if(mat == NULL) return;
	for (int i = 0; i < rows; i++)
		free(mat[i]);
		
	free(mat);
}
