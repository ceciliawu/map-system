#pragma once //protects against multiple inclusions of this header file

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include "StreetsDatabaseAPI.h"
#include <stdlib.h> 
#include <algorithm>
#include <set>
#include "graphics.h"
#include "node_path.h"
#include "POICompare.h"

//use these defines whenever you need earth radius 
//or conversion from degrees to radians
#define EARTH_RADIUS_IN_METERS 6372797.560856
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define zoom_level 9
//use to group streets into different level nonlinearly. It equals to factorial of zoom_level
#define Street_Divider 45

//defining a new structure for intersections
struct intersection{
    int myID;
    LatLon Position;
};

struct zoomlevel{
    
    std::vector <unsigned> level0;
    std::vector <unsigned> level1;
    std::vector <unsigned> level2;
    std::vector <unsigned> level3;
    std::vector <unsigned> level4;
    std::vector <unsigned> level5;
    std::vector <unsigned> level6;
    std::vector <unsigned> level7;
    std::vector <unsigned> level8;
};

struct zoomstreet{
     std::vector <std::string> level0;
    std::vector <std::string> level1;
    std::vector <std::string> level2;
    std::vector <std::string> level3;
    std::vector <std::string> level4;
    std::vector <std::string> level5;
    std::vector <std::string> level6;
    std::vector <std::string> level7;
    std::vector <std::string> level8;
};

//function to load bin or osm map
bool load_map(std::string map_name);

//close the loaded map
void close_map();

//function to return intersection id for an intersection name
unsigned find_intersection_id_from_name(std::string intersection_name);

//function to return street id for an intersection name
unsigned find_street_id_from_name(std::string street_name);

//function to return the street segments for a given intersection 
std::vector<unsigned> find_intersection_street_segments(std::string intersection_name); 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id);         

//function to return street names at an intersection
std::vector<std::string> find_intersection_street_names(std::string intersection_name);     
std::vector<std::string> find_intersection_street_names(unsigned intersection_id);      

//determine whether two intersections are connected through a street segment
bool are_directly_connected(std::string intersection_name1, std::string intersection_name2);

//find all intersections connected by one street segment from given intersection
std::vector<unsigned> find_adjacent_intersections(std::string intersection_name);     

//for a given street, return all the street segments
std::vector<unsigned> find_street_street_segments(std::string street_name);

//for a given street, find all the intersections
std::vector<unsigned> find_all_street_intersections(std::string street_name);

//find distance between two coordinates
double find_distance_between_two_points(LatLon point1, LatLon point2);

//find the length of a given street segments
double find_street_segment_length(unsigned street_segment_id);

//find the length of a whole street
double find_street_length(std::string street_name);

//find the travel time to drive a street segment (time(minutes) = distance(Km)/speed_limit(Km/h)*60)
double find_segment_travel_time(unsigned street_segment_id); 

//find the nearest point of interest (by name) to a given position
std::string find_closest_point_of_interest(LatLon my_position);

//find the nearest point of interest (by ID) to a given position
unsigned find_closest_point_of_interestID(LatLon my_position);

//function that calls find_closet_intersection
std::vector<unsigned> find_closest_intersection_helper (std::string POI_name,unsigned start_id);

//find the ID of nearest intersection given POI position
unsigned find_closest_intersection (LatLon POI_pos);