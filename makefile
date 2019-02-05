CC = clang++
CFLAGS = -std=c++11 -pthread -O3 -Wall -pedantic

hall.bin: prob_game.cpp makefile
	$(CC) $(CFLAGS) prob_game.cpp -o hall.bin
