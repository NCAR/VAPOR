CC=gcc
CXX=g++
CXXFLAGS=-g -std=c++11

ARCH=$(shell uname)
ifeq ($(ARCH), Darwin)
CC=clang
CXX=clang++
endif


Particle.o: Particle.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) -c


clean:
	rm *.o *.out 
