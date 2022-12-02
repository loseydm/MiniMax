CFLAGS=-O3 -std=c++17

all: connect_minimax

connect_minimax: main.cpp
	g++ $(CFLAGS) -o connect_minimax main.cpp

clean:
	rm connect_minimax
