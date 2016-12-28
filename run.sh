#!/bin/bash

#compile the program
make

#get the baseband carrier frequency


#get the passband carrier frequency


#get the file to modulate


#get the length of the data to use in modulation

#get the modulation type

#get the symbol rate


#get the modulation type

./cserialMux -f ReadMe.MD

#convert raw to wav
sox -t raw -r 44100 -s -L -c1 -e float -b 32 tmp/rawout tmp/wavout.wav

