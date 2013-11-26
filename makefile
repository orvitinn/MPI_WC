CC=mpic++
CFLAGS=-Wall -std=gnu++11
OUT=wc

all: do

do: main.cpp
	$(CC) $(CFLAGS) main.cpp reducer.cpp -o $(OUT)


clean:
	rm -rf $(OUT)
