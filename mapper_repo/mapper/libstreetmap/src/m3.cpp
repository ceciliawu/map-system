
#include "m3.h"
extern vector<node_path> Hash_Nodes;
using namespace std;
#define UnderEstimatedSpeed 1667 
vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end){
    LatLon destination=getIntersectionPosition(intersect_id_end);
    //reset Hash_Nodes, such that:
    //1. all nodes are unvisitied
    //2. cost are infinity
    //3. heuristics are set
    for(unsigned i=0;i<Hash_Nodes.size();i++){
        Hash_Nodes[i].setcost(1000000000);
        Hash_Nodes[i].setvisited(false);
        LatLon thisintersect=getIntersectionPosition(i);
        double distance=find_distance_between_two_points(thisintersect,destination);
        double heuristic=distance/UnderEstimatedSpeed;
	Hash_Nodes[i].setheuristic(heuristic);
    }
    //declare a new wavefront
    priority_queue<node_path, vector<node_path>, CompareCost>* waveform=new priority_queue<node_path, vector<node_path>, CompareCost>;
    Hash_Nodes[intersect_id_start].setcost(0);
    vector<unsigned> shortest;
    //round1 is true now and will be set to false in the helper function
    bool found=findpath(intersect_id_start,1,intersect_id_end,true,waveform);
    //push back the segments only when the helper function, findpath, says true
    //return empty vector otherwise
    if(found){
        unsigned shortestpathid=Hash_Nodes[intersect_id_end].getbreadcrumb();
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
            shortestpathid=Hash_Nodes[otherend].getbreadcrumb();
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
        bool round1, priority_queue<node_path, vector<node_path>, CompareCost>* waveform) {
//stage 1 is when the destination id has not been the current node (aka ID) yet. Unless waveform has become empty, meaning no possible path,
//there's no way for the function to return. Once the destination id has become the ID, the program is in stage 2. The program returns when
//this ID's cost exceeds the destination's cost
    do {
        node_path cost;
        unsigned nextlookupID;
        if (!waveform->empty()&&!round1) {            
            cost = waveform->top();
            nextlookupID = cost.getmyID();
            while (Hash_Nodes[nextlookupID].getvisited()&&!waveform->empty()) {
                waveform->pop();
                cost = waveform->top();
                nextlookupID = cost.getmyID();
            }
            if (!waveform->empty()) {
                ID = nextlookupID;
                segment = Hash_Nodes[nextlookupID].getbreadcrumb();
            }
        }        
	//when the current node (ID) equals intersect_id_end, it means that the destination node's cost
        //is lowest so 1. a valid path is found and 2.the program can return
	if (ID == intersect_id_end)
            return true;
        //This node is visited
        Hash_Nodes[ID].setvisited(true);
        vector<small_node> outedges=Hash_Nodes[ID].getoutedges();
        unsigned size = outedges.size();
        for (unsigned i = 0; i < size; i++) {
            double penalty = 0;
            unsigned visitingID = outedges[i].interID;
            unsigned source = outedges[i].segID;
            if (!Hash_Nodes[visitingID].getvisited()) {
                //if parent node's cost+weight+potentially 15s penaly<current cost,
                //modify current cost
                if (!round1 && (getStreetSegmentStreetID(source) != getStreetSegmentStreetID(segment)))
                    penalty = 0.25;
                //G is movement cost+parent cost+penalty
                double G = Hash_Nodes[ID].getcost()+outedges[i].weight + penalty;
                if (Hash_Nodes[visitingID].getcost() > G) {
                    Hash_Nodes[visitingID].setcost(G);
                    Hash_Nodes[visitingID].setbreadcrumb(source);
                    waveform->push(Hash_Nodes[visitingID]);
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

double compute_path_travel_time(const std::vector<unsigned>& path){
        clock_t startTime = clock (); 
    double time = 0;
    unsigned stID_curr,stID_next;
    
    for (unsigned i=0; i<path.size();i++){
        
        time =time+ find_segment_travel_time(path[i]);              //time in minutes
        
        if (i != path.size()-1){
           
           stID_curr = getStreetSegmentStreetID(path[i]);
           stID_next = getStreetSegmentStreetID(path[i+1]);
           //if the two connected segments in the path do that belong to the
           //same street, we need to add the penalty time to the total travel time
           if (stID_curr != stID_next)
               time =time + 0.25;                                    //add 15 secs if streets are different
        }
        //push back the first element

    }
    clock_t currentTime = clock ();
    cout<<"Time used is "<< ((float) (currentTime-startTime))/CLOCKS_PER_SEC<<"seconds"<<endl;
    return time;
}



vector<unsigned> find_path_to_point_of_interest (unsigned 
                   intersect_id_start, std::string point_of_interest_name){
    vector<unsigned> Inter=find_closest_intersection_helper (point_of_interest_name,intersect_id_start);
    //if the vector has a size of zero, it means invalid point_of_interest_name
    //or cannot get there. Either case, simply return
    if(Inter.size()==0)
        return Inter;   
    vector<unsigned> temp_path=find_path_between_intersections(intersect_id_start,Inter[0]);
    //assume the shortest path is the first element, change it when we find
    //it is not later
    vector<unsigned> closest_path_to_POI=temp_path;
    double time=compute_path_travel_time(temp_path);
    for(unsigned i=1;i<Inter.size();i++)
    { 
        temp_path=find_path_between_intersections(intersect_id_start,Inter[i]);
        double temp_time=compute_path_travel_time(temp_path);
        if(temp_time<time&&temp_time!=0)
        {
            time=temp_time;
            closest_path_to_POI=temp_path;
        }
    }
    return closest_path_to_POI;
}
    
