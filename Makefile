<<<<<<< HEAD
all: test/downloader test/uploader test/multi_uploaders
=======
all: test/downloader test/uploader test/multi_downloaders DartSyncTracker

DartSyncTracker: DartSyncTracker.c common/peertable.o common/seg.o
	gcc -Wall -pedantic -std=c99 -g -pthread DartSyncTracker.c common/peertable.o common/seg.o -o DartSyncTracker

common/peertable.o: common/peertable.c common/peertable.h
	gcc -Wall -pedantic -std=c99 -g -c common/peertable.c -o common/peertable.o

common/seg.o: common/seg.c common/seg.h
	gcc -Wall -pedantic -std=c99 -g -c common/seg.c -o common/seg.o
>>>>>>> master

peer/p2p.o: peer/p2p.c
	gcc -Wall -pedantic -std=c99 -g -c peer/p2p.c -o peer/p2p.o

test/downloader: test/downloader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread test/downloader.c peer/p2p.o -o test/downloader

test/multi_uploaders: test/multi_uploaders.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread test/multi_uploaders.c peer/p2p.o -o test/multi_uploaders

test/uploader: test/uploader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread test/uploader.c peer/p2p.o -o test/uploader

clean:
	rm -rf DartSyncTracker
	rm -rf common/peertable.o
	rm -rf common/seg.o
	rm -rf peer/*.o
	rm -rf test/downloader
	rm -rf test/uploader
	rm -rf test/multi_uploaders
	rm -rf test/*.o



