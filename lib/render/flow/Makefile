CC=gcc
CXX=g++
CXXFLAGS=-g -std=c++11

ARCH=$(shell uname)
ifeq ($(ARCH), Darwin)
CC=clang
CXX=clang++
endif

a.out: main.cpp Stream.o Particle.o
	$(CXX) $^ -o $@ $(CXXFLAGS)

Stream.o: Stream.cpp Particle.o
	$(CXX) $< -o $@ $(CXXFLAGS) -c

Particle.o: Particle.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) -c


clean:
	rm *.o *.out 
