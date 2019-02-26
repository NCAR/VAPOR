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
	$(CXX) $< -c -o $@ $(CXXFLAGS)

VaporField.o: VaporField.cpp VelocityField.o
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR) -L$(VAPOR_LIB_DIR) -lvdc

OceanField.o: OceanField.cpp VelocityField.o
	$(CXX) $< -c -o $@ $(CXXFLAGS)

VelocityField.o: VelocityField.cpp
	$(CXX) $< -c -o $@ $(CXXFLAGS)

Particle.o: Particle.cpp
	$(CXX) $^ -c -o $@ $(CXXFLAGS)


clean:
	rm -rf *.o *.out*
