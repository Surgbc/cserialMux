all: src/cserialMultiplexing.c
	gcc -o cserialMux src/cserialMultiplexing.c -lncurses -lm
clean: freqmod
	rm cserialmux
