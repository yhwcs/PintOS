CC = gcc
TARGET = testlib

$(TARGET): main.o list.o hash.o bitmap.o
		   $(CC) -o $(TARGET) main.o list.o hash.o bitmap.o

main.o: main.c list.h hash.h bitmap.h
	$(CC) -c main.c

list.o: list.c list.h
	$(CC) -c list.c

hash.o: hash.c hash.h
	$(CC) -c hash.c

bitmap.o: bitmap.c hex_dump.c bitmap.h limits.h round.h hex_dump.h
	$(CC) -c bitmap.c

clean:
	rm *.o $(TARGET)
