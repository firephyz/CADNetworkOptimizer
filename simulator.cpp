#include "simulator.h"
#include "types.h"
#include "transversal.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <functional>
#include <cmath>

Simulator::Simulator(
    std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    std::vector<WireType>& wires,
    struct pref_t & prefs)
  : nodes(nodes)
  , connections(connections)
  , wires(wires)
  , totalReceiveRate(0)
  , prefs(prefs)
  , maxSimTime(20)
  , simTime(0)
  , numRuns(0)
{
  // Sort nodes by receive rate for determineDestNode
  for(Node& node : nodes) {
    totalReceiveRate += node.receiveRate;
  }

  // Initialize stats
  stats.avg_latency = 0;
  stats.avg_total_error = 0;
  stats.avg_throughput = 0;
  stats.avg_sent = 0;
  stats.avg_arrived = 0;
  stats.avg_lost_line = 0;
  stats.avg_lost_dispatch = 0;
  stats.avg_lost_routing = 0;
  stats.num_nodes_sent = new double[nodes.size()];
  stats.num_nodes_received = new double[nodes.size()];
  stats.num_nodes_lost_sending = new double[nodes.size()];
  stats.num_nodes_lost_routing = new double[nodes.size()];
  for(uint i = 0; i < nodes.size(); ++i) {
    stats.num_nodes_sent[i] = 0;
    stats.num_nodes_received[i] = 0;
    stats.num_nodes_lost_sending[i] = 0;
    stats.num_nodes_lost_routing[i] = 0;
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
  // Reset simulator
  scheduler.events.clear();
  for(Connection& con : connections) {
    con.numPackets = 0;
  }
  simTime = 0;
  stats.packets.clear();

  // Schedule initial packets
  for(Node& node : nodes) {
    double packetTime = node.getNextPacketTime();
    if(packetTime < 0) {
      packetTime *= -1;
    }
    scheduler.schedule((ScheduledEvent){
            packetTime,
            EventType::NODE_GEN_PKT,
            false,
            NetPacket(simTime, &node)});
    node.nextAvailableRouteTime = 0;
  }

  // Begin simulation
  while(simTime < maxSimTime) {
    ScheduledEvent event = scheduler.events.front();
    scheduler.events.pop_front();
    simTime = event.deltaTime;
    NetPacket& packet = event.packet;

    switch(event.type) {
      case EventType::NODE_GEN_PKT:
        //std::cout << "GEN_PKT: {" << packet.sourceNode->id << "}\n";
        packet.destNode = determineDestNode(packet.sourceNode);
        packet.route = routePacket(*packet.destNode, *packet.sourceNode);

        // Start packet on its journey
        if(packet.route.size() == 0) {
          //scheduler.schedule({0, EventType::NODE_RECV_PKT, packet});
          std::cerr << "Simulator routed packet to itself" << std::endl;
        }
        else {
          packet.currentConnection = &connections[packet.route.front()];
          packet.route.pop_front();

          if(!packet.currentConnection->isFull()) {
            packet.currentConnection->numPackets++;

            if(&packet.currentConnection->a == packet.lastNode) {
              packet.nextNode = &packet.currentConnection->b;
            }
            else {
              packet.nextNode = &packet.currentConnection->a;
            }

            if(packet.route.size() == 0) {
              double err = ((double)rand() / RAND_MAX);
              double success = pow((1 - packet.currentConnection->type.errorRate) , real_distance(packet.currentConnection->a,packet.currentConnection->b));
              //std::cout << success << std::endl;
              if(err < success ) {
                scheduler.schedule((ScheduledEvent)
                                           {packet.currentConnection->travelTime + simTime,
                                            EventType::NODE_RECV_PKT,
                                            false,
                                            packet});
              }
              else {
                //std::cout << "check";
                packet.currentConnection->numPackets--;
                packet.arrived = false;
                packet.status = PacketStatus::LOST_ON_LINE;
                stats.packets.push_back(packet);
              }

            }
            else {
              double err = ((double)rand() / RAND_MAX);
              double success = pow((1 - packet.currentConnection->type.errorRate) , real_distance(packet.currentConnection->a,packet.currentConnection->b));
              //std::cout << success << std::endl;
              if(err < success ) {
                scheduler.schedule((ScheduledEvent)
                                           {packet.currentConnection->travelTime + simTime,
                                            EventType::NODE_ROUTE_PKT,
                                            false,
                                            packet});
              }
              else
              {
                //std::cout << "check";
                packet.currentConnection->numPackets--;
                packet.arrived = false;
                packet.status = PacketStatus::LOST_ON_LINE;
                stats.packets.push_back(packet);
              }
            }
          }
          else {
            packet.arrived = false;
            packet.status = PacketStatus::LOST_ON_DISPATCH;
            stats.packets.push_back(packet);
          }
        }

        {
        double nextPacketTime = simTime + packet.lastNode->getNextPacketTime();
        if(nextPacketTime < 0) nextPacketTime *= -1;
        scheduler.schedule((ScheduledEvent)
                    {nextPacketTime,
                    EventType::NODE_GEN_PKT,
                    false,
                    NetPacket(nextPacketTime, packet.lastNode)});


        }
        break;
      case EventType::NODE_ROUTE_PKT:
        if(!event.isQueueing) packet.currentConnection->numPackets--;
        if(simTime < packet.nextNode->nextAvailableRouteTime) {
          // Reschedule the routing
          scheduler.schedule((ScheduledEvent)
            {packet.nextNode->nextAvailableRouteTime,
             EventType::NODE_ROUTE_PKT,
             true,
             packet});
        }
        else {
          packet.nextNode->nextAvailableRouteTime = simTime + 1 / packet.nextNode->routeRate;
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
          if(!packet.currentConnection->isFull()) {
            packet.currentConnection->numPackets++;
            if(packet.route.size() == 0) {
              double err = ((double)rand() / RAND_MAX);
              double success = pow((1 - packet.currentConnection->type.errorRate) , real_distance(packet.currentConnection->a,packet.currentConnection->b));
              //std::cout << success << std::endl;
              if(err < success ) {
                scheduler.schedule((ScheduledEvent)
                                           {packet.currentConnection->travelTime + 1 / packet.nextNode->routeRate + simTime,
                                            EventType::NODE_RECV_PKT,
                                            false,
                                            packet});
              }
              else
              {
                //std::cout << "check";
                packet.currentConnection->numPackets--;
                packet.arrived = false;
                packet.status = PacketStatus::LOST_ON_LINE;
                stats.packets.push_back(packet);
              }
            }
            else {

              double err = ((double)rand() / RAND_MAX);
              double success = pow((1 - packet.currentConnection->type.errorRate) , real_distance(packet.currentConnection->a,packet.currentConnection->b));
              //std::cout << success << std::endl;
              if(err < success ) {
                scheduler.schedule((ScheduledEvent)
                                           {packet.currentConnection->travelTime + 1 / packet.lastNode->routeRate + simTime,
                                            EventType::NODE_ROUTE_PKT,
                                            false,
                                            packet});
              }
              else
              {
                //std::cout << "check";
                packet.currentConnection->numPackets--;
                packet.arrived = false;
                packet.status = PacketStatus::LOST_ON_LINE;
                stats.packets.push_back(packet);
              }
            }
          }
          else {
            packet.arrived = false;
            packet.status = PacketStatus::LOST_ON_ROUTING;
            stats.packets.push_back(packet);
          }
        }
        break;
      case EventType::NODE_RECV_PKT:
        packet.currentConnection->numPackets--;
        packet.arrived = true;
        packet.latency = simTime - packet.sendTime;
        packet.status = PacketStatus::ARRIVED;
        stats.packets.push_back(packet);
        //std::cout << "RECV_PKT: " << simTime << " {" << packet.destNode->id << ", latency: " << packet.latency;
       // std::cout << " , sendTime: " << packet.sendTime << "}\n";
        break;
    }
  }

  gatherStats();
}

// Don't want to migrate these functions from main to avoid breaking Nickys code.
extern double simmed_avg_latency(Simulator&, double);
extern double simmed_total_error_rate(Simulator&, double);
extern double simmed_throughput(Simulator&, double, double);
extern double simmed_lost_by_status(Simulator&, PacketStatus status);

void
Simulator::gatherStats()
{
  stats.avg_latency += simmed_avg_latency(*this, 0) / numRuns;
  stats.avg_total_error += simmed_total_error_rate(*this, 0) / numRuns;
  stats.avg_throughput += simmed_throughput(*this, 0, maxSimTime) / numRuns;
  stats.avg_lost_line += simmed_lost_by_status(*this, PacketStatus::LOST_ON_LINE) / numRuns;
  stats.avg_lost_dispatch += simmed_lost_by_status(*this, PacketStatus::LOST_ON_DISPATCH) / numRuns;
  stats.avg_lost_routing += simmed_lost_by_status(*this, PacketStatus::LOST_ON_ROUTING) / numRuns;
  stats.avg_sent += stats.packets.size() / (double)numRuns;
  for(NetPacket& packet : stats.packets) {
    ++stats.num_nodes_sent[packet.sourceNode->id];
    if(packet.arrived) {
      ++stats.num_nodes_received[packet.destNode->id];
    }
    else if(packet.status == PacketStatus::LOST_ON_DISPATCH) {
      ++stats.num_nodes_lost_sending[packet.sourceNode->id];
    }
    else if(packet.status == PacketStatus::LOST_ON_ROUTING) {
      ++stats.num_nodes_lost_routing[packet.lastNode->id];
    }
  }
  double count = 0;
  for(auto& packet : stats.packets) {
    if(packet.arrived) ++count;
  }
  stats.avg_arrived += count / numRuns;
}

void
Simulator::printStats()
{
  std::cout << "Average Packet Latency: " << stats.avg_latency << std::endl;
  std::cout << "Average Percent Packet Loss: " << stats.avg_total_error << std::endl;
  std::cout << "Average Throughput: " << stats.avg_throughput << std::endl;
  std::cout << "Average Packets Sent: " << stats.avg_sent << std::endl;
  std::cout << "Average Packets Arrived: " << stats.avg_arrived << std::endl;
  std::cout << "Average Packets Lost on Line: " << stats.avg_lost_line << std::endl;
  std::cout << "Average Packets Lost on Dispatch: " << stats.avg_lost_dispatch << std::endl;
  std::cout << "Average Packets Lost on Routing: " << stats.avg_lost_routing << std::endl;

  std::cout << std::endl;
  for(Node& node : nodes) {
    std::cout << "Node \'" << node.name << "\'" << " sent " << stats.num_nodes_sent[node.id] / numRuns << std::endl;
  }

  std::cout << std::endl;
  for(Node& node : nodes) {
    std::cout << "Node \'" << node.name << "\'" << " received " << stats.num_nodes_received[node.id] / numRuns << std::endl;
  }

  std::cout << std::endl;
  for(Node& node : nodes) {
    std::cout << "Node \'" << node.name << "\'" << " lost sending " << stats.num_nodes_lost_sending[node.id] / numRuns << std::endl;
  }

  std::cout << std::endl;
  for(Node& node : nodes) {
    std::cout << "Node \'" << node.name << "\'" << " lost routing " << stats.num_nodes_lost_routing[node.id] / numRuns << std::endl;
  }
  std::cout << std::endl;
}

Node *
Simulator::determineDestNode(Node * sourceNode)
{
  Node * result = NULL;

  do {
    double randNum = (double)rand() / RAND_MAX * totalReceiveRate;
    double counter = 0;
    for(uint index = 0; index < nodes.size(); ++index) {
      counter += nodes[index].receiveRate;
      if(randNum <= counter) {
        result = &nodes[index];
        break;
      }
    }
  } while(result == sourceNode);

  return result;
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
