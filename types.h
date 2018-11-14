#ifndef TYPES_INCLUDED

#include <string>
#include <vector>

class WireType {
public:
  std::string name;
  double cost; // per meter
  double error_rate; // packets per meter per second
  double bandwidth; // packets per second
};

class Connection;

class Node {
  static int nextID;
public:
  int id;
  double xLoc, yLoc;
  std::vector<const Connection *> connections;
  std::string name;
  double sendRate; // packets per second
  double receiveRate; // packets per second
  double routeRate; // packets per second
  int queueSize;

  Node(std::string name, double send, double receive, double route, int queue);

  friend std::ostream& operator<<(std::ostream& out, const Node& node);
};

class Connection {
public:
  int id;
  WireType& type;
  Node& a;
  Node& b;

  Connection(const WireType& type, Node& a, Node& b);
};

extern std::vector<Node> nodes;
extern std::vector<Connection> connections;
extern std::vector<WireType> wires;

#endif