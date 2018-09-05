GCC = gcc 

twentyFour: main.o fifo.o actor.o
	$(GCC) main.o fifo.o actor.o -o twentyfour

main.o: main.c
	$(GCC) -c main.c

fifo.o: fifo.c
	$(GCC) -c fifo.c

actor.o: actor.c
	$(GCC) -c actor.c

clean:
	rm -Rf main.o fifo.o actor.o twentyfour
