#include "m1.h"
#include <iostream>
#include <set>


using namespace std;
//Declare a global hash table(key is street name, value is a vector storing street ID and street segments ID)
unordered_map<string,vector<unsigned>> Hash_Street;
//Declare a global hash table storing two ends (Intersection ID)of a street.
//unordered_map<string,vector<unsigned>> StreetTwoEnds;
//Declare a global hash table(key is intersection name, value is a intersection object
//storing: intersection id and latitude and longitude)
unordered_map<string,intersection> Hash_Intersection;

//Declare a global hash table(key is intersection name, value is a vector of street names)
unordered_map<string,vector<string>> Hash_Intersection_Streets;

//Declare a global hash table(Key is intersection name, value is a vector of segment ids)
unordered_map<string,vector<unsigned>> Hash_Intersection_Segments;

//Declare a global hash table(Key is intersection ID, value is intersection name)
unordered_map<unsigned,string> Hash_Intersection_name;

//Declare a global hash table(Key is POI index, value is a small region where the POI belongs)
unordered_map<unsigned,t_bound_box>Hash_POI;

//Declare a vector(value is class of node_path)
vector<node_path> Hash_Nodes;

//A vector of all intersection names (used by readline)
vector<const char*> intersection_names;
//A vector of all POI names (used by readline)
vector<const char*> POI_names;
//A vector of intersection IDs (used by draw_closest_path)
vector<unsigned> locations;
//A vector of destination POI name (used by draw_closest_path)
vector<string> POI_destination_name;

//A vector including a possible city names
vector<string> city_names = {
    "cairo_egypt",
    "moscow",
    "saint_helena",
    "toronto",
    "hamilton_canada",
    "newyork",
    "toronto_canada"   
};

zoomlevel zl;
//Declare a global structure storing streets drawn at different zoom-in levels
zoomstreet zs;
//Declare of global hash tables. Key is POI names, value is a vector of POI ids 
//which have the key name 
unordered_map<string,vector<unsigned>> POIofsamenames;
unsigned Number_of_interest;
//load the map
bool load_map(std::string map_name) {

    bool load_success = loadStreetDatabaseBIN(map_name);

    // building up the hash table. The first unsigned in the vector is the streetID
    unsigned Num_Street=(unsigned)getNumberOfStreets();
    for(unsigned i=0; i<Num_Street; i++){
        vector<unsigned> my_ID;
        my_ID.push_back(i);
        pair<string,vector<unsigned>> newpair (getStreetName(i),my_ID);
        Hash_Street.insert(newpair);
    }
    //store data size
    Number_of_interest=getNumberOfPointsOfInterest();
    
    //Add streetsegment IDs to the street(the vector<unsigned> in Hash_Street)they belong to 
    unsigned Num_StreetSeg=(unsigned)getNumberOfStreetSegments();
    unsigned BelongToStreetID;
    string BelongToStreetName;
    for(unsigned i=0; i<Num_StreetSeg; i++){
        BelongToStreetID=getStreetSegmentStreetID(i);
        BelongToStreetName=getStreetName(BelongToStreetID);  
        if(Hash_Street.find(BelongToStreetName)!=Hash_Street.end())
            Hash_Street.at(BelongToStreetName).push_back(i);  
    }

    //Store data into Hash_Intersection
    unsigned Num_Intersection=(unsigned)getNumberOfIntersections();
    for(unsigned i=0;i<Num_Intersection;i++){
        string name=getIntersectionName(i);
        //construct a newintersection element
        intersection newintersection;
        //Storing intersectionID info
        newintersection.myID=i;
        //storing intersection's latitude and longtitude
        newintersection.Position=getIntersectionPosition(i);
        //insert the newly constructed element into Hash_Intersection
        Hash_Intersection.emplace(name,newintersection);
        
        const char* inter_name = name.c_str();          //converts string into char*
        
        //stores name of intersections into intersection_names vector
        intersection_names.push_back(inter_name);
        //storing segmentIDs into Hash_Intersection_Segments
        Hash_Intersection_Segments.emplace(name,getIntersectionStreetSegment(i));
        
        
        //storing names into Hash_intersection_name;        
        Hash_Intersection_name.emplace(i,name);
        
        
        //storing street names into Hash_Intersection_Streets. Also eliminate duplicates
        bool same=false;
        string streetname;                                        
        for (unsigned j = 0; j< Hash_Intersection_Segments.at(name).size(); j++){
            unsigned streetid = getStreetSegmentStreetID(Hash_Intersection_Segments.at(name)[j]);
            streetname = getStreetName(streetid);
            //if it is the first street segment, insert it anyways
            //doing it is not only for efficiency but also for Hash_Intersection_Streets.at(name)
            //to be valid
            if(j==0){
                vector<string> street_vec;
                street_vec.push_back(streetname);
                Hash_Intersection_Streets.emplace(name,street_vec);
            }    
            else{
                //size of the streetname vector
                unsigned size=Hash_Intersection_Streets.at(name).size();
                //check for duplicates
                for(unsigned k=0; k<size;k++)
                {
                    if(streetname==(Hash_Intersection_Streets.at(name))[k]){
                        same=true;
                        break;
                    }                    
                }
                //if name does not exist in the vector, insert it
                if(!same)
                    Hash_Intersection_Streets.at(name).push_back(streetname);
                same=false;
            }
        }   
        
          
    }
    //Building up Hash_Nodes
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
        Hash_Nodes.push_back(node_path(ListofNodes, i));
        //current.setoutedges(ListofNodes);

        ListofNodes.clear();

    }

   
 /*for(unsigned i=0; i<getNumberOfIntersections(); i++){
     cout<<i<<"   ";
     cout<<Hash_Nodes[i]->getmyID()<<"   ";
     for(unsigned j=0;j<Hash_Nodes[i]->getoutedges().size();j++)
     {   
         cout<<Hash_Nodes[i]->getoutedges()[j].interID<<"  ";
         cout<<Hash_Nodes[i]->getoutedges()[j].segID<<"  ";
         cout<<Hash_Nodes[i]->getoutedges()[j].weight<<"  ";
     }
     cout<<endl;
        
 }*/

    //Sorting groups of streets
    vector <double> street_length;
    double st_len;
    string st_name;
    
    //create array of street length
    for (auto iter_st=Hash_Street.cbegin(); iter_st != Hash_Street.cend(); iter_st++){
    
        st_name = iter_st->first;
        st_len = find_street_length(st_name);        
        street_length.push_back(st_len);
    }
    
    sort(street_length.begin(),street_length.end());    //sort the array of length from small to large
    
    unsigned long long st_total = getNumberOfStreets();
    //Each zoom-level holds a group of streets that are the extra streets.
    //However, the number of streets each zoom-level holds are different.
    //Therefore, the team decided to divide streets in the following way:
    //level0 holds 1*Street_Divider(#of streets)
    //level1 holds 2*Street_Divider
    //level2 holds 3*Street_Divider and so on
    unsigned bound = st_total/Street_Divider;       
    //find boundaries for the groups
    
    //sorts streets and street segments into 9 groups
    for (auto iter_st=Hash_Street.cbegin(); iter_st != Hash_Street.cend(); iter_st++ ){
        
        st_name = iter_st->first;               //find street name
        st_len = find_street_length(st_name);   //find street length
            
        if (st_name == "(unknown)"||(st_len < street_length[9*bound] && st_len >= 0)){
            
           for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level8.push_back(iter_st->second[i]);        //store street segments into vector
           zs.level8.push_back(st_name);                        //store streets into vector
        }
        else if (st_len < street_length[17*bound] && st_len >= street_length[9*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)  
                zl.level7.push_back(iter_st->second[i]);        
            zs.level7.push_back(st_name);                         
            zs.level7.push_back(st_name);
        }
        else if (st_len < street_length[24*bound] && st_len >= street_length[17*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level6.push_back(iter_st->second[i]);     
            zs.level6.push_back(st_name);                  
            zs.level6.push_back(st_name);
        }
        else if (st_len < street_length[30*bound] && st_len >= street_length[24*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level5.push_back(iter_st->second[i]);
            zs.level5.push_back(st_name);
        }
        else if (st_len < street_length[35*bound] && st_len >= street_length[30*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level4.push_back(iter_st->second[i]);
            zs.level4.push_back(st_name);
        }
        else if (st_len < street_length[39*bound] && st_len >= street_length[35*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level3.push_back(iter_st->second[i]);
            zs.level3.push_back(st_name);
        }
        else if (st_len < street_length[42*bound] && st_len >= street_length[39*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level2.push_back(iter_st->second[i]);
            zs.level2.push_back(st_name);
        }
        else if (st_len < street_length[44*bound] && st_len >= street_length[42*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level1.push_back(iter_st->second[i]);
            zs.level1.push_back(st_name);
        }
        else if (st_len >= street_length[44*bound]){
            for (unsigned i=1;i < iter_st->second.size();i++)
                zl.level0.push_back(iter_st->second[i]);
            zs.level0.push_back(st_name);
        }
        
    }
    
    //POI hash_table
    LatLon POI_pos,int_pos;
    t_point bot_left,top_right;
    
    for (unsigned POI_index=0; POI_index<Number_of_interest; POI_index++){
        
        POI_pos = getPointOfInterestPosition(POI_index);
        
        bot_left.x = (float)POI_pos.lon;                    //creates t_bound box around POI
        bot_left.y = (float)POI_pos.lat;
        
        top_right.x = bot_left.x+0.0001;
        top_right.y = bot_left.y+0.0001;
      
        t_bound_box POI_region(bot_left,top_right);
        
        Hash_POI.emplace(POI_index,POI_region);             //pairs POI t_bound box with POI ID
    }
    
    for (unsigned POI_index=0; POI_index<Number_of_interest; POI_index++){
        
        string name = getPointOfInterestName(POI_index);
        
        const char* POI_name = name.c_str(); 
        
        //Stores name of point of interests into POI_names vector
        POI_names.push_back(POI_name);
        
        //Building up POIofsamenames
        if (POIofsamenames.find(name)==POIofsamenames.end()){
            vector<unsigned> POIID;
            POIID.push_back(POI_index);
            POIofsamenames.emplace(name,POIID);
        }
        else
            POIofsamenames.at(name).push_back(POI_index);
    }
    
   return load_success;
}


//close the map
void close_map() {
    closeStreetDatabase();
    for(auto iter=Hash_Street.begin();iter!=Hash_Street.end();iter++)
        (iter->second).clear();
    Hash_Street.clear();
    Hash_Intersection.clear();
    for(auto iter=Hash_Intersection_Streets.begin();
                iter!=Hash_Intersection_Streets.end();iter++)
        (iter->second).clear();
    Hash_Intersection_Streets.clear();
    for(auto iter=Hash_Intersection_Segments.begin();
            iter!=Hash_Intersection_Segments.end();iter++) 
        (iter->second).clear();
    Hash_Intersection_Segments.clear();
    Hash_Intersection_name.clear();
    Hash_POI.clear();
    zl.level0.clear();zl.level1.clear();zl.level2.clear();zl.level3.clear();
    zl.level4.clear();zl.level5.clear();zl.level6.clear();zl.level7.clear();
    zl.level8.clear();
    zs.level0.clear();zs.level1.clear();zs.level2.clear();zs.level3.clear();
    zs.level4.clear();zs.level5.clear();zs.level6.clear();zs.level7.clear();
    zs.level8.clear();
    Hash_Nodes.clear();
    intersection_names.clear();
    POI_names.clear();
    locations.clear();              
    POI_destination_name.clear(); 

}


//using the intersection_street hash table, find the names of streets at the intersection given its ID
std::vector<std::string> find_intersection_street_names(unsigned intersection_id){
     
    return Hash_Intersection_Streets.at(Hash_Intersection_name.at(intersection_id)); //converts intersection ID to its name using the intersection_name hash table
   
}

//using the intersection_street hash table, find the names of streets at the intersection given its name
std::vector<std::string> find_intersection_street_names(std::string intersection_name){
    
     return Hash_Intersection_Streets.at(intersection_name);
}

//using the intersection_segment hash table, find the IDs of the street segments at the intersection given its ID
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id){
    
    string inter_name;
    
    inter_name = Hash_Intersection_name.at(intersection_id);//converts intersection ID to its name using the intersection_name hash table
    
    return Hash_Intersection_Segments.at(inter_name);
}

//using the intersection_segment hash table, find the IDs of the street segments at the intersection given its name
std::vector<unsigned> find_intersection_street_segments(std::string intersection_name){
    
    return Hash_Intersection_Segments.at(intersection_name);
}

             
//returns the id of an intersection given its name using the general intersection hash table
unsigned find_intersection_id_from_name(std::string intersection_name)
{
  return Hash_Intersection.at(intersection_name).myID;
  
}


//checks whether two intersections are connected through a street segment
bool are_directly_connected(std::string intersection_name1, std::string intersection_name2)
{
    intersection intersection1,intersection2;               //defines variables of intersection type
    intersection1= Hash_Intersection.at(intersection_name1); //obtains ID and latlon from hash table
    intersection2= Hash_Intersection.at(intersection_name2);
    
    //use nested for loops to iterate through street segment vectors to find a common street segment
    for(auto iter1=Hash_Intersection_Segments.at(intersection_name1).begin();iter1!=Hash_Intersection_Segments.at(intersection_name1).end();iter1++)
    {
        for(auto iter2=Hash_Intersection_Segments.at(intersection_name2).begin();iter2!=Hash_Intersection_Segments.at(intersection_name2).end();iter2++)
        { 
            if((*iter1)==(*iter2))
            {  //found the commonly connected street segment
                unsigned common_street_segment= (*iter1);
                if (!getStreetSegmentOneWay(common_street_segment))
                    return true;            //return true if segment is two ways
            else
              {
                if ((getStreetSegmentEnds(common_street_segment).from==unsigned(intersection1.myID))&&(getStreetSegmentEnds(common_street_segment).to==unsigned(intersection2.myID)))
                    return true;            //if street segment is one way from 1st intersection to 2nd intersection, return true
              }
                       
            }
        }
            
    }
    return false;
}

//for a given street, return all the street segments
vector<unsigned> find_street_street_segments(string street_name){
    if(Hash_Street.find(street_name)!=Hash_Street.end()){
        
        vector<unsigned> street_segments=Hash_Street.at(street_name);
        //need to erase the first element in vector because it is the streetID
        street_segments.erase(street_segments.begin());
        //return
        return street_segments;
    }    
    else
        exit (EXIT_FAILURE);
}


//returns the ID of a street given its name using the street hash table
unsigned find_street_id_from_name(std::string street_name)
{
    if(Hash_Street.find(street_name)!=Hash_Street.end())
        return *(Hash_Street.at(street_name).begin());
    else
        exit (EXIT_FAILURE);
}



//find all intersection connected by one street segment from given intersection 
std::vector<unsigned> find_adjacent_intersections(std::string intersection_name)
{
    bool oneway;
    vector<unsigned> segment_vec;
    vector<unsigned> adj_inter_vec;
    StreetSegmentEnds endpoint;
    unsigned intersection_id;
    
    intersection_id = Hash_Intersection.at(intersection_name).myID; //gets intersection ID
    
    segment_vec = Hash_Intersection_Segments.at(intersection_name); //gets street segment vector at that intersection
    
    for (unsigned i = 0; i < segment_vec.size(); i++){
        
       oneway = getStreetSegmentOneWay(segment_vec[i]);             //checks for one way
       
       endpoint = getStreetSegmentEnds(segment_vec[i]);             //gets the endpoints for the street segment           
       
       if (oneway){                                                 //adds intersection ID to vector
           if ((unsigned)endpoint.from == intersection_id)         
               adj_inter_vec.push_back((unsigned)endpoint.to);
       }
       else{                    //two ways
           if ((unsigned)endpoint.from == intersection_id)
               adj_inter_vec.push_back((unsigned)endpoint.to);
           else 
               adj_inter_vec.push_back((unsigned)endpoint.from);
       }
       
    }
    
    return adj_inter_vec;                                               //return vector
}

//for a given street, find all the intersections
std::vector<unsigned> find_all_street_intersections(std::string street_name){
    if(Hash_Street.find(street_name)==Hash_Street.cend())
        exit (EXIT_FAILURE);
    //if the name exists in the hash table
    else{
//Use set to avoid duplicate
        set<unsigned> IntersectionsID;
//StreetSegments has all the StreetSegmentsID made up the street. Except the first
//element is the street ID which is ignored by doing StreetSegments.begin()+1
        vector<unsigned> StreetSegments = Hash_Street.at(street_name);
        for(auto iter=StreetSegments.cbegin()+1; iter!=StreetSegments.cend();iter++){
            StreetSegmentEnds Segment_for_i=getStreetSegmentEnds(*iter);
            IntersectionsID.insert((unsigned)Segment_for_i.from);
            IntersectionsID.insert((unsigned)Segment_for_i.to);    
        }
//copy all the data over a vector, then return
        vector<unsigned> intersectionsID;
        for(auto iter=IntersectionsID.cbegin();iter!=IntersectionsID.cend();iter++)
            intersectionsID.push_back(*iter);
        return intersectionsID;
    }
}

//find distance between two coordinates
double find_distance_between_two_points(LatLon point1, LatLon point2)
{   if((point1.lat==point2.lat)&&(point1.lon==point2.lon))
    return 0;
    double x1,y1,x2,y2;
      //get the average latitude of two points
    double latAve=((point1.lat)+(point2.lat))/2*DEG_TO_RAD;
     //compute the required value and store them into x1,y1,x2,y2 respectively
    x1=(point1.lon)*cos(latAve);
    y1=point1.lat;
    x2=(point2.lon)*cos(latAve);
    y2=point2.lat;
     //calculate the distance
    double distance=sqrt((y2-y1)*(y2-y1)+(x2-x1)*(x2-x1))*DEG_TO_RAD*EARTH_RADIUS_IN_METERS;
    return distance;
}

//find the length of a given street segments
double find_street_segment_length(unsigned street_segment_id)
{   
    long double distance=0;
    unsigned intersection1,intersection2;
        LatLon point1,point2;
        //find two intersections
        intersection1=getStreetSegmentEnds(street_segment_id).from;
        intersection2=getStreetSegmentEnds(street_segment_id).to;
        //find lat/lon to the two intersections
        point1=getIntersectionPosition(intersection1);
        point2=getIntersectionPosition(intersection2);
    if(getStreetSegmentCurvePointCount(street_segment_id)==0)           //not a curve
    {   
        distance=find_distance_between_two_points(point1,point2);
        return distance;
    }
    else                                                                
    {   
        unsigned i;
        distance=find_distance_between_two_points(point1,getStreetSegmentCurvePoint(street_segment_id,0));
        
        for(i=0; i+1< getStreetSegmentCurvePointCount(street_segment_id);i++)
        {
            distance=distance+find_distance_between_two_points(getStreetSegmentCurvePoint(street_segment_id,i),getStreetSegmentCurvePoint(street_segment_id,i+1));
        }
        
        distance= distance+find_distance_between_two_points(getStreetSegmentCurvePoint(street_segment_id,i),point2);
        return distance ;
    }
}

//find the length of a whole street
double find_street_length(std::string street_name)
{
    if(Hash_Street.find(street_name)==Hash_Street.cend())
        exit (EXIT_FAILURE);
    //street name is found
    else{
        double length=0.0;
        vector<unsigned> StreetSegments=Hash_Street.at(street_name);
        //iter is cbegin()+1 because it has to skip over the streetID
        for(auto iter=StreetSegments.cbegin()+1;iter!=StreetSegments.cend();iter++)
            length+=find_street_segment_length(*iter);
        return length;
    }
}

//find the travel time to drive a street segment (time(minutes) = distance(Km)/speed_limit(Km/h)*60)
double find_segment_travel_time(unsigned street_segment_id)
{
    double time;
    double distance;
    double speed_limit;
    
    distance = find_street_segment_length(street_segment_id);
    
    speed_limit = getStreetSegmentSpeedLimit(street_segment_id);
    
    time = ((distance/1000)/speed_limit)*60;
   
    return time;
}

//find the nearest point of interest (by name) to a given position
std::string find_closest_point_of_interest(LatLon my_position)

{   //get the position of the first POI
    LatLon point=getPointOfInterestPosition(0);
    //find the distance between the first POI and my position, and use this as a reference
    double minDistance =find_distance_between_two_points(my_position,getPointOfInterestPosition(0));
    double current =find_distance_between_two_points(my_position,getPointOfInterestPosition(0));
    // find the difference between latitude and longitude
    double lat_difference=abs(getPointOfInterestPosition(0).lat-my_position.lat);
    double lon_difference=abs(getPointOfInterestPosition(0).lon-my_position.lon);
    
    unsigned P_O_I_id=0;
    //for loop to go over each POI
    for(unsigned i=1;i<Number_of_interest;i++)
    {   // get the position and difference of lat and lon for POI[i]
        LatLon position_for_i=getPointOfInterestPosition(i);
        double lat_difference_for_i=abs(position_for_i.lat-my_position.lat);
        double lon_difference_for_i=abs(position_for_i.lon-my_position.lon);
       //calculate the distance between two points only if lat difference or lon difference is smaller than current difference
        if ((lat_difference_for_i<lat_difference)||(lon_difference_for_i<lon_difference))
    {           
        current=find_distance_between_two_points(my_position,position_for_i);
        if(current<minDistance)
        {
            minDistance=current;
            point=position_for_i;
            P_O_I_id=i;
            lat_difference=lat_difference_for_i;
            lon_difference=lon_difference_for_i;
        }
    }
    }
    return getPointOfInterestName(P_O_I_id);
}


//same function as above, except return POI ID
unsigned find_closest_point_of_interestID(LatLon my_position){
    
    //get the position of the first POI
    LatLon point=getPointOfInterestPosition(0);
    //find the distance between the first POI and my position, and use this as a reference
    double minDistance =find_distance_between_two_points(my_position,getPointOfInterestPosition(0));
    double current =find_distance_between_two_points(my_position,getPointOfInterestPosition(0));
    // find the difference between latitude and longitude
    double lat_difference=abs(getPointOfInterestPosition(0).lat-my_position.lat);
    double lon_difference=abs(getPointOfInterestPosition(0).lon-my_position.lon);
    
    unsigned P_O_I_id=0;
    //for loop to go over each POI
    for(unsigned i=1;i<Number_of_interest;i++)
    {   // get the position and difference of lat and lon for POI[i]
        LatLon position_for_i=getPointOfInterestPosition(i);
        double lat_difference_for_i=abs(position_for_i.lat-my_position.lat);
        double lon_difference_for_i=abs(position_for_i.lon-my_position.lon);
       //calculate the distance between two points only if lat difference or lon difference is smaller than current difference
        if ((lat_difference_for_i<lat_difference)||(lon_difference_for_i<lon_difference))
    {           
        current=find_distance_between_two_points(my_position,position_for_i);
        if(current<minDistance)
        {
            minDistance=current;
            point=position_for_i;
            P_O_I_id=i;
            lat_difference=lat_difference_for_i;
            lon_difference=lon_difference_for_i;
        }
    }
    }
    return P_O_I_id;
}


vector<unsigned> find_closest_intersection_helper (string POI_name,unsigned start_id){
    vector<unsigned> Inter_ID;
    priority_queue<pair<unsigned,double>,vector<pair<unsigned,double>>,ComparePOI> Distance;
    LatLon start_position=getIntersectionPosition(start_id);
    vector<unsigned> POI=POIofsamenames.at(POI_name);
    for (unsigned i=0; i<POI.size(); i++)
    {    
            double temp_distance=find_distance_between_two_points(getPointOfInterestPosition(POI[i]),start_position);
            pair<unsigned,double> thispair(POI[i],temp_distance);
            Distance.push(thispair);
    }
    int count=0;
    unsigned POIid;
    unsigned intersection_id;
    //Inter_ID stores the closest two intersections of the POI to the starting intersections
    while(!Distance.empty()&&count<2){
        POIid=Distance.top().first;
        Distance.pop();
        intersection_id = find_closest_intersection(getPointOfInterestPosition(POIid));
            Inter_ID.push_back(intersection_id);
            count++;
        
    }
    return Inter_ID;   
}

unsigned find_closest_intersection (LatLon POI_pos){

    unsigned temp=0;
    auto iter1st = Hash_Intersection.cbegin();
    LatLon int_pos = iter1st->second.Position;              ///get the position of the first element in the hash_table
    double minD = find_distance_between_two_points(POI_pos, int_pos);       

    // find the difference between latitude and longitude
    double lat_difference_curr = abs(int_pos.lat - POI_pos.lat);
    double lon_difference_curr = abs(int_pos.lon - POI_pos.lon);

    for (auto iter_itsec = Hash_Intersection.cbegin(); iter_itsec != Hash_Intersection.cend(); iter_itsec++) {
        
        int_pos = iter_itsec->second.Position;

        double lat_difference_i = abs(int_pos.lat - POI_pos.lat);
        double lon_difference_i = abs(int_pos.lon - POI_pos.lon);

        //calculate the distance between two points only if lat difference or lon difference is smaller than current difference
        if ((lat_difference_i < lat_difference_curr) || (lon_difference_i < lon_difference_curr)) {

            double currD = find_distance_between_two_points(POI_pos, int_pos);

            if (currD <= minD){
                minD = currD;                                   //updates minimum distance
                temp = iter_itsec->second.myID;
                lat_difference_curr = lat_difference_i;
                lon_difference_curr = lon_difference_i;
            }
        }
    }
    
    return temp;
}
