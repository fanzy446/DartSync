all: test/downloader test/uploader test/multi_downloaders DartSyncTracker DartSyncPeer

DartSyncTracker: DartSyncTracker.c common/peertable.o common/seg.o common/filetable.o 
	gcc -Wall -pedantic -std=c99 -g -pthread DartSyncTracker.c common/peertable.o common/seg.o common/filetable.o -o DartSyncTracker

DartSyncPeer: DartSyncPeer.c common/seg.o peer/filemonitor.o common/filetable.o peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread DartSyncPeer.c common/seg.o peer/filemonitor.o common/filetable.o peer/p2p.o -o DartSyncPeer -lcrypto

peer/filemonitor.o: peer/filemonitor.c peer/filemonitor.h
	gcc -Wall -pedantic -g -c peer/filemonitor.c -o peer/filemonitor.o 

common/filetable.o: common/filetable.c common/filetable.h
	gcc -Wall -pedantic -D_GNU_SOURCE -std=c99 -g -c common/filetable.c -o common/filetable.o

common/peertable.o: common/peertable.c common/peertable.h
	gcc -Wall -pedantic -std=c99 -g -c common/peertable.c -o common/peertable.o

common/seg.o: common/seg.c common/seg.h
	gcc -Wall -pedantic -std=c99 -g -c common/seg.c -o common/seg.o

peer/p2p.o: peer/p2p.c
	gcc -Wall -pedantic -std=c99 -g -lcrypto -c peer/p2p.c -o peer/p2p.o

test/downloader: test/downloader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread -lcrypto test/downloader.c peer/p2p.o -o test/downloader

test/multi_downloaders: test/multi_downloaders.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread -lcrypto test/multi_downloaders.c peer/p2p.o -o test/multi_downloaders

test/uploader: test/uploader.c peer/p2p.o
	gcc -Wall -pedantic -std=c99 -g -pthread -lcrypto test/uploader.c peer/p2p.o -o test/uploader

#test/filemonitor_test: peer/filemonitor.c peer/filemonitor.h common/filetable.c common/filetable.h common/seg.c common/seg.h peer/p2p.c peer/p2p.h test/filemonitor_test.c
#	gcc -Wall -D_GNU_SOURCE -pedantic -std=c99 -pthread -g test/filemonitor_test.c peer/filemonitor.c common/filetable.c -o test/filemonitor_test

#test/filetable_test: common/filetable.c common/filetable.h test/filetable_test.c
#	gcc -D_GNU_SOURCE -pedantic -std=c99 -g test/filetable_test.c common/filetable.c -o test/filetable_test


clean:
	rm -rf DartSyncTracker
	rm -rf DartSyncPeer
	rm -rf common/peertable.o
	rm -rf common/filetable.o
	rm -rf common/seg.o
	rm -rf peer/*.o
	rm -rf test/downloader
	rm -rf test/uploader
	rm -rf test/multi_downloaders
	rm -f  test/filetable_test
	rm -f  test/filemonitor_test
	rm -rf test/*.o



