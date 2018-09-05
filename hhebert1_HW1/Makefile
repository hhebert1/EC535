# our beloved g++ compiler
# goes into a variable!
GCC = gcc 

myBitApp: main.o bits.o
	$(GCC) main.o bits.o -lm -o myBitApp

main.o: main.c
	$(GCC) -c main.c

bits.o: bits.c
	$(GCC) -c bits.c

clean:
	rm -Rf main.o bits.o myBitApp
