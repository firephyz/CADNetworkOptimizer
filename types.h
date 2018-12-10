#ifndef TYPES_INCLUDED
#define TYPES_INCLUDED

#include <string>
#include <vector>
#include <unordered_map>
#include <list>

struct pref_t {
  double latency;
  double packetLoss;
  double throughput;
  double budget;
};

class WireType {
public:
  std::string typeName;
  double cost; // per meter
  double bandwidth; // packets per second
  double errorRate; // packets per meter per second

  WireType(std::string& type, double cost, double bandwidth, double errorRate);

  std::string toXML();

  friend std::ostream& operator<<(std::ostream& out, const WireType& wire);
  friend bool operator<(const WireType& wire1, const WireType& wire2);
};

WireType& getWireTypeByName(const char * name);

class Connection;
class Node;

typedef struct netpacket_t {
  double sendTime;
  Node * sourceNode;
  Node * destNode;
  Node * lastNode;
  Connection * currentConnection;
  std::list<int> route; // Indexes into the connections vector

  netpacket_t(double sendTime, Node * source); // For packets not yet routed.
} NetPacket;

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
  double getNextPacketTime();
  Node * determineDestNode(); // Stoicastically determine destination node

  friend std::ostream& operator<<(std::ostream& out, const Node& node);

  // Statics
  static bool clearAllFlags();
};

Node& getNodeByName(std::string& name);
bool update_node_hash();

class Connection {
private:
  static int nextID;
public:
  int id;
  WireType& type;
  Node& a;
  Node& b;
  double travelTime;

  Connection(WireType& type, Node& a, Node& b);
  std::string toXML();

  friend std::ostream& operator<<(std::ostream& out, const Connection& connection);
};

extern std::vector<Node> nodes;
extern std::vector<Connection> connections;
extern std::vector<WireType> wires;
extern std::unordered_map<std::string, Node *> hashed_nodes;
extern pref_t prefs;

double connection_cost(Node& a, Node& b, WireType& wire);
double upgrade_cost(int con, WireType wire);
bool can_afford(double cost);
void pay(double cost);

#endif
