#include "types.h"

#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>

/*
 * Node functions
 */
std::vector<Node> dist_sort(Node home, std::vector<Node> list);

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

//TRANSVERSAL FUNCTIONS

double real_distance(Node a, Node b)
{
  return std::sqrt( (a.xLoc-b.xLoc)*(a.xLoc-b.xLoc) + (a.yLoc-b.yLoc)*(a.yLoc-b.yLoc));
}
double net_distance(Node a, Node b)// assumes network is complete
{
  a.flag=true;
  std::vector<Node> list;
  for(int con: a.connectionIndicies)
  {
    if(connections[con].a.id == a.id)
    {
      list.push_back(connections[con].b);
    }
    else
    {
      list.push_back(connections[con].a);
    }
  }
  list = dist_sort(a,list);
  Node::clearAllFlags();
  return 0;
}
std::vector<Node> dist_sort(Node target, std::vector<Node> list)
{
  bool done = false;
  int count;
  double dist1;
  double dist2;
  Node temp = list[0];
  while(!done)
  {
    done =true;
    count = 0;
    for(int i = 0; i < (int)list.size(); i++)
    {
      if(count > 0)
      {
        dist1 = real_distance(target,temp);
        dist2 = real_distance(target,list[i]);
        if(abs(dist2) < abs(dist1))
        {
          std::swap(list[i],list[i-1]);
          done = false;
        }
      }
      temp = list[i];
      count++;
    }
  }
  return list;
}