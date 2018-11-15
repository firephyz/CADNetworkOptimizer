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
  bool flag;
  double sendRate; // packets per second
  double receiveRate; // packets per second
  double routeRate; // packets per second
  int queueSize;

  Node(std::string& name, double send, double receive, double route, int queue);

  void connect(Node& node, WireType& wire);
  void clear_Flag() { flag =false; }
  
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
extern unordered_map<string, Node*> hashed_nodes;
bool clear_Flags()
{
    for(std::vector<T>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        it.clear_Flag();
    }
    return true;
}
bool add_node_to_hash(Node node)
{
    hashed_nodes[Node.name] = node*;
    return true;
}
bool update_node_hash()
{
    for(std::vector<T>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        add_node_to_hash(it*);
    }
}
#endif
