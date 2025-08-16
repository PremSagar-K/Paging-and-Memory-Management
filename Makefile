CXX = g++
CXXFLAGS = -Wall -g

all: lru fifo

lru: lru.o
	$(CXX) $(CXXFLAGS) -o lru lru.o

fifo: fifo.o
	$(CXX) $(CXXFLAGS) -o fifo fifo.o

lru.o: lru.cpp
	$(CXX) $(CXXFLAGS) -c lru.cpp

fifo.o: fifo.cpp
	$(CXX) $(CXXFLAGS) -c fifo.cpp

clean:
	rm -f lru fifo *.o