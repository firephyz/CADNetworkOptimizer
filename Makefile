
SOURCES=main.cpp types.cpp simulator.cpp

all: types.h simulator.h
	g++ -g -Wall -std=c++14 -o tool ${SOURCES} -lxml2
