all: peer/p2p.o  

peer/p2p.o: common/constants.h common/filetable.h peer/p2p.h
	gcc -Wall -pedantic -std=c99 -g -c peer/p2p.c -o peer/p2p.o

clean:
	rm -rf peer/*.o



