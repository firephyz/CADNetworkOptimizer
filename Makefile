
SOURCES=main.cpp types.cpp simulator.cpp transversal.cpp

all: types.h simulator.h transversal.h
	g++ -g -Wall -std=c++14 -o tool ${SOURCES} -lxml2
