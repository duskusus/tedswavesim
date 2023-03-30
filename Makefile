CXX=g++

test: main.o
	$(CXX) -g main.o -lm -lSDL2 -o test -pthread

main.o: main.cpp 2dArray.hpp
	$(CXX) -c -g -O3 -march=native -pthread -ffast-math main.cpp -o main.o -mavx2
clean:
	rm *.o test