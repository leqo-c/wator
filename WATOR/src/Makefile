# \file Makefile
# \author Leonardo Cariaggi (503140)
#    
#  Si dichiara che il contenuto di questo file e' in ogni sua parte opera
#  originale dell' autore. 

###############################################

# Makefile WATOR (progetto lso 2015)
     
###############################################

# Compilatore
CC= gcc
# Flag di compilazione
CFLAGS = -std=c99 -Wall -pedantic -g

# Librerie 
# Directory in cui si trovano le librerie
LIBDIR = ../lib
# Opzione di linking
LIBS = -L $(LIBDIR)

# Nome libreria progetto
LIBNAME1 = libWator.a

# Oggetti libreria $(LIBNAME1)
objects1 = wator.o rulez.o myFunctions.o createWator.o


.PHONY: clean cleanall lib

DATA1=DATA/planet1.dat
DATA2=DATA/planet2.dat
DATA3=DATA/planet3.dat

CONF1=DATA/wator.conf.1
CONF2=DATA/wator.conf.2

FILES = $(DATA1) $(DATA2) $(DATA3) $(CONF1) $(CONF2)

setup:
	chmod 644 $(FILES)
	cp $(FILES) .
	make lib
	make wator
	make visualizer

# creazione libreria 
lib:  $(objects1)
	-rm  -f $(LIBNAME1)
	-rm  -f $(LIBDIR)/$(LIBNAME1)
	ar -r $(LIBNAME1) $(objects1)
	cp $(LIBNAME1) $(LIBDIR)
 

#make rule .o del primo frammento

wator.o: wator.c wator.h
	$(CC) $(CFLAGS) -c wator.c
		 
createWator.o: createWator.c wator.h
	$(CC) $(CFLAGS) -c createWator.c
	
myFunctions.o: myFunctions.c wator.h myFunctions.h
	$(CC) $(CFLAGS) -c myFunctions.c
	
rulez.o: rulez.c wator.h myFunctions.h
	$(CC) $(CFLAGS) -c rulez.c



######### target visualizer e wator

wator: Wator.o wator.o createWator.o rulez.o myFunctions.o wator.h myFunctions.h
	$(CC) $(CFLAGS) Wator.o $(objects1) -o $@ -lpthread -lm

visualizer: wator visualizer.o
	$(CC) $(CFLAGS) $(objects1) visualizer.o -o $@ -lpthread

# make rule per Wator.o visualizer.o

Wator.o: Wator.c wator.h
	$(CC) $(CFLAGS) -c Wator.c

visualizer.o: visualizer.c
	$(CC) $(CFLAGS) -c visualizer.c


#ripulisce  l'ambiente 
clean:
	find . -name "*.o" -print -exec rm {} \; 

# cancella i file temporanei e tutto quello che devo ricreare/copiare
cleanall: clean
	\rm -f wator visualizer 
	\rm -f wator_worker_?* visualizer_dump out.wator out*.check planet?.check __wator.log wator.check planet?.dat wator.conf.?
	\rm -f *~
	\rm -f $(LIBDIR)/$(LIBNAME1) $(LIBNAME1)

