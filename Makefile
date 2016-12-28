all: src/cserialMultiplexing.c
	gcc -o cserialmux src/cserialMultiplexing.c -lncurses -lm
clean: freqmod
	rm cserialmux
