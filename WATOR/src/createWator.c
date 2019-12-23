/** \file createWator.c
    \author Leonardo Cariaggi (503140)
    
     Si dichiara che il contenuto di questo file e' in ogni sua parte opera
     originale dell' autore.  
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include"wator.h"
#include"myFunctions.h"
#define RETURN(e, x); errno=e; return x;

wator_t* new_wator (char* fileplan)// FILE DA CUI CARICARE IL PIANETA
{	
	FILE *conf = fopen(CONFIGURATION_FILE, "r");
	if(conf == NULL) { RETURN(ENOENT, NULL); }// NO SUCH FILE OR DIRECTORY
	
	FILE *planetFile = fopen(fileplan, "r");
	planet_t *planet = load_planet(planetFile);
	if(planet == NULL)
	{
		fclose(conf);
		return NULL;
	}

	wator_t *wator = (wator_t *) malloc(sizeof(wator_t)); //CREO LA SIMULAZIONE
	if(wator == NULL)// MALLOC FALLITA
	{
		free(planet);
		fclose(conf);
		return NULL;
	}
	
	
	/**
		temp:  è la variabile in cui memorizzo il valore intero associato al parametro sb, sd o fb
		buffer:  è l'array in cui memorizzo le righe lette dal file .conf
		coppiaDiCaratteri:  è la variabile d'appoggio utilizzata per capire che parametro ho letto dal file (sd, sb o fb)
	*/
	int righeLette = 0, temp, sdCounter = 0, sbCounter = 0, fbCounter = 0;
	char buffer[15], *coppiaDiCaratteri;
	
	
	while( fgets(buffer, 15 , conf) != NULL)// LEGGI UNA RIGA DI CONF
	{
		if (righeLette > 3) 
		{
			free(planet); free(wator);
			fclose(conf);
			RETURN(ERANGE, NULL);// FILE MAL FORMATTATO, HA PIÙ DI TRE RIGHE
		}
		if (strlen(buffer) < 4)
		{
			free(planet); free(wator);
			fclose(conf);
			RETURN(ERANGE, NULL);// FILE MAL FORMATTATO, I PARAMETRI NEL FILE .conf NON SONO SPECIFICATI CORRETTAMENTE
		}
		
		if(  (coppiaDiCaratteri = BufferIsValid(buffer, &temp))  == NULL)// SETTO ANCHE TEMP
		{
			free(planet); free(wator);
			fclose(conf);
			RETURN(ERANGE, NULL);// FILE MAL FORMATTATO, I PARAMETRI NEL FILE .conf NON RISPETTANO IL FORMATO GIUSTO
		}
		else
		{
			if(strcmp(coppiaDiCaratteri, "sd") == 0) { wator->sd = temp; sdCounter++; }
			else if(strcmp(coppiaDiCaratteri, "sb") == 0) { wator->sb = temp; sbCounter++; }
			else if(strcmp(coppiaDiCaratteri, "fb") == 0) { wator->fb = temp; fbCounter++; }
			
			if(sdCounter > 1 || sbCounter > 1 || fbCounter > 1)// QUALCHE COPPIA È STATA RIPETUTA
			{
				free(planet); free(wator);
				fclose(conf);
				errno = ERANGE;// FILE MAL FORMATTATO
				return NULL;	
			}
			
			righeLette++;
		}
				
	}
	
	wator->ns = shark_count(planet);
	wator->nf = fish_count(planet);	
	wator->plan = planet;
	
	//MANCA CHRONON E NWORK
	wator->nwork = NWORK_DEF;
	wator->chronon = CHRON_DEF;
	
	fclose(planetFile);
	fclose(conf);
	return wator;
	
}

void free_wator(wator_t* pw)
{
	if(pw != NULL)
	{
		free_planet(pw->plan);
		free(pw);
	}
}
