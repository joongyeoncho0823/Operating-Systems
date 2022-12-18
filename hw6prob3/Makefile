main.exe: fifo.o cv.o spinlock.o tas.o
	gcc -o main.exe fifo.o cv.o spinlock.o tas.o

fifo.o: fifo.c fifo.h
	gcc -c fifo.c

cv.o: cv.c cv.h
	gcc -c cv.c

spinlock.o: spinlock.c spinlock.h
	gcc -c spinlock.c

tas.o: tas.S tas.h
	gcc -c tas.S

clean:
	rm -f *.exe *.o *.stackdump *~

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp *.h backups
	cp Makefile backups