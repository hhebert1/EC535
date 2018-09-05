GCC = gcc 

partTwo: simpleISS.o
	$(GCC) simpleISS.o -o test

simpleISS.o: simpleISS.c
	$(GCC) -c simpleISS.c

clean:
	rm -Rf simpleISS.o test