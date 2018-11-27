#ifndef TYPES_INCLUDED

#include <string>
#include <vector>
#include <unordered_map>

class WireType {
public:
  std::string typeName;
  double cost; // per meter
  double bandwidth; // packets per second
  double errorRate; // packets per meter per second

  WireType(std::string& type, double cost, double bandwidth, double errorRate);

  std::string toXML();

  friend std::ostream& operator<<(std::ostream& out, const WireType& wire);
};

WireType& getWireTypeByName(std::string& name);

class Connection;

class Node {
  static int nextID;
public:
  int id;
  double xLoc, yLoc;
  std::vector<int> connectionIndicies;
  std::string name;
  bool flag; // Marks for traversal
  double sendRate; // packets per second
  double receiveRate; // packets per second
  double routeRate; // packets per second
  int queueSize;

  Node(std::string& name, double xLoc, double yLoc, double send, double receive, double route, int queue);

  void connect(Node& node, WireType& wire);
  void clear_Flag() { flag =false; }
  bool addToHash();
  std::string toXML();

  friend std::ostream& operator<<(std::ostream& out, const Node& node);

  // Statics
  static bool clearAllFlags();
};

Node& getNodeByName(std::string& name);
bool clear_Flags();
bool add_node_to_hash(Node& node);
bool update_node_hash();

class Connection {
private:
  static int nextID;
public:
  int id;
  WireType& type;
  Node& a;
  Node& b;

  Connection(WireType& type, Node& a, Node& b);
  std::string toXML();

  friend std::ostream& operator<<(std::ostream& out, const Connection& connection);
};

extern std::vector<Node> nodes;
extern std::vector<Connection> connections;
extern std::vector<WireType> wires;
extern std::unordered_map<std::string, Node *> hashed_nodes;

std::vector<std::reference_wrapper<Node>> dist_sort(Node& target, std::vector<std::reference_wrapper<Node>>  list);
bool net_distance_deep(double &dist,Node & current, Node & target);
double net_distance(Node & a, Node & b);
#endif
