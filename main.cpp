#include "types.h"

#include <iostream>
#include <fstream>
#include <string>
#include <libxml2/libxml/parser.h>

// Populated by input file
std::vector<Node> nodes;
std::vector<Connection> connections;
std::vector<WireType> wires;
std::unordered_map<std::string, Node *> hashed_nodes;

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
    WireType& wire = getWireTypeByName(type);

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

void completeNetworkGraph()
{
  // TODO
}

int main(int argc, char **argv)
{
  if(argc != 3) {
    printUsage();
  }

  readInputFile(argv[1]);

  completeNetworkGraph();
  double dist = net_distance(nodes[0] ,nodes[1]);
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