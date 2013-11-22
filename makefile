CC=mpic++
CFLAGS=-Wall -std=gnu++11
OUT=mpi_wc

all: do

do: main.cpp
	$(CC) $(CFLAGS) main.cpp -o $(OUT)

clean:
	rm -rf $(OUT)
