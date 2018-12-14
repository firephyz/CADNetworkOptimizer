#include "types.h"
#include "simulator.h"
#include "transversal.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <libxml2/libxml/parser.h>
#include <vector>
#include <functional>
#include <cmath>
#include <ctime>
#include <sstream>

// Populated by input file
std::vector<Node> nodes;
std::vector<Connection> connections;
std::vector<WireType> wires; // sorted by cost
struct pref_t prefs;

std::unordered_map<std::string, Node *> hashed_nodes;

double simmed_avg_latency(Simulator& sim, int time);
double simmed_total_error_rate(Simulator& sim, int time);
double simmed_throughput(Simulator& sim, double startTime, double endTime);

void printUsage()
{
  std::cerr << "./tool <input_file> <output_file>" << std::endl;
  exit(-1);
}

xmlAttrPtr xmlGetAttribute(xmlNodePtr node, const char * attr_name)
{
  xmlAttrPtr result = node->properties;
  while(result != NULL) {
    if(xmlStrcmp(result->name, (const xmlChar *)attr_name) == 0) {
      return result;
    }
    result = result->next;
  }

  return (xmlAttrPtr)NULL;
}

void parseListOfNodes(xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar *)"Node") != 0) {
      std::cerr << "Invalid tag inside ListOfNodes: \'" << child->name << "\'\n.";
      continue;
    }

    std::string name = (const char *)xmlGetAttribute(child, "name")->children->content;
    double xLoc = 1000 * std::atof((const char *)xmlGetAttribute(child, "x")->children->content);
    double yLoc = 1000 * std::atof((const char *)xmlGetAttribute(child, "y")->children->content);
    double sendRate = std::atof((const char *)xmlGetAttribute(child, "sendRate")->children->content);
    double receiveRate = std::atof((const char *)xmlGetAttribute(child, "receiveRate")->children->content);
    double routeRate = std::atof((const char *)xmlGetAttribute(child, "routeRate")->children->content);
    int queueSize = std::atoi((const char *)xmlGetAttribute(child, "queueSize")->children->content);

    nodes.emplace_back(Node(name, xLoc, yLoc, sendRate, receiveRate, routeRate, queueSize));

    child = xmlNextElementSibling(child);
  }
}

void parseWireTypes(xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar *)"Wire") != 0) {
      std::cerr << "Invalid tag inside WireTypes: \'" << child->name << "\'\n.";
      continue;
    }

    std::string type = (const char *)xmlGetAttribute(child, "type")->children->content;
    double cost = std::atof((const char *)xmlGetAttribute(child, "cost")->children->content);
    double bandwidth = std::atof((const char *)xmlGetAttribute(child, "bandwidth")->children->content);
    double errorRate = std::atof((const char *)xmlGetAttribute(child, "errorRate")->children->content) / 1000;

    wires.emplace_back(WireType(type, cost, bandwidth, errorRate));

    child = xmlNextElementSibling(child);
  }

  // Sort wires by cost
  std::sort(wires.begin(), wires.end());
}

void parseListOfConnections(xmlNodePtr node)
{
  xmlNodePtr child = xmlFirstElementChild(node);
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar *)"Connection") != 0) {
      std::cerr << "Invalid tag inside ListOfConnections: \'" << child->name << "\'\n.";
      continue;
    }

    std::string type = (const char *)xmlGetAttribute(child, "wireType")->children->content;
    std::string node1Name = (const char *)xmlGetAttribute(child, "node1")->children->content;
    std::string node2Name = (const char *)xmlGetAttribute(child, "node2")->children->content;

    Node& node1 = getNodeByName(node1Name);
    Node& node2 = getNodeByName(node2Name);
    WireType& wire = getWireTypeByName(type.c_str());

    // Automatically updates external records like 'connections'
    node1.connect(node2, wire);

    child = xmlNextElementSibling(child);
  }
}

void parsePreferences(xmlNodePtr node)
{
  prefs.latency = std::atof((const char *)xmlGetAttribute(node, "latency")->children->content);
  prefs.packetLoss = std::atof((const char *)xmlGetAttribute(node, "packetLoss")->children->content);
  prefs.throughput = std::atof((const char *)xmlGetAttribute(node, "throughput")->children->content);
  prefs.budget = std::atof((const char *)xmlGetAttribute(node, "budget")->children->content);
  prefs.originalBudget = prefs.budget;
}

void readInputFile(char * fileName)
{
  xmlDocPtr doc = xmlReadFile(fileName, NULL, 0x0);
  if (doc == NULL) {
    std::cerr << "Couldn't parse xml file \'" << fileName << "\'.\n";
    exit(-1);
  }
  xmlNodePtr root = xmlDocGetRootElement(doc);
  xmlNodePtr child = xmlFirstElementChild(root);
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar *)"ListOfNodes") == 0) {
      parseListOfNodes(child);
    }
    else if (xmlStrcmp(child->name, (const xmlChar *)"WireTypes") == 0) {
      parseWireTypes(child);
    }
    else if (xmlStrcmp(child->name, (const xmlChar *)"ListOfConnections") == 0) {
      parseListOfConnections(child);
    }
    else if (xmlStrcmp(child->name, (const xmlChar *)"Preferences") == 0) {
      parsePreferences(child);
    }
    else if (xmlStrcmp(child->name, (const xmlChar *)"Statistics") == 0) {
      // Ignore stats field. That is generated by this tool.
    }
    else {
      std::cerr << "Invalid XML tag \'" << child->name << "\'.\n";
    }
    child = xmlNextElementSibling(child);
  }
}

std::string preferencesToXML() {
  std::string result = "";
  result += "<Preferences";
  result += "/>";
  return result;
}

std::string statsToXML(Simulator& sim) {
  std::ostringstream result;
  result << "<Statistics ";
  result << "latency=\'" << simmed_avg_latency(sim,5) << "\' ";
  result << "packetLoss=\'" << simmed_total_error_rate(sim,5) << "\' ";
  result << "throughput=\'" << simmed_throughput(sim, 0, sim.simTime) << "\' ";
  result << "cost=\'" << (prefs.originalBudget - prefs.budget) << "\' ";
  result << "/>";
  return result.str();
}

void outputResults(char * fileName, Simulator& sim) {
  std::ofstream file(fileName, std::ios_base::out);

  file << "<NetworkDefinition>\n";
  file << "\t<ListOfNodes>\n";
  for(auto& node : nodes) {
    file << "\t\t" << node.toXML() << std::endl;
  }
  file << "\t</ListOfNodes>\n";
  file << "\t<WireTypess>\n";
  for(auto& wire : wires) {
    file << "\t\t" << wire.toXML() << std::endl;
  }
  file << "\t</WireTypes>\n";
  file << "\t<ListOfConnections>\n";
  for(auto& connection : connections) {
    file << "\t\t" << connection.toXML() << std::endl;
  }
  file << "\t</ListOfConnections>\n";
  file << "\t" << preferencesToXML() << std::endl;
  file << "\t" << statsToXML(sim) << std::endl;
  file << "</NetworkDefinition>\n";

  file.close();
}


std::vector<std::vector<Node *>>
findNetworkGroups()
{
  std::vector<std::vector<Node *>> groups;
  std::vector<Node *> nodesToBeGrouped;

  for(auto& node : nodes) {
    nodesToBeGrouped.push_back(&node);
  }

  while(!nodesToBeGrouped.empty()) {
    std::vector<Node *> currentNodes;
    for(int i = nodesToBeGrouped.size() - 1; i >= 0; --i) {
      currentNodes.push_back(nodesToBeGrouped[i]);
      nodesToBeGrouped.pop_back();
    }

    std::vector<Node *> group;
    for(auto node : currentNodes) {
      if(group.empty()) {
        group.push_back(node);
      }
      else if(num_jumps_breadth(*node, *group[0]) != -1) {
        group.push_back(node);
      }
      else {
        nodesToBeGrouped.push_back(node);
      }
    }
    groups.push_back(std::move(group));
  }

  return std::move(groups);
}

// TODO Add more decision heuristics other than minimum average distance to other nodes.
// Take into account node characteristics for example. A midnode that has a low routing
// speed will make communication between initial groups very poor.
Node *
findGroupMidNode(std::vector<Node *> group)
{
  double lowestDistance = DBL_MAX;
  Node * bestNode = NULL;

  for(auto& node : group) {
    double averageDistance = 0;
    for(auto& target : group) {
      if(node == target) continue;
      double distance = num_jumps_breadth(*node, *target);
      averageDistance += distance / (group.size() - 1);
    }

    if(averageDistance < lowestDistance) {
      lowestDistance = averageDistance;
      bestNode = node;
    }
  }

  return bestNode;
}

void completeNetworkGraph()
{
  std::vector<std::vector<Node*>> groups = findNetworkGroups();
  std::vector<Node *> midNodes;
  for(auto& group : groups) {
    midNodes.push_back(findGroupMidNode(group));
  }

  if(midNodes.size() == 1) return;

  // TODO improve initial connection.
  WireType& wire = wires[0];
  for(uint i = 0; i < midNodes.size(); ++i) {
    if(i == midNodes.size() - 1) {
      prefs.budget -= connection_cost(*midNodes[i], *midNodes[0],wire);
      midNodes[i]->connect(*midNodes[0], wire);
    }
    else {
      prefs.budget -= connection_cost(*midNodes[i], *midNodes[i + 1],wire);
      midNodes[i]->connect(*midNodes[i + 1], wire);
    }
  }
}
void Graphviz(std::string name)
{
  std::ofstream myfile;
  myfile.open (name);
  myfile << "digraph G { \n";
  for( Node & node: nodes)
  {
    node.flag =true;
    for(int con : node.connectionIndicies)
    {
      if(connections[con].a.id == node.id && connections[con].b.flag == false)
      {
        myfile <<"    " << connections[con].a.name << " -> " << connections[con].b.name << " [label=" << "\""<< connections[con].type.typeName <<  "\"];\n";
      }
      else if(connections[con].b.id == node.id && connections[con].a.flag == false)
      {
        myfile << "    " << connections[con].b.name << " -> " << connections[con].a.name << " [label=" << "\""<< connections[con].type.typeName <<  "\"];\n";
      }
    }
  }
  myfile << "}\n";
  Node::clearAllFlags();

}
bool check_for_direct_connection(Node& a, Node& b)
{
  for(Connection con : connections)
  {
    if((con.a.id == a.id && con.b.id == b.id)||(con.a.id == b.id && con.b.id == a.id))
    {
      return true;
    }
  }
  return false;
}
bool check_graph_full()
{
  bool check;
  for(int i = 0; i < (int)nodes.size(); i++)
  {
    for(int j = 0; j < (int)nodes.size(); j++)
    {
      if( i != j)
      {
        check = false;
        for(Connection con : connections)
        {
          if((con.a.id == nodes[i].id && con.b.id == nodes[j].id)||(con.a.id == nodes[j].id && con.b.id == nodes[i].id))
          {
            check = true;
          }
        }
        if(!check)
        {
          return false;
        }
      }
    }
  }
  return true;
}
bool check_graph_completely_upgraded()
{
  for(Connection con : connections)
  {
    if(con.type.typeName != wires.back().typeName) // assumes that the wireTypes are ordered from worst to best
    {
      return false;
    }
  }
  return true;
}
double simmed_avg_latency( Simulator& sim, int time)// averaged latency
{
  int count = 0;
  double lat = 0;
  for( NetPacket pack : sim.stats.packets)
  {
    if(pack.arrived && pack.sendTime >= time)
    {
      lat += pack.latency;
      count++;
    }
  }
  return lat / count;
}

double simmed_total_error_rate(Simulator& sim, int time) // rate of errors for the entire network
{
  int count = 0;
  double err_count = 0;
  for( NetPacket pack : sim.stats.packets)
  {
      if( pack.sendTime >= time)
      {
          if(!pack.arrived)
          {
              err_count += 1;
          }
          count++;
      }
  }
  return err_count / count;
}

double simmed_throughput(Simulator& sim, double startTime, double endTime) //successful packets per second during the defined period of the simulation
{
  double count = 0;
  for( NetPacket pack : sim.stats.packets)
  {
    if( pack.sendTime >= startTime && (pack.sendTime + pack.latency) <= endTime && pack.arrived)
    {
      count +=1;
    }
  }
  return count / (endTime - startTime);
}
bool upgrade_con(int con)
{
    WireType type = connections[con].type;
    for (int i = 0; i < (int)wires.size();i++)
    {
        if (wires[i].typeName == type.typeName)
        {
            if((int)wires.size() - 1 >  i)
            {
                connections[con].type = wires[i+1];
                return true;
            }
            else{
                return false;
            }
        }
    }
    return false;
}
bool downgrade_con(int con)
{
    WireType type = connections[con].type;
    for (int i = 0; i < (int)wires.size();i++)
    {
        if (wires[i].typeName == type.typeName)
        {
            if(i > 0)
            {
                connections[con].type = wires[i-1];
                return true;
            }
            else{
                return false;
            }
        }
    }
    return false;
}
int add_con(Node& a, Node& b)
{
    a.connect(b,wires[0]);
    return connections.size() -1;
}
bool remove_connection() //do not use for anything not in source code
{
    if(connections[(int)connections.size()-1].a.connectionIndicies[connections[(int)connections.size()-1].a.connectionIndicies.size()-1] == (int)connections.size()-1 && connections[(int)connections.size()-1].b.connectionIndicies[connections[(int)connections.size()-1].b.connectionIndicies.size()-1] == (int)connections.size()-1 )
    {
        connections[(int)connections.size()-1].a.connectionIndicies.pop_back();
        connections[(int)connections.size()-1].b.connectionIndicies.pop_back();
        connections.pop_back();
        return true;
    }
    return false;
}
int find_uprade_target1(Simulator sim)
{
    int upgrade_target = 0;
    for(int i = 0; i < (int)sim.stats.cons.size(); i++)
    {
        if(sim.stats.cons[i].num_denied > sim.stats.cons[upgrade_target].num_denied )
        {
            upgrade_target = i;
        }
    }
    return upgrade_target;
}
int find_uprade_target2(Simulator sim)
{
    int upgrade_target = 0;
    for(int i = 0; i < (int)sim.stats.cons.size(); i++)
    {
        if(sim.stats.cons[i].num_packets > sim.stats.cons[upgrade_target].num_packets )
        {
            upgrade_target = i;
        }
    }
    return upgrade_target;
}
std::vector<int> potential_con1(Simulator sim)
{
    int upgrade_target = 0;
    std::vector<int> out;
    for(int i = 0; i < (int)sim.stats.packets.size(); i++)
    {
        if(sim.stats.packets[i].latency > sim.stats.packets[upgrade_target].latency )
        {
            upgrade_target = i;
        }
    }
    out.push_back(sim.stats.packets[upgrade_target].sourceNode->id);
    out.push_back(sim.stats.packets[upgrade_target].destNode->id);
    return  out;
}
std::vector<int> potential_con2(Simulator sim)
{
    int node1 = 0;
    int node2 = 1;
    int jumps = 0;
    int temp = 0;
    std::vector<int> out;
    for(int i = 0; i < (int)nodes.size(); i++)
    {
        for(int j = 0; j < (int)nodes.size(); j++)
        {
            if ( i != j)
            {
                temp = num_jumps_breadth(nodes[i],nodes[j]);
                if(temp > jumps)
                {
                    jumps = temp;
                    node1 = i;
                    node2 = j;
                }
            }
        }
    }
    out.push_back(node1);
    out.push_back(node2);
    return out;
}
int main(int argc, char **argv)
{
  if(argc != 3) {
    printUsage();
  }

  std::srand(clock());
  readInputFile(argv[1]);
  Graphviz("Input_graph.dot");
  completeNetworkGraph();
  if(prefs.budget < 0) {
    std::cerr << "Could not complete graph within budget. Requests ";
    std::cerr << prefs.originalBudget - prefs.budget << " of " << prefs.originalBudget << ".\n";
  }
  //Graphviz("graph.dot");

  //prefs.budget = 0;
    Simulator sim(nodes, connections, wires, prefs);
    std::vector<double> current_lat;
    std::vector<double> current_err;
    std::vector<double> current_thru;
    sim.simulate();
    std::cout << "simulation complete\n";
    current_lat.push_back( simmed_avg_latency(sim,5));
    current_err.push_back(simmed_total_error_rate(sim,5));
    current_thru.push_back( simmed_throughput(sim, 5, 15));
    current_lat.push_back( 0);
    current_err.push_back(0);
    current_thru.push_back( 0);
    current_lat.push_back( 0);
    current_err.push_back(0);
    current_thru.push_back( 0);
    current_lat.push_back( 0);
    current_err.push_back(0);
    current_thru.push_back( 0);
    current_lat.push_back( 0);
    current_err.push_back(0);
    current_thru.push_back( 0);

  while(prefs.budget > 0 || (check_graph_full() && check_graph_completely_upgraded())) {
      int upgrade_target1 = find_uprade_target1(sim);
      int upgrade_target2 = find_uprade_target2(sim);
      std::vector<int> add_target1 = potential_con2(sim);
      std::vector<int> add_target2 = potential_con1(sim);

      // simulation 1
      bool check = upgrade_con(upgrade_target1);
      if(check)
      {
          sim.simulate();
          current_lat[1]=simmed_avg_latency(sim,5);
          current_err[1] = simmed_total_error_rate(sim,5);
          current_thru[1] = simmed_throughput(sim, 5, 15);
          downgrade_con(upgrade_target1);
      }
      else
      {
          current_lat[1]= -1;
          current_err[1] = 1;
          current_thru[1] = -1;
      }

      //simulation 2
      check = upgrade_con(upgrade_target2);
      if(check)
      {
          sim.simulate();
          current_lat[2]=simmed_avg_latency(sim,5);
          current_err[2] = simmed_total_error_rate(sim,5);
          current_thru[2] = simmed_throughput(sim, 5, 15);
          downgrade_con(upgrade_target2);
      }
      else
      {
          current_lat[2]= -1;
          current_err[2] = 1;
          current_thru[2] = -1;
      }

      //simulation 3

      add_con(nodes[add_target1[0]],nodes[add_target1[1]]);
      sim.simulate();
      current_lat[3]=simmed_avg_latency(sim,5);
      current_err[3] = simmed_total_error_rate(sim,5);
      current_thru[3] = simmed_throughput(sim, 5, 15);
      remove_connection();






    // TODO upgrade network


    if(check_graph_completely_upgraded() && check_graph_full()) break;

    std::cout << simmed_avg_latency(sim,5) << std::endl;
    std::cout << simmed_total_error_rate(sim,5) << std::endl;
    std::cout << simmed_throughput(sim, 0, 1) << std::endl;

    prefs.budget = 0;
  }

  // double dist = net_distance(nodes[0] ,nodes[1]);
  // std::cout << dist << std::endl;

  outputResults(argv[2], sim);
  Graphviz("Output_graph.dot");
  return 0;
}
