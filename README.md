# Wator

This file contains the instructions for building and executing the program.
Equivalent instructions in Italian can be found in the [README_wator_italian.txt](README_wator_italian.txt)

## Preparation of the executables
	
cd into the folder containing the Makefile and execute either "make" or
"make setup". The output of the command will be the creation of a library
and of the two executables "wator" and "visualizer".

## Code execution

Open a command line and execute the command 

	./wator file [-n nwork] [-v chronon] [-f dumpfile]

where "file" is the file the planet will be loaded from (mandatory), while
the remaining optional parameters are used to modify the initial
configuration of the simulation. Respectively, -n represents the number of
workers, -v defines the rate at which snapshots of the planet's state are
taken and -f is the output file in which to save them. If not spedicied,
-f is just "stdout".
  
## Termination

To end the simulation, one can either send a SIGINT/SIGTERM signal to
the process (via command line) or wait for it to perform the predfined
number of iterations (10000).
  
## Checkpointing

To take a snapshot of the planet's state in a certain moment of the
execution, send a SIGUSR1 signal to the "wator" process. The planet's
state will be printed to the "wator.check" file.
  
## Configuration file

To load a planet, it is recommended to use the "planet.dat" file. Even
though it is initially empty, it can filled by copying the content of some
example files under the DATA/ folder, like

	planet1.dat, planet2.dat, planet3.dat

To set the life parameters of the planet's inhabitants, just modify the
"wator.conf" file. As for the "planet.dat" file, you can paste the content
of some example files (wator.conf.1 wator.conf.2) under the DATA/ folder.
  
## Checking the validity of an input file

Before executing the wator process it is advisable to check that the file
containing the planet's initial state is well formed. To do so, one should
execute the bash script "watorscript" in the following way

	./watorscript [-s] [-f] [file] [--help]

where -s and -f are two optional parameter that count the number of sharks
(-s) the number of fishes (-f). Note that you can only specifiy one of
them at a time, not both. Finally, "file" is the file that needs to be
checked. If the file is well formatted, the script will print an "OK"
message (plus an eventual number when either -s or -f is specified). On
the contrary, when the file is not well formatted, an error message will
be printed.

The --help option shows how to use the script.
