#include "types.h"

#include <string>
#include <iostream>

/*
 * Node functions
 */

int Node::nextID = 0;

Node::Node(std::string& name, double send, double receive, double route, int queue)
  : xLoc(0.0)
  , yLoc(0.0)
  , name(name)
  , sendRate(send)
  , receiveRate(receive)
  , routeRate(route)
  , queueSize(queue)
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
  Connection& connection = connections.back();
  nodeConnections.push_back(&connection);
  node.nodeConnections.push_back(&connection);
}

bool clear_Flags()
{
    for(Node& node : nodes) {
        node.clear_Flag();
    }
    return true;
}

bool add_node_to_hash(Node& node)
{
    hashed_nodes[node.name] = &node;
    return true;
}

bool update_node_hash()
{
    for(Node& node : nodes) {
        add_node_to_hash(node);
    }
    return true;
}

std::ostream&
operator<<(std::ostream& out, const Node& node)
{
  out << "NODE {";
  out << node.name << ", id=" << node.id;
  out << "}" << std::endl;
  for(Connection * con : node.nodeConnections) {
    out << *con << std::endl;
  }
  return out;
}

/*
 * Connection functions
 */

int Connection::nextID = 0;

Connection::Connection(WireType& type, Node& a, Node& b)
  : type(type)
  , a(a)
  , b(b)
{
  id = nextID;
  nextID++;
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

WireType& getWireTypeByName(std::string& name)
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