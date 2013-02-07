all: test.cpp gtthreads.cpp
	g++ -o test gtthreads.cpp test.cpp

clean:
	rm test
