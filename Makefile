CC = gcc

CFLAGS = -Wall -pedantic -std=c99

assignment2:
	$(CC) $(CFLAGS) -o assignment2 assignment2.c -pthread

clean:
	rm *.o assignment2

run:
	./assignment2 in.txt out.txt ${T}

checkleak:
	valgrind --leak-check=full ./assignment2 in.txt out.txt 5
