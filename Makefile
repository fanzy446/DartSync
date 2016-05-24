all: test/downloader test/uploader test/multi_uploaders

peer/p2p.o: peer/p2p.c
	gcc -Wall -pedantic -std=c99 -g -c peer/p2p.c -o peer/p2p.o

test/downloader: test/downloader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread test/downloader.c peer/p2p.o -o test/downloader

test/multi_uploaders: test/multi_uploaders.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread test/multi_uploaders.c peer/p2p.o -o test/multi_uploaders

test/uploader: test/uploader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread test/uploader.c peer/p2p.o -o test/uploader

clean:
	rm -rf peer/*.o
	rm -rf test/downloader
	rm -rf test/uploader
	rm -rf test/multi_uploaders
	rm -rf test/*.o



