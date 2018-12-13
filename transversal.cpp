#include "types.h"
#include "transversal.h"

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <functional>
#include <cmath>

double real_distance(Node& a, Node& b)
{
    return std::sqrt( (a.xLoc-b.xLoc)*(a.xLoc-b.xLoc) + (a.yLoc-b.yLoc)*(a.yLoc-b.yLoc));
}
double net_distance(Node& a, Node& b)// assumes network is complete
{
    a.flag=true;
    bool check;
    std::vector<std::reference_wrapper<Node>> list;
    double dist = 0;
    if(a.id == b.id)
    {
        Node::clearAllFlags();
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
            check = net_distance_deep(dist, n, b);
            if(check)
            {
                Node::clearAllFlags();
                return dist + real_distance(a,b);
            }
        }
    }
    Node::clearAllFlags();
    return 0;
}
bool net_distance_deep(double & dist,Node &current, Node &target)
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
        else if (connections[con].b.id == current.id && connections[con].a.flag == false)
        {
            list.push_back(connections[con].a);
        }
    }
    dist_sort(target,list);
    for(Node & n: list)
    {
        if (!n.flag)
        {
            check = net_distance_deep(dist, n, target);
            if (check)
            {
                dist += real_distance(current,n);
                return true;
            }
        }

    }
    return false;
}
//replacing the vectors of nodes with lists of node&
void dist_sort(Node& target, std::vector<std::reference_wrapper<Node>> & list)
{
    bool done = false;
    int count;
    double dist1;
    double dist2;
    if(list.empty())
    {
        return;
    }
    Node temp = list[0];
    while(!done)
    {
        done =true;
        count = 0;
        for(int i = 0; i < (int)list.size(); i++)
        {
            if(count > 0)
            {
                dist1 = real_distance(target,temp);
                dist2 = real_distance(target,list[i]);
                if(abs(dist2) < abs(dist1))
                {
                    std::swap(list[i],list[i-1]);
                    done = false;
                }
            }
            temp = list[i];
            count++;
        }
    }
    //return list;
}

int num_jumps(Node& a, Node& b)
{
    a.flag=true;
    bool check;
    std::vector<std::reference_wrapper<Node>> list;
    int dist = 0;
    if(a.id == b.id)
    {
        Node::clearAllFlags();
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
    Node::clearAllFlags();
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
std::vector<int> connection_jumps(Node& a, Node& b)
{
    a.flag=true;
    bool check;
    std::vector<std::reference_wrapper<Node>> list;
    std::vector<int> outlist;
    if(a.id == b.id)
    {
        return outlist;
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
            check = connection_jumps_deep(outlist, n, b);
            if(check)
            {
                for(int  con: n.connectionIndicies)
                {
                    if( connections[con].a.id == a.id && connections[con].b.id == n.id )
                    {
                        outlist.push_back(con);
                    }
                    else if( connections[con].b.id == a.id && connections[con].a.id == n.id)
                    {
                        outlist.push_back(con);
                    }
                }
                Node::clearAllFlags();
                return outlist;
            }
        }
    }
    Node::clearAllFlags();
    return outlist;
}
bool connection_jumps_deep(std::vector<int> & outlist, Node &current, Node &target)
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
            check = connection_jumps_deep(outlist, n, target);
            if (check)
            {
                for(int  con: current.connectionIndicies)
                {
                    if( connections[con].a.id == current.id && connections[con].b.id == n.id )
                    {
                        outlist.push_back(con);
                    }
                    else if( connections[con].b.id == current.id && connections[con].a.id == n.id)
                    {
                        outlist.push_back(con);
                    }
                }
                return true;
            }
        }

    }
    return false;
}
int num_jumps_breadth(Node& a, Node& b) //returns the minimum number of jumps neccessary between 2 nodes
{
    std::vector<std::vector<std::reference_wrapper<Node>>> queue;
    std::vector<std::reference_wrapper<Node>> temp;
    temp.push_back(a);
    queue.push_back(temp);
    for(int i = 0; i < (int)queue.size();i++)
    {
        queue[i].back().get().flag = true;
        if(queue[i].back().get().id == b.id)
        {
            Node::clearAllFlags();
            return (int)queue[i].size() - 1;
        }
        for(int con: queue[i].back().get().connectionIndicies)
        {
            temp = queue[i];
            if(connections[con].a.id == queue[i].back().get().id && connections[con].b.flag == false)
            {
                temp.push_back(connections[con].b);
            }
            else if(connections[con].b.id == queue[i].back().get().id && connections[con].a.flag == false)
            {
                temp.push_back(connections[con].a);
            }
            queue.push_back(temp);
        }
    }
    Node::clearAllFlags();
    std::vector<std::reference_wrapper<Node>> empty;
    return (int)empty.size();
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
std::vector<std::reference_wrapper<Node>> connection_jumps_path(Node& a, Node& b)
{
    //a.flag=true;
    //bool check;
    std::vector<std::vector<std::reference_wrapper<Node>>> queue;
    std::vector<std::reference_wrapper<Node>> temp;
    temp.push_back(a);
    queue.push_back(temp);
    for(int i = 0; i < (int)queue.size();i++)
    {
        queue[i].back().get().flag = true;
        if(queue[i].back().get().id == b.id)
        {
            Node::clearAllFlags();
            return queue[i];
        }
        for(int con: queue[i].back().get().connectionIndicies)
        {
            temp = queue[i];
            if(connections[con].a.id == queue[i].back().get().id && connections[con].b.flag == false)
            {
                temp.push_back(connections[con].b);
            }
            else if(connections[con].b.id == queue[i].back().get().id && connections[con].a.flag == false)
            {
                temp.push_back(connections[con].a);
            }
            queue.push_back(temp);
        }
    }
    Node::clearAllFlags();
    std::vector<std::reference_wrapper<Node>> empty;
    return empty;
}
std::vector<int> path_to_con(std::vector<std::reference_wrapper<Node>> path) {
    std::vector<int> connect;
    if (path.size() < 2)
    {
        return connect;
    }
    for (int i = 0; i < (int) path.size() - 1; i++) {
        for (int con: path[i].get().connectionIndicies) {

            if (connections[con].a.id == path[i + 1].get().id) {
                connect.push_back(con);
            } else if (connections[con].b.id == path[i + 1].get().id) {
                connect.push_back(con);
            }
        }
    }
    return connect;
}
double latency(Node& a, Node& b)
{
    std::vector<std::reference_wrapper<Node>> path;
    path = connection_jumps_path(a, b);
    if (path.size() < 2)
    {
        return 0;
    }
    //double dist;
    double latency;
    std::vector<int> cons;
    cons = path_to_con(path);
    for (int i = 0; i < (int) path.size(); i++)
    {
        latency += 1 / path[i].get().routeRate;
    }
    for (int i = 0; i < (int) cons.size(); i++)
    {
        latency += connections[cons[i]].travelTime;
    }
    return latency;
}
double avg_latency()
{
    double count;
    double lat;
    for(int i = 0; i < (int)nodes.size(); i++)
    {
        for(int j = 0; j < (int)nodes.size(); j++)
        {
            if( i != j)
            {
                count ++;
                lat += latency(nodes[i],nodes[j]);
            }
        }
    }
    return lat / count;

}
std::vector<std::reference_wrapper<Node>> find_alt_path(Node& a, Node& b, std::vector<int> blocked)
{
    //a.flag=true;
    //bool check;
    std::vector<std::vector<std::reference_wrapper<Node>>> queue;
    std::vector<std::reference_wrapper<Node>> temp;
    temp.push_back(a);
    queue.push_back(temp);
    for(int i = 0; i < (int)queue.size();i++)
    {
        queue[i].back().get().flag = true;
        if(queue[i].back().get().id == b.id)
        {
            Node::clearAllFlags();
            return queue[i];
        }
        for(int con: queue[i].back().get().connectionIndicies)
        {
            temp = queue[i];
            if(connections[con].a.id == queue[i].back().get().id && connections[con].b.flag == false && std::find(blocked.begin(), blocked.end(), con) == blocked.end())
            {
                temp.push_back(connections[con].b);
            }
            else if(connections[con].b.id == queue[i].back().get().id && connections[con].a.flag == false && std::find(blocked.begin(), blocked.end(), con) == blocked.end())
            {
                temp.push_back(connections[con].a);
            }
            queue.push_back(temp);
        }
    }
    Node::clearAllFlags();
    std::vector<std::reference_wrapper<Node>> empty;
    return empty;
}