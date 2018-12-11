#include "simulator.h"
#include "types.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

Simulator::Simulator(
    std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    std::vector<WireType>& wires,
    struct pref_t & prefs)
  : nodes(nodes)
  , connections(connections)
  , wires(wires)
  , prefs(prefs)
  , maxSimTime(1)
{
  // Sort nodes by receive rate for determineDestNode
  for(Node& node : nodes) {
    sortedNodes.push_back(&node);
  }
  std::sort(sortedNodes.begin(), sortedNodes.end(),
    [](auto& a, auto& b)
    {
      return a->receiveRate < b->receiveRate;
    });

  // Schedule initial packets
  for(Node& node : nodes) {
    double packetTime = node.getNextPacketTime();
    if(packetTime < 0) {
      packetTime *= -1;
    }
    scheduler.schedule((ScheduledEvent){
      packetTime,
      EventType::NODE_GEN_PKT,
      NetPacket(simTime, &node)});
  }
}

// Scheduling new events (likely) will be near the end of the
// events list. Could use this fact to improve performance.
void
Scheduler::schedule(ScheduledEvent event)
{
  // Insert event into the right spot on the schedule
  auto pos = events.begin();
  while(pos != events.end()) {
    if(pos->deltaTime > event.deltaTime) {
      events.insert(pos, event);
      break;
    }
    else {
      pos++;
    }
  }
  if(pos == events.end()) {
    events.insert(pos, event);
  }
}

void
Simulator::simulate()
{
  while(simTime < maxSimTime) {
    ScheduledEvent event = scheduler.events.front();
    scheduler.events.pop_front();
    simTime += event.deltaTime;
    NetPacket& packet = event.packet;

    switch(event.type) {
      case EventType::NODE_GEN_PKT:
        packet.destNode = determineDestNode(packet.sourceNode);
        packet.route = routePacket(*packet.destNode, *packet.sourceNode);

        // Start packet on its journey
        if(packet.route.size() == 0) {
          scheduler.schedule({0, EventType::NODE_RECV_PKT, packet});
        }
        else {
          int connIndex = packet.route.front();
          packet.route.pop_front();
          Connection& connection = connections[connIndex];
          //connection.packets.push(&packet);
          packet.currentConnection = &connection;
          scheduler.schedule((ScheduledEvent){connection.travelTime, EventType::NODE_RECV_PKT, packet});
        }
        break;
      case EventType::NODE_RECV_PKT:
        Node * currentNode = &packet.currentConnection->a;
        if(currentNode == packet.lastNode) {
          currentNode = &packet.currentConnection->b;
        }
        packet.lastNode = currentNode;

        // Packet finished route
        if(packet.route.size() == 0) {
          free(&packet);
        }
        else {
          int connIndex = packet.route.front();
          packet.route.pop_front();
          Connection& connection = connections[connIndex];
          packet.currentConnection = &connection;

          scheduler.schedule((ScheduledEvent){connection.travelTime, EventType::NODE_RECV_PKT, packet});
        }
        break;
    }
  }
}

Node *
Simulator::determineDestNode(Node * sourceNode)
{
  double nodeIndex = (double)rand() / RAND_MAX * sortedNodes.size();
  return sortedNodes[(int)nodeIndex];
}

std::list<int>
routePacket(const Node& dest, const Node& src)
{
}
