/* 
 * File:   NODE_PATH.h
 * Author: leimiche
 *
 * Created on March 6, 2015, 4:19 PM
 */

#ifndef NODE_PATH_H
#define	NODE_PATH_H
#include <vector>
#include "small_node.h"
using namespace std;
class node_path{
public:
    node_path()
    {
    //initial cost and myID approaches infinity
    cost=1000000000;
    visited=false;
    breadcrumb=12345678;
    myID=12345678;
    }
    node_path(vector<small_node>& _outedges,unsigned _myID)
    {
     //initial cost approaches infinity
    cost=1000000000;
    outedges=_outedges;
    visited=false;
    breadcrumb=12345678;
    myID=_myID;
    }
    ~node_path()
    {
        outedges.clear(); 
    }
    double getcost() const
    {       
        return cost;
    }
    void setcost(double _cost)
    {
        cost=_cost;
    }
    vector<small_node> getoutedges()
    {
        return outedges;
    }
    void setoutedges(vector<small_node>& _outedges)
    {
        outedges=_outedges;
    }
    bool getvisited()
    {
        return visited;
    }
    void setvisited(bool _visited)
    {
            visited=_visited;
    }
    unsigned getbreadcrumb(){
        return breadcrumb;
    }
    void setbreadcrumb(unsigned _breadcrumb){
        breadcrumb=_breadcrumb;
    }
    void setmyID(unsigned _myID)
    {
        myID=_myID;
    }
    unsigned getmyID()
    {
        return myID;
    }
    void setheuristic(double _heuristic)
    {
        heuristic=_heuristic;
    }
    double getF(){
        return heuristic+cost;
    }
private:
    unsigned myID;
    double cost;
    double heuristic;
    vector<small_node> outedges;
    bool visited;
    //breadcrumb marks which intersection id brings to this node
    unsigned breadcrumb;
};

class CompareCost
{
public:
    bool operator()(node_path& _lhs, node_path& _rhs){
        if (_lhs.getF()>_rhs.getF())
            return true;
        else
            return false;
    }    
};

class CompareCostinTravellingSalesman
{
public:
    bool operator()(node_path& _lhs, node_path& _rhs){
        if (_lhs.getcost()>_rhs.getcost())
            return true;
        else
            return false;
    }    
};
#endif	/* NODE_PATH_H */

