CC=gcc
CXX=g++
CXXFLAGS=-g -std=c++11

ARCH=$(shell uname)
ifeq ($(ARCH), Darwin)
CC=clang
CXX=clang++
endif


Stream.o: Stream.cpp Particle.o
	$(CXX) $< -o $@ $(CXXFLAGS) -c

Particle.o: Particle.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) -c


clean:
	rm *.o *.out 
