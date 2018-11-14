#ifndef TYPES_INCLUDED

#include <string>
#include <vector>
#include <cstdint>

class WireType {
  std::string name;
  double cost; // per meter
  double error_rate; // packets per meter per second
  double bandwidth; // packets per second
};

class Connection;

class Node {
  uint32_t id;
  double xLoc, yLoc;
  std::vector<const Connection *> connections;
  double sendRate; // packets per second
  double receiveRate; // packets per second
  double routeRate; // packets per second
  int queueSize;

  Node(double send, double receive, double route, int queue);
};

class Connection {
  uint32_t id;
  WireType& type;
  Node& a;
  Node& b;

  Connection(const WireType& type, Node& a, Node& b);
};

extern std::vector<Node> nodes;
extern std::vector<Connection> connections;
extern std::vector<WireType> wires;

#endif