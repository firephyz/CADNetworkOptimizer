#ifndef TRANSVERSAL_LIB
#define TRANSVERSAL_LIB

#include "types.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <functional>

int num_jumps(Node& a, Node& b);
bool num_jumps_deep(int & dist,Node &current, Node &target);
int num_jumps_breadth(Node& a, Node& b);
bool num_jumps_breadth_deep(std::vector<std::reference_wrapper<Node>> & list,Node& current, Node& target);
std::vector<int> connection_jumps(Node& a, Node& b);
bool connection_jumps_deep(std::vector<int> & outlist,Node &current, Node &target);
std::vector<std::reference_wrapper<Node>> connection_jumps_path(Node& a, Node& b);
std::vector<int> path_to_con(std::vector<std::reference_wrapper<Node>> path);


#endif