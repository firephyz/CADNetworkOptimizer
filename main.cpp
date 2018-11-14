#include "types.h"

#include <iostream>
#include <libxml2/libxml/parser.h>

// Populated by input file
std::vector<Node> nodes;
std::vector<Connection> connections;
std::vector<WireType> wires;

void printUsage() {
  std::cerr << "./tool <input_file> <output_file>" << std::endl;
  exit(-1);
}

void readInputFile(char * fileName) {
  xmlDocPtr doc = xmlReadFile(fileName, NULL, 0x0);
  if (doc == NULL) {
    std::cerr << "Couldn't parse xml file \'" << fileName << "\'.\n";
    exit(-1);
  }
  xmlNodePtr root = xmlDocGetRootElement(doc);
}

void constructInitialGraph() {

}

int main(int argc, char **argv) {

  if(argc != 2) {
    printUsage();
  }

  readInputFile(argv[1]);

  constructInitialGraph();

  return 0;
}