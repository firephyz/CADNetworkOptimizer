#include "simulator.h"
#include "types.h"
#include "transversal.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <functional>

Simulator::Simulator(
    std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    std::vector<WireType>& wires,
    struct pref_t & prefs)
  : nodes(nodes)
  , connections(connections)
  , wires(wires)
  , prefs(prefs)
  , simTime(0)
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
    simTime = event.deltaTime;
    NetPacket& packet = event.packet;

    switch(event.type) {
      case EventType::NODE_GEN_PKT:
        std::cout << "GEN_PKT: {" << packet.sourceNode->id << "}\n";
        packet.destNode = determineDestNode(packet.sourceNode);
        packet.route = routePacket(*packet.destNode, *packet.sourceNode);

        // Start packet on its journey
        if(packet.route.size() == 0) {
          //scheduler.schedule({0, EventType::NODE_RECV_PKT, packet});
          std::cerr << "Simulator routed packet to itself" << std::endl;
        }
        else {
          //connection.packets.push(&packet);
          packet.currentConnection = &connections[packet.route.front()];
          packet.route.pop_front();
          if(&packet.currentConnection->a == packet.lastNode) {
            packet.nextNode = &packet.currentConnection->b;
          }
          else {
            packet.nextNode = &packet.currentConnection->a;
          }

          if(packet.route.size() == 0) {
            scheduler.schedule((ScheduledEvent)
              {packet.currentConnection->travelTime + simTime,
               EventType::NODE_RECV_PKT,
               packet});
          }
          else {
            scheduler.schedule((ScheduledEvent)
              {packet.currentConnection->travelTime + simTime,
               EventType::NODE_ROUTE_PKT,
               packet});
          }
        }

        {
        double nextPacketTime = simTime + packet.lastNode->getNextPacketTime();
        if(nextPacketTime < 0) nextPacketTime *= -1;
        scheduler.schedule((ScheduledEvent)
          {nextPacketTime,
           EventType::NODE_GEN_PKT,
           NetPacket(nextPacketTime, packet.lastNode)});
        }
        break;
      case EventType::NODE_ROUTE_PKT:

        if(simTime < packet.nextNode->nextAvailableRouteTime) {
          // Reschedule the routing
          scheduler.schedule((ScheduledEvent)
            {packet.nextNode->nextAvailableRouteTime,
             EventType::NODE_ROUTE_PKT,
             packet});

        }
        else {

          packet.nextNode->nextAvailableRouteTime += 1 / packet.nextNode->routeRate;
          // Move packet to staging area of node it is routed to
          packet.lastNode = packet.nextNode;
          if(&packet.currentConnection->a == packet.lastNode) {
            packet.nextNode = &packet.currentConnection->b;
          }
          else {
            packet.nextNode = &packet.currentConnection->a;
          }
          packet.currentConnection = &connections[packet.route.front()];
          packet.route.pop_front();

          // Route
          if(packet.route.size() == 0) {
            scheduler.schedule((ScheduledEvent)
              {packet.currentConnection->travelTime + 1 / packet.nextNode->routeRate + simTime,
               EventType::NODE_RECV_PKT,
               packet});
          }
          else {
            scheduler.schedule((ScheduledEvent)
              {packet.currentConnection->travelTime + 1 / packet.nextNode->routeRate + simTime,
               EventType::NODE_ROUTE_PKT,
               packet});
          }
        }
        break;
      case EventType::NODE_RECV_PKT:
        packet.arrived = true;
        packet.latency = simTime - packet.sendTime;
        stats.packets.push_back(packet);
        std::cout << "RECV_PKT: " << simTime << " {" << packet.destNode->id << ", latency: " << packet.latency;
        std::cout << " , sendTime: " << packet.sendTime << "}\n";
        break;
    }
  }
}

Node *
Simulator::determineDestNode(Node * sourceNode)
{
  int nodeIndex = (int)((double)rand() / RAND_MAX * sortedNodes.size());
  while(sortedNodes[nodeIndex] == sourceNode) {
    nodeIndex = (int)((double)rand() / RAND_MAX * sortedNodes.size());
  }
  return sortedNodes[nodeIndex];
}

std::list<int>
routePacket(Node& dest, Node& src)//returns empty in case of error ( src = dest/not connected)
{
  std::vector<std::reference_wrapper<Node>> path = connection_jumps_path(src, dest);
  std::vector<int> cons = path_to_con(path);
  std::list<int> list;
  std::copy( cons.begin(), cons.end(), std::back_inserter( list ) );
  return list;
}
