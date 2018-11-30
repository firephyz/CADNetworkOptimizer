#include "types.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <libxml2/libxml/parser.h>
#include <vector>
#include <functional>

// Populated by input file
std::vector<Node> nodes;
std::vector<Connection> connections;
std::vector<WireType> wires; // sorted by cost
std::unordered_map<std::string, Node *> hashed_nodes;
int num_jumps(Node& a, Node& b);
bool num_jumps_deep(int & dist,Node &current, Node &target);
int num_jumps_breadth(Node& a, Node& b);
bool num_jumps_breadth_deep(std::vector<std::reference_wrapper<Node>> & list,Node& current, Node& target);


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
    double xLoc = std::atof((const char *)xmlGetAttribute(child, "x")->children->content);
    double yLoc = std::atof((const char *)xmlGetAttribute(child, "y")->children->content);
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
    double errorRate = std::atof((const char *)xmlGetAttribute(child, "errorRate")->children->content);

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

std::string statsToXML() {
  std::string result = "";
  result += "<Statistics";
  result += "/>";
  return result;
}

void outputResults(char * fileName) {
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
  file << "\t" << statsToXML() << std::endl;
  file << "</NetworkDefinition>\n";

  file.close();
}

int
num_jumps(Node& a, Node& b)
{
  a.flag=true;
  bool check;
  std::vector<std::reference_wrapper<Node>> list;
  int dist = 0;
  if(a.id == b.id)
  {
    return 0;
  }
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
  dist_sort(b,list);
  for(Node & n: list)
  {
    if(!n.flag)
    {
      check = num_jumps_deep(dist, n, b);
      if(check)
      {
        Node::clearAllFlags();
        return dist + 1;
      }
    }
  }
  return -1;
}
bool num_jumps_deep(int & dist,Node &current, Node &target)
{
  if(current.id == target.id)
  {
    return true;
  }
  current.flag = true;
  bool check;
  std::vector<std::reference_wrapper<Node>> list;
  for(int con: current.connectionIndicies)
  {
    if(connections[con].a.id == current.id && connections[con].b.flag == false)
    {
      list.push_back(connections[con].b);
    }
    else if(connections[con].b.id == current.id && connections[con].a.flag == false)
    {
      list.push_back(connections[con].a);
    }
  }
  dist_sort(target,list);
  for(Node & n: list)
  {
    if (!n.flag)
    {
      check = num_jumps_deep(dist, n, target);
      if (check)
      {
        dist += 1;
        return true;
      }
    }

  }
  return false;
}
int num_jumps_breadth(Node& a, Node& b) //returns the minimum number of jumps neccessary between 2 nodes
{
  a.flag=true;
  bool check;
  std::vector<std::reference_wrapper<Node>> list;
  int dist = 1;
  if(a.id == b.id)
  {
    return 0;
  }
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
  int levelEnd =list.size();
  for(int i = 0; i < (int)list.size(); i++)
  {
    if(i > levelEnd)
    {
      levelEnd = list.size();
      dist++;
    }
    if(!list[i].get().flag)
    {
      check = num_jumps_breadth_deep(list,list[i],b);
      if(check)
      {
        Node::clearAllFlags();
        return dist;
      }
    }
  }
  return -1;
}
bool num_jumps_breadth_deep(std::vector<std::reference_wrapper<Node>> & list,Node& current, Node& target)
{
  if(current.id == target.id)
  {
    return true;
  }
  current.flag = true;
  for(int con: current.connectionIndicies)
  {
    if(connections[con].a.id == current.id && connections[con].b.flag == false)
    {
      list.push_back(connections[con].b);
    }
    else if(connections[con].b.id == current.id && connections[con].a.flag == false)
    {
      list.push_back(connections[con].a);
    }
  }
  return false;
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
      else if(num_jumps(*node, *group[0]) != -1) {
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
      int distance = num_jumps(*node, *target);
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
  for(uint i = 0; i < midNodes.size() - 1; ++i) {
    if(i == midNodes.size() - 1) {
      midNodes[i]->connect(*midNodes[0], wire);
    }
    else {
      midNodes[i]->connect(*midNodes[i + 1], wire);
    }
  }
}

int main(int argc, char **argv)
{
  if(argc != 3) {
    printUsage();
  }

  readInputFile(argv[1]);

  completeNetworkGraph();
  int dist = num_jumps_breadth(nodes[0],nodes[3]);
  std::cout << dist << std::endl;
  outputResults(argv[2]);

  for(const Node& node : nodes) {
    std::cout << node << std::endl;
  }

  for(const WireType& wire : wires) {
    std::cout << wire.typeName << std::endl;
    std::cout << wire.cost << std::endl;
    std::cout << wire.bandwidth << std::endl;
    std::cout << wire.errorRate << std::endl;
  }

  for(const Connection& connection : connections) {
    std::cout << connection << std::endl;
  }

  return 0;
}