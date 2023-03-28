CXX=g++

test: main.o
	$(CXX) -g main.o -lm -lSDL2 -o test

main.o: main.cpp 2dArray.hpp
	$(CXX) -c -g -O3 main.cpp -o main.o -mavx

clean:
	rm *.o test