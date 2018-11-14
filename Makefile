
SOURCES=main.cpp types.cpp

all: types.h
	g++ -g -Wall -std=c++14 -o tool ${SOURCES} -lxml2