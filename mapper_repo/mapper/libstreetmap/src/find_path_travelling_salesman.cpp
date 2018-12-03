#include "find_path_travelling_salesman.h"
#define UnderEstimatedSpeed 1667 
vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end,vector<node_path>& Hash_Nodes_T){
    LatLon destination=getIntersectionPosition(intersect_id_end);
    //reset Hash_Nodes_T, such that:
    //1. all nodes are unvisitied
    //2. cost are infinity
    //3. heuristics are set
    for(unsigned i=0;i<Hash_Nodes_T.size();i++){
        Hash_Nodes_T[i].setcost(1000000000);
        Hash_Nodes_T[i].setvisited(false);
        LatLon thisintersect=getIntersectionPosition(i);
        double distance=find_distance_between_two_points(thisintersect,destination);
        double heuristic=distance/UnderEstimatedSpeed;
	Hash_Nodes_T[i].setheuristic(heuristic);
    }
    //declare a new wavefront
    priority_queue<node_path, vector<node_path>, CompareCost>* waveform=new priority_queue<node_path, vector<node_path>, CompareCost>;
    Hash_Nodes_T[intersect_id_start].setcost(0);
    vector<unsigned> shortest;
    //round1 is true now and will be set to false in the helper function
    bool found=findpath(intersect_id_start,1,intersect_id_end,true,waveform,Hash_Nodes_T);
    //push back the segments only when the helper function, findpath, says true
    //return empty vector otherwise
    if(found){
        unsigned shortestpathid=Hash_Nodes_T[intersect_id_end].getbreadcrumb();
        StreetSegmentEnds twoends=getStreetSegmentEnds(shortestpathid);
        unsigned otherend;
        if(twoends.from==intersect_id_end)
            otherend=twoends.to;
        else if (twoends.to==intersect_id_end)
            otherend=twoends.from;
        else
            exit (EXIT_FAILURE);
        while(otherend!=intersect_id_start){
            shortest.push_back(shortestpathid);
            shortestpathid=Hash_Nodes_T[otherend].getbreadcrumb();
            twoends=getStreetSegmentEnds(shortestpathid);
            if(twoends.from==otherend)
            otherend=twoends.to;
            else if (twoends.to==otherend)
            otherend=twoends.from;
        }
        //push back the first element
        shortest.push_back(shortestpathid);
         
    }

   reverse(shortest.begin(),shortest.end());
    delete waveform;
    return shortest;
}

//use a* algorithm
bool findpath(unsigned ID, unsigned segment, unsigned intersect_id_end,
        bool round1, priority_queue<node_path, vector<node_path>, CompareCost>* waveform,vector<node_path>& Hash_Nodes_T) {
//stage 1 is when the destination id has not been the current node (aka ID) yet. Unless waveform has become empty, meaning no possible path,
//there's no way for the function to return. Once the destination id has become the ID, the program is in stage 2. The program returns when
//this ID's cost exceeds the destination's cost
    do {
        node_path cost;
        unsigned nextlookupID;
        if (!waveform->empty()&&!round1) {            
            cost = waveform->top();
            nextlookupID = cost.getmyID();
            while (Hash_Nodes_T[nextlookupID].getvisited()&&!waveform->empty()) {
                waveform->pop();
                cost = waveform->top();
                nextlookupID = cost.getmyID();
            }
            if (!waveform->empty()) {
                ID = nextlookupID;
                segment = Hash_Nodes_T[nextlookupID].getbreadcrumb();
            }
        }        
	//when the current node (ID) equals intersect_id_end, it means that the destination node's cost
        //is lowest so 1. a valid path is found and 2.the program can return
	if (ID == intersect_id_end)
            return true;
        //This node is visited
        Hash_Nodes_T[ID].setvisited(true);
        vector<small_node> outedges=Hash_Nodes_T[ID].getoutedges();
        unsigned size = outedges.size();
        for (unsigned i = 0; i < size; i++) {
            double penalty = 0;
            unsigned visitingID = outedges[i].interID;
            unsigned source = outedges[i].segID;
            if (!Hash_Nodes_T[visitingID].getvisited()) {
                //if parent node's cost+weight+potentially 15s penaly<current cost,
                //modify current cost
                if (!round1 && (getStreetSegmentStreetID(source) != getStreetSegmentStreetID(segment)))
                    penalty = 0.25;
                //G is movement cost+parent cost+penalty
                double G = Hash_Nodes_T[ID].getcost()+outedges[i].weight + penalty;
                if (Hash_Nodes_T[visitingID].getcost() > G) {
                    Hash_Nodes_T[visitingID].setcost(G);
                    Hash_Nodes_T[visitingID].setbreadcrumb(source);
                    waveform->push(Hash_Nodes_T[visitingID]);
                }
            }
        }
        round1=false;
    }    
    while (!waveform->empty());
    //when wavefront is empty and the program has not returned, it means no legal path
    //can be found, therefore; return false
        return false;
}
