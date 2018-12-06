#ifndef SIMULATOR_INCLUDED
#define SIMULATOR_INCLUDED

#include "types.h"
#include <vector>

class Simulator {
  std::vector<Node>& nodes;
  std::vector<Connection>& connections;
  std::vector<WireType>& wires;
  struct pref_t & prefs;

  std::vector<std::pair<double, Node *>> sortedSendNodes;
  std::vector<std::pair<double, Node *>> sortedRecvNodes;

public:
  Simulator(std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    std::vector<WireType>& wires,
    struct pref_t & prefs);

  void simulate();
};

#endif
