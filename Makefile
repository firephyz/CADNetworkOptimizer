
SOURCES=main.cpp types.cpp transversal.cpp

all: types.h transversal.h
	g++ -g -Wall -std=c++14 -o tool ${SOURCES} -lxml2