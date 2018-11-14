#include "types.h"

#include <string>
#include <iostream>

/*
 * Node functions
 */

int Node::nextID = 0;

Node::Node(std::string name, double send, double receive, double route, int queue)
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

std::ostream& operator<<(std::ostream& out, const Node& node) {
  out << "NODE {";
  out << node.name << ", id=" << node.id;
  out << "}";
  return out;
}