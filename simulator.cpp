#include "simulator.h"
#include "types.h"

#include <vector>
#include <iostream>
#include <algorithm>

Simulator::Simulator(
    std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    std::vector<WireType>& wires,
    struct pref_t & prefs)
  : nodes(nodes)
  , connections(connections)
  , wires(wires)
  , prefs(prefs)
{
  for(Node& node : nodes) {
    sortedSendNodes.emplace_back(std::pair<double, Node *>(node.sendRate, &node));
  }
  std::sort(sortedSendNodes.begin(), sortedSendNodes.end(),
            [](auto& pair1, auto& pair2) -> bool
    {
      return pair1.second->sendRate < pair2.second->sendRate;
    });
}

void
Simulator::simulate()
{
  prefs.budget = 0;



  // for(const Node& node : nodes) {
  //   std::cout << node << std::endl;
  // }
  //
  // for(const WireType& wire : wires) {
  //   std::cout << wire.typeName << std::endl;
  //   std::cout << wire.cost << std::endl;
  //   std::cout << wire.bandwidth << std::endl;
  //   std::cout << wire.errorRate << std::endl;
  // }
  //
  // for(const Connection& connection : connections) {
  //   std::cout << connection << std::endl;
  // }
}
