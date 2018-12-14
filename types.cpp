#include "types.h"
#include "transversal.h"

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <functional>
#include <cstdlib>

/*
 * Node functions
 */

int Node::nextID = 0;

Node::Node(std::string& name,
           double xLoc,
           double yLoc,
           double send,
           double receive,
           double route,
           int queue)
  : xLoc(xLoc)
  , yLoc(yLoc)
  , name(name)
  , flag(false)
  , sendRate(send)
  , receiveRate(receive)
  , routeRate(route)
  , queueSize(queue)
  , nextAvailableRouteTime(0)
{
  id = nextID;
  nextID++;
}

Node& getNodeByName(std::string& name)
{
  for(Node& node : nodes) {
    if(node.name == name) {
      return node;
    }
  }

  std::cerr << "Could not find requested node \'" << name << "\'\n";
  exit(-1);
}

void
Node::connect(Node& node, WireType& wire)
{
  connections.emplace_back(Connection(wire, *this, node));
  int connectionIndex = connections.size() - 1;
  connectionIndicies.push_back(connectionIndex);
  node.connectionIndicies.push_back(connectionIndex);
}

std::string
Node::toXML()
{
  std::ostringstream result;
  result << "<Node";
  result << " name='" + name + '\'';
  result << " x='" << xLoc << "'";
  result << " y='" << yLoc << "'";
  result << " sendRate='" << sendRate << "'";
  result << " receiveRate='" << receiveRate << "'";
  result << " routeRate='" << routeRate << "'";
  result << " queueSize='" << queueSize << "'";
  result << "/>";
  return result.str();
}

bool Node::clearAllFlags()
{
    for(Node& node : nodes) {
        node.clear_Flag();
    }
    return true;
}

bool Node::addToHash()
{
    hashed_nodes[name] = this;
    return true;
}

double
Node::getNextPacketTime()
{
  static double randRange = 0.05;
  double randDouble = (double)std::rand() / RAND_MAX;
  return 1 / (sendRate * (1 + 2 * randRange * (randDouble - 0.5)));
}

bool update_node_hash()
{
    hashed_nodes.clear();
    for(Node& node : nodes) {
        node.addToHash();
    }
    return true;
}

std::ostream&
operator<<(std::ostream& out, const Node& node)
{
  out << "NODE {";
  out << node.name << ", id=" << node.id;
  out << "}" << std::endl;
  for(int index : node.connectionIndicies) {
    out << connections[index] << std::endl;
  }
  return out;
}

bool
operator<(const WireType& wire1, const WireType& wire2)
{
  return wire1.cost < wire2.cost;
}

netpacket_t::netpacket_t(double sendTime, Node * source)
  : sendTime(sendTime)
  , sourceNode(source)
  , destNode(NULL)
  , lastNode(source)
  , arrived(false)
  , latency(0)
  , currentConnection(NULL)
{}

/*
 * Connection functions
 */

int Connection::nextID = 0;

Connection::Connection(WireType& type, Node& a, Node& b)
  : type(type)
  , a(a)
  , b(b)
  , length(real_distance(a, b))
  , maxPackets(type.bandwidth / 150000000 * length + 1)
  , numPackets(0)
{
  id = nextID;
  nextID++;
  travelTime = real_distance(a, b) / 150000000;
}

bool
Connection::isFull()
{
  return numPackets == maxPackets;
}

std::string
Connection::toXML()
{
  std::ostringstream result;
  result << "<Connection";
  result << " wireType='" << type << "'";
  result << " node1='" << a.name << "'";
  result << " node='" << b.name << "'";
  result << "/>";
  return result.str();
}

std::ostream&
operator<<(std::ostream& out, const Connection& connection)
{
  out << "CONNECTION {";
  out << connection.a.name << " <=> " << connection.b.name;
  out << ", " << connection.type;
  out << "}";
  return out;
}

/*
 * WireType functions
 */

WireType::WireType(std::string& type, double cost, double bandwidth, double errorRate)
  : typeName(type)
  , cost(cost)
  , bandwidth(bandwidth)
  , errorRate(errorRate)
{}

std::string
WireType::toXML()
{
  std::ostringstream result;
  result << "<Wire";
  result << " type='" << typeName << "'";
  result << " cost='" << cost << "'";
  result << " bandwidth='" << bandwidth << "'";
  result << " errorRate='" << errorRate << "'";
  result << "/>";
  return result.str();
}

WireType& getWireTypeByName(const char * name)
{
  for(WireType& wire : wires) {
    if(wire.typeName == name) {
      return wire;
    }
  }

  std::cerr << "Could not find requested wire type \'" << name << "\'\n";
  exit(-1);
}

std::ostream&
operator<<(std::ostream& out, const WireType& wire)
{
  out << wire.typeName;
  return out;
}

//BUDGETING FUNCTIONS
double connection_cost(Node& a, Node& b, WireType& wire)
{
  double dist;
  double cost;
  dist = real_distance(a,b);
  cost = wire.cost * dist;
  return cost;
}
double upgrade_cost(int con, WireType wire)
{
  double dist;
  double cost;
  dist = real_distance(connections[con].a,connections[con].b);
  cost = wire.cost * dist;
  return cost;
}
bool can_afford(double cost)
{
  if(cost > prefs.budget)
  {
    return false;
  }
  else
  {
    return true;
  }
}
void pay(double cost)
{
  if(can_afford(cost))
  {
    prefs.budget -= cost;
  }
  else
  {
    std::cerr << "ERROR: ATTEMPT TO OVERSPEND BUDGET";
  }
}
