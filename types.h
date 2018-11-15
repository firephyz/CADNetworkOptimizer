#ifndef TYPES_INCLUDED

#include <string>
#include <vector>

class WireType {
public:
  std::string typeName;
  double cost; // per meter
  double bandwidth; // packets per second
  double errorRate; // packets per meter per second

  WireType(std::string& type, double cost, double bandwidth, double errorRate);

  friend std::ostream& operator<<(std::ostream& out, const WireType& wire);
};

WireType& getWireTypeByName(std::string& name);

class Connection;

class Node {
  static int nextID;
public:
  int id;
  double xLoc, yLoc;
  std::vector<Connection *> nodeConnections;
  std::string name;
  double sendRate; // packets per second
  double receiveRate; // packets per second
  double routeRate; // packets per second
  int queueSize;

  Node(std::string& name, double send, double receive, double route, int queue);

  void connect(Node& node, WireType& wire);

  friend std::ostream& operator<<(std::ostream& out, const Node& node);
};

Node& getNodeByName(std::string& name);

class Connection {
private:
  static int nextID;
public:
  int id;
  WireType& type;
  Node& a;
  Node& b;

  Connection(WireType& type, Node& a, Node& b);

  friend std::ostream& operator<<(std::ostream& out, const Connection& connection);
};

extern std::vector<Node> nodes;
extern std::vector<Connection> connections;
extern std::vector<WireType> wires;

#endif