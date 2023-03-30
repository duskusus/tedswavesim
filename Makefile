CXX=g++

test: main.o
	$(CXX) -g main.o -lm -lSDL2 -o test -pthread

main.o: main.cpp 2dArray.hpp
	$(CXX) -c -g -O3 -pthread -ffast-math main.cpp -o main.o
clean:
	rm *.o test