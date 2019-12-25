/** \file wator.c
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

char cell_to_char (cell_t a) 
{
    char result;
    switch(a)
    {
    	case SHARK: result = 'S';
    	break;
    	case FISH: result = 'F';
    	break;
    	case WATER: result = 'W';
    	break;
    	default: result = '?';
    	break;
    }
	return result;
}

int char_to_cell (char c)
{
	int result;
	switch(c)
	{
		case 'S': result = 0;
		break;
		case 'F': result = 1;
		break;
		case 'W': result = 2;
		break;
		default: result = -1;
		break;
	}
	return result;
}

planet_t * new_planet (unsigned int nrow, unsigned int ncol)
{
	int i, j;
	if(nrow <= 0 || ncol <= 0) return NULL;
	planet_t *planet = (planet_t*) malloc(sizeof(planet_t));
	if(planet == NULL) return NULL; //controllo errore malloc
	planet->nrow = nrow;
	planet->ncol = ncol;
	
	cell_t **mat = (cell_t**) malloc(nrow * sizeof(cell_t*));//array di puntatori a righe
	if(mat == NULL) 
	{
		free(planet);
		return NULL; //controllo errore malloc (MATRICE NULLA)
	}
	for (i = 0; i < nrow; i += 1)
	{
		mat[i] = (cell_t*) malloc(ncol * sizeof(cell_t));
		if(mat[i] == NULL) 
		{
			free_planet(planet);
			return NULL; //errore allocazione di una riga
		} 
		for (j = 0; j < ncol; j += 1)
			mat[i][j] = WATER;
		
	}
	
	planet->w = mat;
	
	planet->btime = (int**) calloc(nrow, sizeof(int*));// INIZIALIZZA btime ALLA MATRICE NULLA (ZERI)
	if(planet->btime == NULL) { free_planet(planet); return NULL; }
	for (i = 0; i < nrow; i += 1)
	{
		planet->btime[i] = (int*) calloc(ncol, sizeof(int));
		if(planet->btime[i] == NULL) 
		{
			free_planet(planet);
			return NULL; //errore allocazione di una riga
		} 
	}
	
	planet->dtime = (int**) calloc(nrow, sizeof(int*));// INIZIALIZZA dtime ALLA MATRICE NULLA (ZERI)
	if(planet->dtime == NULL) { free_planet(planet); return NULL; }
	for (i = 0; i < nrow; i += 1)
	{
		planet->dtime[i] = (int*) calloc(ncol, sizeof(int));
		if(planet->dtime[i] == NULL) 
		{
			free_planet(planet);
			return NULL; //errore allocazione di una riga
		} 
	}
	
	
	return planet;
}

void free_planet(planet_t *p)
{
	if(p == NULL) return;
	
	unsigned int rows = p->nrow;
	int i;
	
	for (i = 0; i < rows; i += 1)// LIBERA TUTTE LE RIGHE DI W, BTIME, DTIME
	{
		if(p->w != NULL) free(p->w[i]);
		if(p->btime != NULL) free(p->btime[i]);
		if(p->dtime != NULL) free(p->dtime[i]);
	}
	free(p->w); // LIBERA W
	free(p->btime); // LIBERA BTIME
	free(p->dtime); // LIBERA DTIME
	free(p); // LIBERA IL PIANETA
}

int print_planet (FILE* f, planet_t* p)
{
	//if(!ValidatePlanet(p)) { RETURN(EINVAL, -1); } //RETURN(EINVAL, -1); // CONTROLLO CHE BTIME E DTIME NON SIANO NULLI NONOSTANTE NON SIANO UTILIZZATI
	 
	unsigned int rows = p->nrow, columns = p->ncol;

	if(fprintf(f, "%d\n", rows) < 0) { RETURN(EBADF, -1); } //File pointer non valido
	if(fprintf(f, "%d\n", columns) < 0) { RETURN(EBADF, -1); } //File pointer non valido
	
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			if(j == columns-1) //RAGGIUNTO L'ULTIMO ELEMENTO DELLA RIGA NON STAMPO LO SPAZIO E VADO A CAPO
				 fprintf(f, "%c\n", cell_to_char( (p->w)[i][j] ) );
			
			else fprintf(f, "%c ", cell_to_char( (p->w)[i][j] ) );
			
		}
	}
	return 0;	
}


planet_t* load_planet (FILE* f) //f deve essere aperto in lettura
{
	unsigned int rows, columns; 
	int intRows, intColumns;
	
	char tempIntConversionBuffer[15]; //BUFFER IN CUI MEMORIZZO I VALORI DI nrow E ncol LETTI DAL FILE
	
	char *errorBuffer; //BUFFER PER CONTROLLARE IL BUON ESITO DI strtol
	
	if(fgets(tempIntConversionBuffer, 15, f) == NULL) { return NULL; }
	intRows = strtol(tempIntConversionBuffer, &errorBuffer, 10);
	if(errorBuffer[0] != '\0' && errorBuffer[0] != '\n')// LA CONVERSIONE E' ANDATA MALE
	{
		RETURN(ERANGE, NULL);
	}
	
	if(fgets(tempIntConversionBuffer, 15, f) == NULL) { return NULL; }
	intColumns = strtol(tempIntConversionBuffer, &errorBuffer, 10);
	if(errorBuffer[0] != '\0' && errorBuffer[0] != '\n')// LA CONVERSIONE E' ANDATA MALE
	{
		RETURN(ERANGE, NULL);
	}
	
	if (intRows <= 0 || intColumns <= 0)
	{
		RETURN(ERANGE, NULL);
	}
	rows = intRows;
	columns = intColumns;
	
	int dimBuffer = 2*columns + 1; //NEL BUFFER ALLOCO SPAZIO SUFFICIENTE A CONTENERE UN NUMERO DI CARATTERI PARI A NCOL PIÙ SPAZI DI SEPARAZIONE E NEWLINE FINALE
	int columnCharsCount = 0; //CONTATORE CHE MI TIENE TRACCIA DI QUANTI CARATTERI HO LETTO: DEVO LEGGERE ESATTAMENTE ncol CARATTERI
	
	char *buffer = (char*) malloc(dimBuffer * sizeof(char)); //BUFFER IN CUI MEMORIZZO LA RIGA CONTENENTE CELLE DEL PIANETA LETTA DAL FILE
	
	if (buffer == NULL)
	{
		return NULL;
	}
	
	planet_t *planet = new_planet(rows, columns);
	if (planet == NULL)
	{
		free(buffer);
		return NULL; // SE NEW_PLANET FALLISCE
	}
	char tempCell = 0;	
	cell_t convertedCell;

	for (int i = 0; i < rows; i++)// PER OGNI RIGA
	{
		if(fgets(buffer, 2*columns + 1, f) == NULL) //MAL FORMATTATO (POTREBBERO ANCHE ESSERCI POCHE RIGHE)
		{
			free_planet(planet); 
			free(buffer);
			RETURN(ERANGE, NULL);
		}
		//INIZIO A LEGGERE IL BUFFER
		for (int j = 0; j < strlen(buffer); j += 1)
		{
			tempCell = buffer[j];
			if(tempCell == ' ' || tempCell == '\n') continue;
			
			convertedCell = char_to_cell(tempCell);
			if (convertedCell == -1)//MAL FORMATTATO
			{
				free_planet(planet); 
				free(buffer);
				RETURN(ERANGE, NULL);
			}
			else if(columnCharsCount < columns)
			{
				planet->w[i][columnCharsCount] = convertedCell;
				columnCharsCount++;
			}
			else //MAL FORMATTATO
			{
				free_planet(planet); 
				free(buffer);
				RETURN(ERANGE, NULL);
			}
			
		}
		if (columnCharsCount != columns)//MAL FORMATTATO
		{
			free_planet(planet); 
			free(buffer);
			RETURN(ERANGE, NULL);
		}
		columnCharsCount = 0;
		
	}//endfor scorrimento righe
	
	char *errorTestBuffer = (char*) malloc(dimBuffer * sizeof(char));
	if (errorTestBuffer == NULL)
	{
		free_planet(planet); 
		free(buffer);
		RETURN(ENOMEM, NULL);
	}
	// CONTROLLO SE C'È RIMASTO ANCORA QUALCOSA NEL FILE E DO ERRORE IN CASO POSITIVO
	if(fgets(errorTestBuffer, dimBuffer, f) != NULL)
	{	//C'É QUALCOSA CHE NON VA
		free_planet(planet); 
		free(errorTestBuffer);
		free(buffer);
		RETURN(ERANGE, NULL);
	}
	
	free(errorTestBuffer);
	free(buffer);	
	return planet;
}
