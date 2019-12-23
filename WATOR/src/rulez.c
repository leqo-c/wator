/** \file rulez.c
    \author Leonardo Cariaggi (503140)
    
     Si dichiara che il contenuto di questo file e' in ogni sua parte opera
     originale dell' autore.  
*/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include"wator.h"
#include"myFunctions.h"

#define RETURN(e, x); errno=e; return x; 

int shark_rule1 (wator_t* pw, int x, int y, int *k, int* l)
{
	//if(!ValidateWator(pw)) { RETURN(EINVAL, -1); }//INVALID ARGUMENT
	if(x < 0 || y < 0) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	
	/**
		adj:  array in cui memorizzo le 4 celle adiacenti a quella presa in questione
		fishes:  array in cui memorizzo le celle adiacenti contenenti pesci
		waters:  array in cui memorizzo le celle adiacenti contenenti acqua
	*/
	Neighbor adj[4], fishes[4], waters[4];
	int sharkCounter = 0, fishCounter = 0, waterCounter = 0, randomIndex;
	Neighbor temp;
	
	temp.x = x;
	temp.y = Module(y-1, pw->plan->ncol);
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y-1) LEFT
	adj[0] = temp;
	
	temp.x = Module(x-1, pw->plan->nrow);
	temp.y = y;
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x-1, y) UP
	adj[1] = temp;
	
	temp.x = x;
	temp.y = Module(y+1, pw->plan->ncol);
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y+1) RIGHT
	adj[2] = temp;
	
	temp.x = Module(x+1, pw->plan->nrow);
	temp.y = y;
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x+1, y) DOWN
	adj[3] = temp;
	
	for (int i = 0; i < 4; i += 1)
		if(adj[i].cell == SHARK) sharkCounter++; // CONTA IL NUMERO DI SQUALI ADIACENTI
	
	if(sharkCounter == 4) return STOP;
	
	for (int i = 0; i < 4; i += 1)
		if(adj[i].cell == FISH) fishes[fishCounter++] = adj[i]; // CONTA IL NUMERO DI PESCI ADIACENTI
	
		
	srand(time(NULL));
	
	if (fishCounter > 0)
	{
		randomIndex = rand() % fishCounter;
		// MANGIA IL PESCE E CAMBIA LO STATO DEL PIANETA
		pw->plan->w[ fishes[randomIndex].x ][ fishes[randomIndex].y ] = SHARK;
		pw->plan->btime[ fishes[randomIndex].x ][ fishes[randomIndex].y ] = pw->plan->btime[x][y];
		pw->plan->dtime[ fishes[randomIndex].x ][ fishes[randomIndex].y ] = pw->plan->dtime[x][y];
		
		pw->plan->w[x][y] = WATER;
		pw->plan->btime[x][y] = 0;
		pw->plan->dtime[x][y] = 0;
		
		*k = fishes[randomIndex].x;
		*l = fishes[randomIndex].y;
		return EAT;
	}
	
	for (int i = 0; i < 4; i += 1)
		if(adj[i].cell == WATER) waters[waterCounter++] = adj[i]; // CONTA IL NUMERO DI ACQUE ADIACENTI
	
	if (waterCounter > 0)
	{
		randomIndex = rand() % waterCounter;
		// MUOVITI IN UNA CELLA ACQUA A CASO E CAMBIA LO STATO DEL PIANETA
		pw->plan->w[ waters[randomIndex].x ][ waters[randomIndex].y ] = SHARK;
		pw->plan->btime[ waters[randomIndex].x ][ waters[randomIndex].y ] = pw->plan->btime[x][y];
		pw->plan->dtime[ waters[randomIndex].x ][ waters[randomIndex].y ] = pw->plan->dtime[x][y];
		
		pw->plan->w[x][y] = WATER;
		pw->plan->btime[x][y] = 0;
		pw->plan->dtime[x][y] = 0;
		
		*k = waters[randomIndex].x;
		*l = waters[randomIndex].y;
		return MOVE;
	}
	
	else { RETURN(EINVAL, -1); } // C'È QUALCHE CELLA MAL FORMATTATA
		
}

int shark_rule2 (wator_t* pw, int x, int y, int *k, int* l)
{
	//if(!ValidateWator(pw)) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	if(x < 0 || y < 0) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	
	// NASCITE
	if(pw->plan->btime[x][y] < pw->sb) pw->plan->btime[x][y]++; // NON È ORA DI GENERARE
	else if(pw->plan->btime[x][y] == pw->sb) // È ORA DI GENERARE UN NUOVO SQUALO
	{
		/**
			adj:  array in cui memorizzo le 4 celle adiacenti a quella presa in questione
			waters:  array in cui memorizzo le celle adiacenti contenenti acqua
	    */
		Neighbor adj[4], waters[4];
		int randomIndex, waterCounter = 0;
		Neighbor temp;
	
		temp.x = x;
		temp.y = Module(y-1, pw->plan->ncol);
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y-1) LEFT
		adj[0] = temp;
	
		temp.x = Module(x-1, pw->plan->nrow);
		temp.y = y;
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x-1, y) UP
		adj[1] = temp;
	
		temp.x = x;
		temp.y = Module(y+1, pw->plan->ncol);
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y+1) RIGHT
		adj[2] = temp;
	
		temp.x = Module(x+1, pw->plan->nrow);
		temp.y = y;
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x+1, y) DOWN
		adj[3] = temp;
		
		for (int i = 0; i < 4; i += 1)
			if(adj[i].cell == WATER) waters[waterCounter++] = adj[i];
		
		if(waterCounter > 0)
		{
			srand(time(NULL));
	
			randomIndex = rand() % waterCounter;
			// GENERA LO SQUALO E CAMBIA LO STATO DEL PIANETA
			pw->plan->w[ waters[randomIndex].x ][ waters[randomIndex].y ] = SHARK;
			//pw->plan->btime[ waters[randomIndex].x ][ waters[randomIndex].y ] = 0;
			//pw->plan->dtime[ waters[randomIndex].x ][ waters[randomIndex].y ] = 0; //IN TEORIA DOVREBBERO ESSERE GIÀ A ZERO
			*k = waters[randomIndex].x;
			*l = waters[randomIndex].y;
		}
		
		pw->plan->btime[x][y] = 0; // IN ENTRAMBI I CASI RESETTO BTIME[X][Y]
	
	}
	
	
	// MORTI
	if (pw->plan->dtime[x][y] < pw->sd) { pw->plan->dtime[x][y]++; return ALIVE; }
	else //if(pw->plan->dtime[x][y] == pw->sd) // A QUESTO PUNTO LO SQUALO DEVE MORIRE
	{
		pw->plan->w[x][y] = WATER;
		pw->plan->btime[x][y] = 0;
		pw->plan->dtime[x][y] = 0;
		return DEAD;
	} 
}

int fish_rule3 (wator_t* pw, int x, int y, int *k, int* l)
{
	//if(!ValidateWator(pw)) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	if(x < 0 || y < 0) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	
	/**
		adj:  array in cui memorizzo le 4 celle adiacenti a quella presa in questione
		waters:  array in cui memorizzo le celle adiacenti contenenti acqua
	*/
	
	Neighbor adj[4], waters[4];
	int waterCounter = 0, randomIndex;
	Neighbor temp;
	
	temp.x = x;
	temp.y = Module(y-1, pw->plan->ncol);
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y-1) LEFT
	adj[0] = temp;
	
	temp.x = Module(x-1, pw->plan->nrow);
	temp.y = y;
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x-1, y) UP
	adj[1] = temp;
	
	temp.x = x;
	temp.y = Module(y+1, pw->plan->ncol);
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y+1) RIGHT
	adj[2] = temp;
	
	temp.x = Module(x+1, pw->plan->nrow);
	temp.y = y;
	temp.cell = pw->plan->w[temp.x][temp.y]; //(x+1, y) DOWN
	adj[3] = temp;
	
	for (int i = 0; i < 4; i += 1)
		if(adj[i].cell == WATER) waters[waterCounter++] = adj[i]; // CONTA IL NUMERO DI ACQUE ADIACENTI
		
	if(waterCounter > 0)
	{
		srand(time(NULL));
	
		randomIndex = rand() % waterCounter;
		// IL PESCE SI SPOSTA E CAMBIA LO STATO DEL PIANETA
		pw->plan->w[ waters[randomIndex].x ][ waters[randomIndex].y ] = FISH;
		pw->plan->btime[ waters[randomIndex].x ][ waters[randomIndex].y ] = pw->plan->btime[x][y];
		
		*k = waters[randomIndex].x;
		*l = waters[randomIndex].y;
		
		pw->plan->w[x][y] = WATER;
		pw->plan->btime[x][y] = 0;
		pw->plan->dtime[x][y] = 0;
		
		return MOVE;
	}
	else
		return STOP;		
}

int fish_rule4 (wator_t* pw, int x, int y, int *k, int* l)
{
	//if(!ValidateWator(pw)) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	if(x < 0 || y < 0) { RETURN(EINVAL, -1); } //INVALID ARGUMENT
	
	if(pw->plan->btime[x][y] < pw->fb) pw->plan->btime[x][y]++; // NON È ORA
	else if(pw->plan->btime[x][y] == pw->fb) // È ORA DI GENERARE UN PESCE
	{
		/**
			adj:  array in cui memorizzo le 4 celle adiacenti a quella presa in questione
			waters:  array in cui memorizzo le celle adiacenti contenenti acqua
		*/
		Neighbor adj[4], waters[4];
		int waterCounter = 0, randomIndex;
		Neighbor temp;
	
		temp.x = x;
		temp.y = Module(y-1, pw->plan->ncol);
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y-1) LEFT
		adj[0] = temp;
	
		temp.x = Module(x-1, pw->plan->nrow);
		temp.y = y;
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x-1, y) UP
		adj[1] = temp;
	
		temp.x = x;
		temp.y = Module(y+1, pw->plan->ncol);
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x, y+1) RIGHT
		adj[2] = temp;
	
		temp.x = Module(x+1, pw->plan->nrow);
		temp.y = y;
		temp.cell = pw->plan->w[temp.x][temp.y]; //(x+1, y) DOWN
		adj[3] = temp;
		
		for (int i = 0; i < 4; i += 1)
			if(adj[i].cell == WATER) waters[waterCounter++] = adj[i]; // CONTA IL NUMERO DI ACQUE ADIACENTI
		
		if(waterCounter > 0)
		{
			srand(time(NULL));
	
			randomIndex = rand() % waterCounter;
			// GENERA IL PESCE E CAMBIA LO STATO DEL PIANETA
			pw->plan->w[ waters[randomIndex].x ][ waters[randomIndex].y ] = FISH;
			//pw->plan->btime[ waters[randomIndex].x ][ waters[randomIndex].y ] = 0;
			//pw->plan->dtime[ waters[randomIndex].x ][ waters[randomIndex].y ] = 0; //IN TEORIA DOVREBBERO ESSERE GIÀ A ZERO
			*k = waters[randomIndex].x;
			*l = waters[randomIndex].y;
		}
		
		pw->plan->btime[x][y] = 0; //IN ENTRAMBI I CASI RESETTO BTIME[X][Y]
		
	}
	
	return 0;
}

int fish_count (planet_t* p)
{
	int cont = 0;

	if(!ValidatePlanet(p)) { RETURN(EINVAL, -1); }
	for (int i = 0; i < p->nrow; i++)
		for (int j = 0; j < p->ncol; j++)
			if(p->w[i][j] == FISH) cont++;
		
	return cont;
}

int shark_count (planet_t* p)
{
	int cont = 0;

	if(!ValidatePlanet(p)) { RETURN(EINVAL, -1); }
	for (int i = 0; i < p->nrow; i++)
		for (int j = 0; j < p->ncol; j++)
			if(p->w[i][j] == SHARK) cont++;
		
	return cont;
}

int update_wator (wator_t * pw)
{
	if(! ValidateWator(pw)) { RETURN(EINVAL, -1); }
	
	// oldMatrix VIENE UTILIZZATA PER EVITARE DI APPLICARE LE REGOLE SU PESCI/SQUALI (CHE SI MUOVONO) PIÙ DI UNA VOLTA (È UNA COPIA DELLA MATRICE W).
	// PRIMA DI PROCEDERE AD APPLICARE LA REGOLA, CONTROLLO CHE LA CELLA DI W IN QUESTIONE SIA UGUALE A QUELLA DI oldMatrix.
	cell_t **oldMatrix = (cell_t**) malloc(pw->plan->nrow * sizeof(cell_t*));//array di puntatori a righe
	if(oldMatrix == NULL) 
	{
		free(oldMatrix);
		errno = ENOMEM; // NOT ENOUGH SPACE
		return -1;
	}
	for (int i = 0; i < pw->plan->nrow; i++)
	{
		oldMatrix[i] = (cell_t*) malloc(pw->plan->ncol * sizeof(cell_t));
		
		if (oldMatrix[i] == NULL)
			freeMatrix(oldMatrix, pw->plan->nrow);
		
		for (int j = 0; j < pw->plan->ncol; j++)
			oldMatrix[i][j] = pw->plan->w[i][j];
		
	}
	
	int coorX, coorY, result;
	
	for (int i = 0; i < pw->plan->nrow; i++) // PRIMA TUTTI GLI SQUALI
	{
		for (int j = 0; j < pw->plan->ncol; j++)
		{
			if(pw->plan->w[i][j] == oldMatrix[i][j])
			{
				switch(pw->plan->w[i][j])
				{
					case SHARK:
						if( (result = shark_rule1(pw, i, j, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }
						
						if(result == MOVE || result == EAT){ //CHIAMO LA REGOLA NELLA CELLA IN CUI MI SONO MOSSO DOPO LA RULE 1
							if( (result = shark_rule2(pw, coorX, coorY, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }
						}
						else if( (result = shark_rule2(pw, i, j, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }//CHIAMO LA REGOLA NELLA CELLA ATTUALE
					break;

					default: break;
				}
			}
		}
	}
	
	for (int i = 0; i < pw->plan->nrow; i++) // POI TUTTI I PESCI
	{
		for (int j = 0; j < pw->plan->ncol; j++)
		{
			if(pw->plan->w[i][j] == oldMatrix[i][j])
			{
				switch(pw->plan->w[i][j])
				{
					case FISH:
						if( (result = fish_rule4(pw, i, j, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }						
						if( (result = fish_rule3(pw, i, j, &coorX, &coorY)) == -1) { RETURN(EINVAL, -1); }
					break;
					
					default: break;
				}
			}
		}
	}
	
	freeMatrix(oldMatrix, pw->plan->nrow);
	
	//AGGIORNO I PARAMETRI DELLA SIMULAZIONE
	
	pw->ns = shark_count(pw->plan);
	pw->nf = fish_count(pw->plan);
	return 0;
}
