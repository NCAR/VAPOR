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

ifeq ($(HOSTNAME), cisl-jasper)
    HOME_DIR=/Users/$(shell whoami)
    VAPOR_LIB_DIR=$(HOME_DIR)/Build/VAPOR-flow-build/lib
    VAPOR_INC_DIR=$(HOME_DIR)/Git/VAPOR-flow-src/include
endif


a.out: main.cpp Particle.o
	$(CXX) $^ -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR) 

Advection.o: Advection.cpp Field.o
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR) 

UnsteadyVAPORField.o: UnsteadyVAPORField.cpp Field.o
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR) -L$(VAPOR_LIB_DIR) -lvdc

SteadyVAPORField.o: SteadyVAPORField.cpp Field.o
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR) -L$(VAPOR_LIB_DIR) -lvdc

OceanField.o: OceanField.cpp Field.o
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR)

Field.o: Field.cpp
	$(CXX) $< -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR)

Particle.o: Particle.cpp
	$(CXX) $^ -c -o $@ $(CXXFLAGS) -I$(VAPOR_INC_DIR)


clean:
	rm -rf *.o *.out*
