#include "m4.h"
extern vector<node_path> Hash_Nodes;
//two greedy algorithm so far so two threads
#define NUM_THREADS     3
#define TIME_LIMIT      30
unordered_map <unsigned,vector<pair<double,unsigned>>> travel_direct_distance;
//Common to all greedy algorithms
//Any algorithms can load the segments path into this hash_table whenever calling find_paths.
//Eventually, after choosing the path in the main function(traveling_salesman), use this hash_table
//to push_back the vector
unordered_map<unsigned,vector<vector<unsigned>>>segpaths;

vector<unsigned> traveling_salesman(vector<unsigned> intersections_to_traverse,vector<unsigned> depot_locations){
    clock_t startTime = clock (); 
    //build up the hash table that contains the vector of distance to others
    //also setting up the (global) hash_table for segpaths
    //also setting up the (local) hash_table for IDtoIndex;
    unsigned inter_size=intersections_to_traverse.size();
    vector<vector<unsigned>> row_segpaths;
    vector<unsigned> a;
    for(unsigned i=0; i<inter_size;i++){
        row_segpaths.push_back(a);
    }
    unordered_map<unsigned,unsigned> IDtoIndex; 
    for(unsigned i=0; i<inter_size;i++)
    {   unsigned my_inter_id=intersections_to_traverse[i];
        LatLon my_pos;
        my_pos.lat=getIntersectionPosition(my_inter_id).lat;
        my_pos.lon=getIntersectionPosition(my_inter_id).lon;
        vector<pair<double,unsigned>> my_direct_dist;
        for(unsigned j=0;j<inter_size;j++)
        {  pair<double,unsigned> temp;
           double temp_dist=(find_distance_between_two_points(my_pos,getIntersectionPosition(intersections_to_traverse[j])));
           if(temp_dist!=0)
           {
           temp=make_pair(temp_dist,intersections_to_traverse[j]);
           my_direct_dist.push_back(temp);
           }
           if(!my_direct_dist.empty())
           sort(my_direct_dist.begin(),my_direct_dist.end());
        }
        travel_direct_distance.emplace(my_inter_id,my_direct_dist);
        segpaths.emplace(intersections_to_traverse[i],row_segpaths);
        IDtoIndex.emplace(intersections_to_traverse[i],i);
    }
    
    /*for(unsigned i=0;i<inter_size;i++)
    {   cout<<"next:"<<endl;
        unsigned test=intersections_to_traverse[i];
        vector<pair<double,unsigned>> temp=travel_direct_distance.at(test);
        for(unsigned j=0;j<temp.size();j++)
        cout<<temp[j].second<<"    "<<temp[j].first<<endl;
        
    }*/
 
    pthread_t threads[NUM_THREADS];
    struct thread_data td[NUM_THREADS];
    pthread_attr_t attr;
    void *status;
    // Initialize and set thread joinable
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc;
    int i;
    //Building up td[i].Hash_Nodes_T
    for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
        // store node_path into each node ID(equivalent to intersection id)
        vector <unsigned> adjStreetSegment = find_intersection_street_segments(i);
        vector <small_node> ListofNodes;
        //for loop to create the vector of outedges
        //cout<<adjStreetSegment.size()<<endl;
        for (unsigned l = 0; l < adjStreetSegment.size(); l++) {
            unsigned SegmentID = adjStreetSegment[l];
            //cout<<SegmentID<<"     ";
            unsigned otherInterID;
            bool insert;
            if (getStreetSegmentEnds(SegmentID).from == i) {
                otherInterID = getStreetSegmentEnds(SegmentID).to;
                insert = true;
            } else if (getStreetSegmentEnds(SegmentID).to == i) {
                otherInterID = getStreetSegmentEnds(SegmentID).from;
                insert = true;
            } else
                insert = false;
            if (insert) {
                bool connected = false;
                if (!getStreetSegmentOneWay(SegmentID))
                    connected = true; 
                else if ((getStreetSegmentEnds(SegmentID).from == i)&&(getStreetSegmentEnds(SegmentID).to == otherInterID))
                    connected = true; 
                if (connected)
                {
                    small_node temp;
                    temp.segID = SegmentID;
                    temp.interID = otherInterID;
                    temp.weight = find_segment_travel_time(SegmentID);
                    ListofNodes.push_back(temp);
                    // cout<<temp.segID<<"   "<<temp.interID<<"   "<<temp.weight<<endl;

                }
            }
        }
        //node_path* p = new node_path(ListofNodes, i);
        td[0].Hash_Nodes_T.push_back(node_path(ListofNodes, i));
        td[1].Hash_Nodes_T.push_back(node_path(ListofNodes, i));
        td[2].Hash_Nodes_T.push_back(node_path(ListofNodes, i));
        //current.setoutedges(ListofNodes);

        ListofNodes.clear();

    }
    for( i=0; i < NUM_THREADS; i++){
        td[i].intersections_to_traverse=intersections_to_traverse;
        td[i].depot_locations=depot_locations;
        td[i].starttime=startTime;
        if(i==0)
            rc = pthread_create(&threads[i], NULL,
                          greedy_algorithm1, (void *)&td[i]);
        else if (i==1)
            rc = pthread_create(&threads[i], NULL,
                          greedy_algorithm2, (void *)&td[i]);
        else
            rc = pthread_create(&threads[i], NULL,
                          genetic_algorithm, (void *)&td[i]);
        if (rc){
         cout << "Error:unable to create thread," << rc<< endl;
         exit(-1);
        }
    }

   for( i=0; i < NUM_THREADS; i++ ){
      rc= pthread_join(threads[i], &status);
      if (rc){
         cout << "Error:unable to join," << rc << endl;
         exit(-1);
      }
   }
 
   /*********After all greedy functions return, do selection here**************/
   vector<unsigned> chosen_path;
   //genetic algorithm calculates the traveling time wrongly
   
   /*if((td[0].traveltime<td[1].traveltime)&&(td[0].traveltime<td[2].traveltime))
       chosen_path=td[0].path;
   else if((td[1].traveltime<td[0].traveltime)&&(td[1].traveltime<td[2].traveltime))
       chosen_path=td[1].path;
   else
   {
       chosen_path=td[2].path;
   }*/
   if(td[0].traveltime<td[1].traveltime)
        chosen_path=td[0].path;
   else
       chosen_path=td[1].path;
       
 
   /***************************************************************************/
    
   /********Process the chosen path further************************************/
   /*********Include the closest depots locations at front and back*************/
   add_depo(chosen_path,depot_locations,startTime);
   /***************Finally, return the paths of street segments****************/
   vector<unsigned> path_of_segments;
   int chosen_path_size=chosen_path.size();
   vector<unsigned> temp_path_seg;
   unsigned temp_path_seg_size;
   //The following method needs debugging
   //find path between start depot and the first delivery location
   path_of_segments=find_path_between_intersections(chosen_path[0],chosen_path[1]);
   if(path_of_segments.size()==0){
       cout<<"can't link start depo to start delivery"<<endl;
   }
   /*for(int i=0;i<chosen_path_size;i++){
       cout<<chosen_path[i]<<" \n";
   }
   cout<<endl;*/
   //find path between the delivery locations
   for(int i=1;i<chosen_path_size-2;i++){
       unsigned Key=IDtoIndex.at(chosen_path[i+1]);
       if((segpaths.at(chosen_path[i]))[Key].empty()){
           temp_path_seg=find_path_between_intersections(chosen_path[i],
                                           chosen_path[i+1]);
           temp_path_seg_size=temp_path_seg.size();
           for(unsigned j=0;j<temp_path_seg_size;j++){
               path_of_segments.push_back(temp_path_seg[j]);
           }
       }
       else{
           temp_path_seg_size=(segpaths.at(chosen_path[i]))[Key].size();
           for(unsigned j=0;j<temp_path_seg_size;j++){
               path_of_segments.push_back((segpaths.at(chosen_path[i]))[Key][j]);
           }
       }
   }
  //find path between the last delivery location and the end depot
   temp_path_seg=find_path_between_intersections(chosen_path[chosen_path_size-2],
                                          chosen_path[chosen_path_size-1]);
   temp_path_seg_size=temp_path_seg.size();
   if(temp_path_seg_size==0)
       cout<<"can't link end depo to lost delivery"<<endl;
   for(unsigned j=0;j<temp_path_seg_size;j++){
       path_of_segments.push_back(temp_path_seg[j]);
   }
   
   /*for(int i=0;i<chosen_path_size-1;i++){
           temp_path_seg=find_path_between_intersections(chosen_path[i],
                                           chosen_path[i+1]);
           temp_path_seg_size=temp_path_seg.size();
           for(unsigned j=0;j<temp_path_seg_size;j++){
               path_of_segments.push_back(temp_path_seg[j]);
           }
   }*/
   
   /***************************************************************************/

    
   //clear the unordered_map travel_direct_distance
   for(auto iter=travel_direct_distance.begin();
            iter!=travel_direct_distance.end();iter++) 
        (iter->second).clear();
    travel_direct_distance.clear();
    
    //clear the segpaths
    for(auto iter=segpaths.begin();iter!=segpaths.end();iter++){
        unsigned size=iter->second.size();
        for(unsigned i=0;i<size;i++){
            (iter->second)[i].clear();
        }
        (iter->second).clear();
    }
    segpaths.clear();
    
    //clock_t currentTime = clock ();
    //cout<<"Time used is "<< ((float) (currentTime-startTime))/CLOCKS_PER_SEC<<"seconds"<<endl;
    return path_of_segments;
    pthread_exit(NULL);

}


void* greedy_algorithm2(void *threadarg)
{   //get data
    struct thread_data *data;
    data=(struct thread_data*) threadarg;
    vector<unsigned> intersections_to_traverse=data->intersections_to_traverse;
    vector<unsigned> depot_locations=data->depot_locations;
    data->traveltime=0;
    set<unsigned> delivery_order;
    double min_dist=100000000000;
    unsigned start_delivery=0;
    unsigned inter_size=intersections_to_traverse.size();
    unsigned depot_size=depot_locations.size();
    vector<unsigned> path;
    unordered_map<unsigned,vector<double>>paths;
    vector<double> row_paths;
    unordered_map<unsigned,unsigned> IDtoIndex;
    //for loop to find the closest intersection to either one of the depots
    for (unsigned i=0;i<inter_size;i++)
    {   LatLon inter_pos;
        inter_pos.lat=getIntersectionPosition(intersections_to_traverse[i]).lat;
        inter_pos.lon=getIntersectionPosition(intersections_to_traverse[i]).lon;
        for(unsigned j=0;j<depot_size;j++)
        {   unsigned depot=depot_locations[j];
            double temp=find_distance_between_two_points(inter_pos,getIntersectionPosition(depot));
            if(temp<min_dist)
            {
                min_dist=temp;
                start_delivery=intersections_to_traverse[i];
            }
        }
        row_paths.push_back(0);
        IDtoIndex.emplace(intersections_to_traverse[i],i);
    }
    
    for(unsigned i=0;i<inter_size;i++){
        paths.emplace(intersections_to_traverse[i],row_paths);
    }
    delivery_order.insert(start_delivery);
    path.push_back(start_delivery);
    while(delivery_order.size()<intersections_to_traverse.size())
    {
        vector<pair<double,unsigned>> this_dist=travel_direct_distance.at(start_delivery);
        unsigned j=0;
        while(delivery_order.find(this_dist[j].second)!=delivery_order.end())
            j++;
        delivery_order.insert(this_dist[j].second);
        path.push_back(this_dist[j].second);
        start_delivery=this_dist[j].second;
    }
    
        data->path=path;
    unsigned path_size=path.size();
    for(unsigned i=0;i<path_size-1;i++){
        unsigned key=IDtoIndex.at(path[i+1]);
        vector<unsigned> temp_path=find_path_between_intersections(path[i],path[i+1],data->Hash_Nodes_T);
        double temp_travel_time=compute_path_travel_time(temp_path);
        data->traveltime=data->traveltime+temp_travel_time;
        (paths.at(path[i]))[key]=temp_travel_time;
    }
    twoopt(data,paths,IDtoIndex,intersections_to_traverse);
    pthread_exit(NULL);
}


void* greedy_algorithm1(void *threadarg){
    //recover the data
    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    vector<unsigned> intersections_to_traverse=my_data->intersections_to_traverse;
    vector<unsigned> depot_locations=my_data->depot_locations;
    unordered_map<unsigned,bool> expansion;
    unordered_map<unsigned,unsigned> IDtoIndex;
    //the following table storing path time
    unordered_map<unsigned,vector<double>>paths;
    vector<double> row_paths;
    unsigned num_inter=intersections_to_traverse.size();
    //count counts how many intersections have not been found during the
    //expansion so that the expansion exists when count=0
    unsigned count=num_inter;
    double totallat=0;
    double totallon=0;
    double avelat,avelon;
    vector<LatLon> position;
    for(unsigned i=0;i<num_inter;i++){
        position.push_back(getIntersectionPosition(intersections_to_traverse[i]));
        totallat+=position[i].lat;
        totallon+=position[i].lon;
        //building up the expansion
        expansion.emplace(intersections_to_traverse[i],false);
        IDtoIndex.emplace(intersections_to_traverse[i],i);
        row_paths.push_back(0);
    }
    for(unsigned i=0;i<num_inter;i++){
        paths.emplace(intersections_to_traverse[i],row_paths);
    }
    avelat=totallat/num_inter;
    avelon=totallon/num_inter;
    LatLon avelatlon(avelat,avelon);
    unsigned maxindex=0;
    double maxdistance=find_distance_between_two_points(position[0],avelatlon);
    double tempdistance;
    for(unsigned i=1;i<num_inter;i++){
        tempdistance=find_distance_between_two_points(position[i],avelatlon);
        if(tempdistance>maxdistance){
            maxindex=i;
            maxdistance=tempdistance;
        }
    }
    //Expansion starts
    //set up the expansion
    unsigned ID=intersections_to_traverse[maxindex];
    unsigned previousID=ID;
    bool round1=true;
    priority_queue<node_path, vector<node_path>, CompareCostinTravellingSalesman> waveform;
    unsigned segment=0;
    for(unsigned i=0;i<Hash_Nodes.size();i++){
        my_data->Hash_Nodes_T[i].setcost(1000000000);
        my_data->Hash_Nodes_T[i].setvisited(false);
    }
    my_data->Hash_Nodes_T[ID].setcost(0);
    do {
        node_path cost;
        unsigned nextlookupID;
        //if the ID is in the traversing vector and it's not been found yet
	if (expansion.find(ID)!=expansion.end()&&!expansion.at(ID)){
            if(previousID!=ID){
                unsigned key2=IDtoIndex.at(ID);
                (paths.at(previousID))[key2]=my_data->Hash_Nodes_T.at(ID).getcost();
                previousID=ID;
            }
            expansion.find(ID)->second=true;
            my_data->path.push_back(ID);
            //cout<<"count: "<<count<<endl;
            count=count-1;
            /*********Restart*************/
            for(unsigned i=0;i<my_data->Hash_Nodes_T.size();i++){
                my_data->Hash_Nodes_T[i].setcost(1000000000);
                my_data->Hash_Nodes_T[i].setvisited(false);
            }
            my_data->Hash_Nodes_T[ID].setcost(0);
            while(!waveform.empty())
                waveform.pop();
            round1=true;
            /*****************************/
        }
        if (!round1&&!waveform.empty()) {            
            cost = waveform.top();
            nextlookupID = cost.getmyID();
            while (my_data->Hash_Nodes_T[nextlookupID].getvisited()&&!waveform.empty()) {
                waveform.pop();
                cost = waveform.top();
                nextlookupID = cost.getmyID();
            }
            if (!waveform.empty()) {
                ID = nextlookupID;
                segment = my_data->Hash_Nodes_T[nextlookupID].getbreadcrumb();
            }
        }        
        //This node is visited
        my_data->Hash_Nodes_T[ID].setvisited(true);
        vector<small_node> outedges=my_data->Hash_Nodes_T[ID].getoutedges();
        unsigned size = outedges.size();
        for (unsigned i = 0; i < size; i++) {
            double penalty = 0;
            unsigned visitingID = outedges[i].interID;
            unsigned source = outedges[i].segID;
            if (!my_data->Hash_Nodes_T[visitingID].getvisited()) {
                //if parent node's cost+weight+potentially 15s penaly<current cost,
                //modify current cost
                if (!round1 && (getStreetSegmentStreetID(source) != getStreetSegmentStreetID(segment)))
                    penalty = 0.25;
                //G is movement cost+parent cost+penalty
                double G = my_data->Hash_Nodes_T[ID].getcost()+outedges[i].weight + penalty;
                if (my_data->Hash_Nodes_T[visitingID].getcost() > G) {
                    my_data->Hash_Nodes_T[visitingID].setcost(G);
                    my_data->Hash_Nodes_T[visitingID].setbreadcrumb(source);
                    waveform.push(my_data->Hash_Nodes_T[visitingID]);
                }
            }
        }
        round1=false;
    }
    while (count);
    //The following tests whether the path time found in the above method is
    //the same with the one found by find_path
    unsigned pathsize=my_data->path.size()-1;
    //cout<<"pathsize is "<<pathsize<<endl;
    double theotime=0;
    //double realtime=0;
    for(unsigned i=0;i<pathsize;i++){
        unsigned key=IDtoIndex.at(my_data->path[i+1]);
        if((paths.at(my_data->path[i]))[key]==0){
            cout<<"error at "<<my_data->path[i]<<" to "<<my_data->path[i+1]<<endl;
            exit (EXIT_FAILURE);
        }
        else{
            //cout<<i<<": theo: "<<(paths.at(my_data->path[i]))[key]<<endl;
            theotime=theotime+(paths.at(my_data->path[i]))[key];
        }
        /*vector<unsigned> temppath=find_path_between_intersections(my_data->path[i],my_data->path[i+1]);
        double temptime=compute_path_travel_time(temppath);
        cout<<i<<":real "<<temptime<<endl;
        realtime=realtime+temptime;*/
    }
    /*cout<<"travelingsalesman time is "<<theotime<<endl;
    cout<<"findpath time is "<<realtime;*/
    my_data->traveltime=theotime;
    /********calling 2-opt***********/
    twoopt(my_data,paths,IDtoIndex,my_data->intersections_to_traverse);
    pthread_exit(NULL);
}

void twoopt(struct thread_data * my_data,unordered_map<unsigned,vector<double>>& paths,
            unordered_map<unsigned,unsigned>& IDtoIndex,
            vector<unsigned> intersections_to_traverse){
    //the best path is the original path so far
    vector<unsigned> best_path=my_data->path;
    double best_time=my_data->traveltime;
    // Get path size
    int size = best_path.size();
    clock_t startTime=my_data->starttime;
    // repeat until no improvement is made 
    clock_t currentTime=clock();
    float timespent = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;
    for (int i = 0; i < size - 1 && timespent < TIME_LIMIT * 0.85; i++) {
        for (int k = i + 1; k < size && timespent < TIME_LIMIT * 0.85; k++) {
            vector<unsigned> new_path;
            //1.keep the old path from best_path[0]-[i-1]
            double new_time = 0;
            for (int c = 0; c <= i - 1; c++) {
                new_path.push_back(best_path[c]);
            }

            //2.reverse the path from best_path[i]-[k]
            unsigned dec = 0;
            for (int c = i; c <= k; c++) {
                new_path.push_back(best_path[k - dec]);
                dec++;
            }

            //3.copy the path from best_path[k+1] till end
            for (int c = k + 1; c < size; c++) {
                new_path.push_back(best_path[c]);
            }
            for (int c = 0; c < size - 1; c++) {
                int key = IDtoIndex.at(new_path[c + 1]);
                double intermediate_time = (paths.at(new_path[c]))[key];
                if (intermediate_time == 0) {
                    vector<unsigned> intermediate_path = find_path_between_intersections(
                            new_path[c], new_path[c + 1], my_data->Hash_Nodes_T);
                    intermediate_time = compute_path_travel_time(intermediate_path);
                    (paths.at(new_path[c]))[key] = intermediate_time;
                    (segpaths.at((new_path[c])))[key] = intermediate_path;
                }
                new_time = new_time + intermediate_time;
            }

            if (new_time < best_time) {
                best_path = new_path;
                best_time = new_time;
            }
            currentTime = clock();
            timespent = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;
        }
    }
    my_data->path=best_path;
    my_data->traveltime=best_time;
    
   //Random reverse part of the path in the remaining time
    int count=1;
    int index=0;
    int traverse_size=my_data->intersections_to_traverse.size();
    while(timespent< TIME_LIMIT * 0.85&&count<size){
        //work on the best path so far
        vector<unsigned> new_path = best_path;
        double new_time=0;
        //reverse
        unsigned tempID=new_path[index+count];
        new_path[index+count]=new_path[index];
        new_path[index]=tempID;
        bool switch_possible = true;
        for (int c = 0; c <= (count / 2); c++) {
            unsigned tempID = new_path[index + count - c];
            new_path[index + count - c] = new_path[index];
            new_path[index] = tempID;
        }
        //evalute the new travel time
        for (int c = 0; c < size - 1; c++) {
            int key = IDtoIndex[new_path[c+1]];
            double intermediate_time = paths[new_path[c]][key];
            if (intermediate_time == 0){
                vector<unsigned> intermediate_path = find_path_between_intersections(
                        new_path[c], new_path[c + 1], my_data->Hash_Nodes_T);
                if(intermediate_path.empty()){
                    //the two intersections are not connected, so set to infinite traveling time
                    paths[new_path[c]][key] = 1000000;
                    intermediate_time=1000000;
                    switch_possible=false;
                    break;
                }
                else{
                    intermediate_time = compute_path_travel_time(intermediate_path);
                    paths[new_path[c]][key] = intermediate_time;
                    segpaths[new_path[c]][key] = intermediate_path;
                }
            }
            else if( intermediate_time== 1000000){
                intermediate_time=1000000;
                switch_possible=false;
                break;               
            }
            new_time = new_time + intermediate_time;
        }
 
        //best path equals new path if traveling time is less
        if(switch_possible&&new_time<best_time){
            best_path=new_path;
            best_time=new_time;
        }
        //Reset for next round
        switch_possible=true;        
        index=index+1;
        if((index+count)>=size){
            index=0;
            count=count+1;
        }
        currentTime = clock();
        timespent = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;  
        if(count>=size)
            timespent=TIME_LIMIT;
            
    }
    my_data->path=best_path;
    my_data->traveltime=best_time;
    
    //Random swap again in the remaining time
    count=1;
    index=0;
    traverse_size=my_data->intersections_to_traverse.size();
    while(timespent< TIME_LIMIT * 0.85&&count<size){
        //work on the best path so far
        vector<unsigned> new_path = best_path;
        double new_time=0;
        //swap
        unsigned tempID=new_path[index+count];
        new_path[index+count]=new_path[index];
        new_path[index]=tempID;
        bool switch_possible=true;
        //evalute the new travel time
        for (int c = 0; c < size - 1; c++) {
            int key = IDtoIndex[new_path[c+1]];
            double intermediate_time = paths[new_path[c]][key];
            if (intermediate_time == 0){
                vector<unsigned> intermediate_path = find_path_between_intersections(
                        new_path[c], new_path[c + 1], my_data->Hash_Nodes_T);
                if(intermediate_path.empty()){
                    //the two intersections are not connected, so set to infinite traveling time
                    paths[new_path[c]][key] = 1000000;
                    intermediate_time=1000000;
                    switch_possible=false;
                    break;
                }
                else{
                    intermediate_time = compute_path_travel_time(intermediate_path);
                    paths[new_path[c]][key] = intermediate_time;
                    segpaths[new_path[c]][key] = intermediate_path;
                }
            }
            else if( intermediate_time== 1000000){
                intermediate_time=1000000;
                switch_possible=false;
                break;               
            }
            new_time = new_time + intermediate_time;
        }
 
        //best path equals new path if traveling time is less
        if(switch_possible&&new_time<best_time){
            best_path=new_path;
            best_time=new_time;
        }
        //Reset for next round
        switch_possible=true;        
        index=index+1;
        if((index+count)>=size){
            index=0;
            count=count+1;
        }
        currentTime = clock();
        timespent = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;  
        if(count>=size)
            timespent=TIME_LIMIT;
            
    }
    my_data->path=best_path;
    my_data->traveltime=best_time;
    
    int paths_size=paths.size();
    for(int i=0;i<paths_size;i++)
        paths[i].clear();
    paths.clear();
}

 void add_depo(vector<unsigned>& chosen_path, vector<unsigned> depot_locations,clock_t startTime){
     unsigned start_depot=depot_locations[0];
     unsigned end_depot=depot_locations[0];
     
     unsigned start_delivery=chosen_path.front();
     unsigned end_delivery=chosen_path.back();
     
     LatLon start_delivery_pos=getIntersectionPosition(start_delivery);
     LatLon end_delivery_pos=getIntersectionPosition(end_delivery);
     
     LatLon depot_pos=getIntersectionPosition(depot_locations[0]);
     
     double min_distance_start=find_distance_between_two_points(start_delivery_pos,depot_pos);
     double min_distance_end=find_distance_between_two_points(end_delivery_pos,depot_pos);
     
     double temp_start_distance,temp_end_distance;
     unsigned depot_size=depot_locations.size();

    clock_t currentTime = clock();
    float timespent = ((float) (currentTime - startTime)) / CLOCKS_PER_SEC;
    if (timespent < TIME_LIMIT * 0.9) {
        //change the definition of min_distance_start and min_distance_end;
        //also assume the depot_locations[0] both provides the minimum first
        vector<unsigned> startdepot_delivery=find_path_between_intersections
                                                    (depot_locations[0],chosen_path[0]);
        vector<unsigned> enddelivery_depot=find_path_between_intersections
                                                    (chosen_path.back(),depot_locations[0]);
        min_distance_start=compute_path_travel_time(startdepot_delivery);
        min_distance_end=compute_path_travel_time(enddelivery_depot);
        for (unsigned i = 1; i < depot_size; i++) {
            startdepot_delivery=find_path_between_intersections
                                                    (depot_locations[i],chosen_path[0]);
            enddelivery_depot=find_path_between_intersections
                                                    (chosen_path.back(),depot_locations[i]);
          
            //use the same variables that are for distance  for time
            // do not matter, just variables.. also double
            temp_start_distance=compute_path_travel_time(startdepot_delivery);
            temp_end_distance=compute_path_travel_time(enddelivery_depot);
            if (temp_start_distance < min_distance_start) {
                start_depot = depot_locations[i];
                min_distance_start = temp_start_distance;
            }
            if (temp_end_distance < min_distance_end) {
                end_depot = depot_locations[i];
                min_distance_end = temp_end_distance;
            }
        }
    }
    else {
        for (unsigned i = 1; i < depot_size; i++) {
            depot_pos = getIntersectionPosition(depot_locations[i]);
            temp_start_distance = find_distance_between_two_points(start_delivery_pos, depot_pos);
            temp_end_distance = find_distance_between_two_points(end_delivery_pos, depot_pos);
            if (temp_start_distance < min_distance_start) {
                start_depot = depot_locations[i];
                min_distance_start = temp_start_distance;
            }
            if (temp_end_distance < min_distance_end) {
                end_depot = depot_locations[i];
                min_distance_end = temp_end_distance;
            }
        }
    }
     unsigned chosen_path_size=chosen_path.size();
     chosen_path.push_back(chosen_path.back());
     for(unsigned i=chosen_path_size-1;i>0;i--){
         chosen_path[i]=chosen_path[i-1];
     }
     chosen_path[0]=start_depot;
     chosen_path.push_back(end_depot);
 }
 
 
    
     //another algorithm called genetic algorithm
void* genetic_algorithm(void *threadarg)
{    //get data
    clock_t start_time=clock();
    struct thread_data *data;
    data=(struct thread_data*) threadarg;
    vector<unsigned> intersections_to_traverse=data->intersections_to_traverse;
    vector<unsigned> depot_locations=data->depot_locations;
    //if intersections size<5,return a random path
    if(intersections_to_traverse.size()<5)
    {
         vector<unsigned>best_seg;
         double best_time=0;
     for(unsigned j=0;j<intersections_to_traverse.size()-1;j++)
     {
         vector<unsigned> one_path=find_path_between_intersections(intersections_to_traverse[j],intersections_to_traverse[j+1],data->Hash_Nodes_T);
         for(unsigned k=0;k<one_path.size();k++)
             best_seg.push_back(one_path[k]);
         best_time=best_time+compute_path_travel_time(one_path);
    }
        data->path=intersections_to_traverse;
        data->seg_path=best_seg;
        data->traveltime=best_time;
        return data;
    }
    set<pair<double,vector<unsigned>>> distance_to_route;
    unordered_map <double,vector<double>> dist_to_seperate_dist;
    unsigned num_per_chain=intersections_to_traverse.size()/5+1;//number of elements per chain
    for(unsigned i=0;i<num_per_chain*10;i++)
    {
        vector<unsigned> this_shuffle=intersections_to_traverse;
        random_shuffle(this_shuffle.begin(),this_shuffle.end());
        double dist=0;
          vector<double>dist_vec;
        for(unsigned j=0;j<this_shuffle.size()-1;j++)
        {   
            dist_vec.push_back(find_distance_between_two_points(getIntersectionPosition(this_shuffle[j]),getIntersectionPosition(this_shuffle[j+1])));
            dist=dist+dist_vec[j];
        }
          dist_to_seperate_dist.emplace(dist,dist_vec); 
          pair<double,vector<unsigned>> this_pair;
          this_pair.first=dist;
          this_pair.second=this_shuffle;
          distance_to_route.insert(this_pair);
    }

    clock_t current_time=clock();
    float timeused=((float)(current_time-start_time))/CLOCKS_PER_SEC;
    //while loop to iterate many times
    while(timeused<(30-num_per_chain))
    {  // copy out the needed route and distance
        vector<unsigned>route1;
        vector<unsigned>route2;
        double dist1,dist2;
        auto iter1=distance_to_route.begin();
        dist1=(*iter1).first;
        //cout<<"dist1 is:"<<dist1<<endl;
        route1=(*iter1).second;
        vector<double> seperate_dist1=dist_to_seperate_dist.at(dist1);
        iter1++;
        auto iter2=iter1;
        dist2 =(*iter2).first;
        //cout<<"dist2 is:"<<dist2<<endl;
        route2=(*iter2).second;  
        vector<double> seperate_dist2=dist_to_seperate_dist.at(dist2);
        unsigned chain_start_id=0;
        //unsigned chain_end_id=0;
        double min_dist=10000000000;
        double current_dist=0;
                 
//stuff to do with the second route
        //for loop to calculate the distance of first chain 
        chain_start_id=0;
       // chain_end_id=0;
        min_dist=1000000000000;
        current_dist=0;
        for (unsigned l=0;l<num_per_chain-1;l++)
            current_dist=current_dist+seperate_dist2[l];
        unsigned first_ele=1;
        unsigned last_ele=first_ele+num_per_chain-2;
        //while loop to iterate through each element and find the chain with the smallest dist
         while(last_ele<seperate_dist2.size())
        {
             current_dist=current_dist-seperate_dist2[first_ele]+seperate_dist2[last_ele];
            //if smaller dist found,update start_id
            if(current_dist<min_dist)
            {
                min_dist=current_dist;
                chain_start_id=first_ele;
            }
             first_ele++;
             last_ele++;
        }

        //a set for the other route to check existence
         set<unsigned> check_existance2;
         vector<unsigned> copy_chain2;
         for(unsigned l=chain_start_id;l<chain_start_id+num_per_chain;l++)
         {
             check_existance2.insert(route2[l]);
             copy_chain2.push_back(route2[l]);
         }

         double  max_distance=0;
         unsigned max_id=0;
         for(unsigned m=0;m<seperate_dist1.size();m++)
         {
             if(seperate_dist1[m]>max_distance)
             {
                 max_distance=seperate_dist1[m];
                 max_id=m;
             }
         } 
         //copy the data into a new route
         vector<unsigned> new_route2;
         for(unsigned n=0;n<=max_id;n++)
         {
             if(check_existance2.find(route1[n])==check_existance2.end())
             {
                 new_route2.push_back(route1[n]);
             }
         }
         for(unsigned n=0;n<copy_chain2.size();n++)
             new_route2.push_back(copy_chain2[n]);
         for(unsigned n=max_id+1;n<route1.size();n++)
         {
             if(check_existance2.find(route1[n])==check_existance2.end())
             {
                 new_route2.push_back(route1[n]);
             }
         }
         dist2=0;
         vector<double>dist_vec2;
        for(unsigned j=0;j<new_route2.size()-1;j++)
        {   
            dist_vec2.push_back(find_distance_between_two_points(getIntersectionPosition(new_route2[j]),getIntersectionPosition(new_route2[j+1])));
            dist2=dist2+dist_vec2[j];
        }
          pair<double,vector<unsigned>> this_pair2;
          this_pair2.first=dist2;
          this_pair2.second=new_route2;

          dist_to_seperate_dist.emplace(dist2,dist_vec2);        
          distance_to_route.insert(this_pair2);
          //cout<<distance_to_route.size()<<endl;
          if(distance_to_route.find(this_pair2)==distance_to_route.end())
          {
          auto it=distance_to_route.end();
          it--;
          distance_to_route.erase(it);
          }

//keep the first element && delete the others,reinsert until size expands again
   auto iter=distance_to_route.begin();
    iter++;
    while(iter!=distance_to_route.end())
    {  
        iter++;
        auto iter5=iter;
        iter--;
        distance_to_route.erase(iter);
        iter=iter5;
    }
   
    dist_to_seperate_dist.clear();
    iter=distance_to_route.begin();
    vector<unsigned> temp_path=(*iter).second;
    double temp_dist=0;
          vector<double> temp_dist_vec;
        for(unsigned j=0;j<temp_path.size()-1;j++)
        {   
            temp_dist_vec.push_back(find_distance_between_two_points(getIntersectionPosition(temp_path[j]),getIntersectionPosition(temp_path[j+1])));
            temp_dist=temp_dist+temp_dist_vec[j];
        }
          if(dist_to_seperate_dist.find(temp_dist)==dist_to_seperate_dist.end())
          dist_to_seperate_dist.emplace(temp_dist,temp_dist_vec); 
        for(unsigned i=0;i<num_per_chain*10;i++)
    {
        vector<unsigned> this_shuffle=intersections_to_traverse;
        random_shuffle(this_shuffle.begin(),this_shuffle.end());
        double dist=0;
          vector<double>dist_vec;
        for(unsigned j=0;j<this_shuffle.size()-1;j++)
        {   
            dist_vec.push_back(find_distance_between_two_points(getIntersectionPosition(this_shuffle[j]),getIntersectionPosition(this_shuffle[j+1])));
            dist=dist+dist_vec[j];
        }
          if(dist_to_seperate_dist.find(dist)==dist_to_seperate_dist.end())
          dist_to_seperate_dist.emplace(dist,dist_vec); 
          pair<double,vector<unsigned>> this_pair;
          this_pair.first=dist;
          this_pair.second=this_shuffle;
          if(distance_to_route.find(this_pair)==distance_to_route.end())
          distance_to_route.insert(this_pair);
    }
          current_time=clock();
        timeused=((float)(current_time-start_time))/CLOCKS_PER_SEC;

    }
    
    vector<unsigned> best_seg;
    vector<unsigned> best_path;
    double best_time=10000000000;
    auto iter=distance_to_route.begin();
    //for loop to compute the best few 
    for(unsigned i=0;i<2;i++)
    {
     vector<unsigned> temp_path=(*iter).second;
     vector<unsigned> temp_seg;
     //find the complete street segment vector
     for(unsigned j=0;j<temp_path.size()-1;j++)
     {
         vector<unsigned> one_path=find_path_between_intersections(temp_path[j],temp_path[j+1],data->Hash_Nodes_T);
         for(unsigned k=0;k<one_path.size();k++)
             temp_seg.push_back(one_path[k]);
    }
     double temp_time=compute_path_travel_time(temp_seg);
     if (temp_time<best_time)
     {
         best_time=temp_time;
         best_seg=temp_seg;
         best_path=temp_path;
     }
     iter++;
    }
    //store data into multi_threading structure
        data->path=best_path;
        data->seg_path=best_seg;
        data->traveltime=best_time;
}


