#ifndef SIMULATOR_INCLUDED
#define SIMULATOR_INCLUDED

#include "types.h"
#include <vector>
#include <list>

enum class EventType {
  NODE_GEN_PKT,
  NODE_ROUTE_PKT,
  NODE_RECV_PKT
};

class Simulator;

typedef struct scheduled_event_t {
  double deltaTime;
  EventType type;
  bool isQueueing;
  NetPacket packet;
} ScheduledEvent;

typedef struct cons_t{
    long num_packets;
    long num_denied;
    double time_full; // percentage of the time spent full
} Cons;

typedef struct stats_t{
    std::vector<NetPacket> packets;
    std::vector<Cons> cons;
} Stats;


class Scheduler {
  std::list<ScheduledEvent> events;
public:
  Scheduler() = default;

  void schedule(ScheduledEvent event);

  friend Simulator;
};

class Simulator {
  std::vector<Node>& nodes;
  std::vector<Connection>& connections;
  std::vector<WireType>& wires;
  // sorted by receive rate. First element of pair is the ratio of receive rate it consumes
  double totalReceiveRate;
  struct pref_t & prefs;
  Scheduler scheduler;

  double totalSimTime;

public:
  double maxSimTime; // max sim time for a single run
  double simTime; // simulation time of the current run
  struct stats_t stats;
  Simulator(std::vector<Node>& nodes,
            std::vector<Connection>& connections,
            std::vector<WireType>& wires,
            struct pref_t & prefs);


  void simulate();
  // Randomly find node based on node receive rates
  Node * determineDestNode(Node * sourceNode);
};

std::list<int> routePacket( Node& dest, Node& src);
// Randomly find node based on node receive rates
Node * determineDestNode(Node * sourceNode);

#endif
