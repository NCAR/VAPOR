CC=gcc
CXX=g++
CXXFLAGS=-g -std=c++11

ARCH=$(shell uname)
ifeq ($(ARCH), Darwin)
    CC=clang
    CXX=clang++
endif

HOME_DIR=/home/$(shell whoami)
VAPOR_LIB_DIR=$(HOME_DIR)
VAPOR_INC_DIR=$(HOME_DIR)

HOSTNAME=$(shell hostname)
ifeq ($(HOSTNAME), wd-jasper)
    VAPOR_LIB_DIR=$(HOME_DIR)/Build/VAPOR-flow-build/lib
    VAPOR_INC_DIR=$(HOME_DIR)/Git/VAPOR-flow-src/include
endif


a.out: main.cpp Advection.o OceanField.o VelocityField.o OceanField.o Particle.o
	$(CXX) $^ -o $@ $(CXXFLAGS)

Advection.o: Advection.cpp VelocityField.o
	$(CXX) $< -o $@ $(CXXFLAGS) -c

VaporField.o: VaporField.cpp VelocityField.o
	$(CXX) $< -o $@ $(CXXFLAGS) -c -I$(VAPOR_INC_DIR) -L$(VAPOR_LIB_DIR) -lvdc

OceanField.o: OceanField.cpp VelocityField.o
	$(CXX) $< -o $@ $(CXXFLAGS) -c

VelocityField.o: VelocityField.cpp
	$(CXX) $< -o $@ $(CXXFLAGS) -c

Particle.o: Particle.cpp
	$(CXX) $^ -o $@ $(CXXFLAGS) -c


clean:
	rm -rf *.o *.out*
