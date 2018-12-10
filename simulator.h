#ifndef SIMULATOR_INCLUDED
#define SIMULATOR_INCLUDED

#include "types.h"
#include <vector>
#include <list>

enum class EventType {
  NODE_GEN_PKT,
  NODE_RECV_PKT
};

class Simulator;

typedef struct scheduled_event_t {
  double deltaTime;
  EventType type;
  NetPacket * packet;
} ScheduledEvent;

class Scheduler {
  std::list<ScheduledEvent> events;
public:
  Scheduler();

  void schedule(ScheduledEvent event);

  friend Simulator;
};

class Simulator {
  std::vector<Node>& nodes;
  std::vector<Connection>& connections;
  std::vector<WireType>& wires;
  struct pref_t & prefs;
  Scheduler scheduler;

  double simTime; // simulation time of the current run
  double maxSimTime; // max sim time for a single run
  double totalSimTime;

public:
  Simulator(std::vector<Node>& nodes,
    std::vector<Connection>& connections,
    std::vector<WireType>& wires,
    struct pref_t & prefs);

  void simulate();
};

#endif