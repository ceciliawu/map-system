/* 
 * File:   m3.h
 * Author: wusi9
 *
 * Created on March 6, 2015, 2:47 PM
 */

#ifndef M3_H
#define	M3_H

#pragma once
#include <vector>
#include <list>
#include <queue>
#include <string>
#include <unordered_map>
#include "m1.h"
#include <algorithm>
// Returns a path (route) between the start intersection and the end 
// intersection, if one exists. If no path exists, this routine returns 
// an empty (size == 0) vector. If more than one path exists, the path 
// with the shortest travel time is returned. The path is returned as a vector 
// of street segment ids; traversing these street segments, in the given order,
// would take one from the start to the end intersection.
std::vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end);


// Returns the time required to travel along the path specified. The path
// is passed in as a vector of street segment ids, and this function can 
// assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus 15 seconds per turn implied by the path. A turn occurs
// when two consecutive street segments have different street names.
double compute_path_travel_time(const std::vector<unsigned>& path);


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.
std::vector<unsigned> find_path_to_point_of_interest (unsigned 
                   intersect_id_start, std::string point_of_interest_name);

//Helper function to find_path_between_intersections
//return true if a legal path is found, false otherwise
//the round 1 parameter detects whether the while loop is got in the first time
bool findpath(unsigned ID, unsigned segment,unsigned intersect_id_end,
        bool round1, priority_queue<node_path, vector<node_path>, CompareCost>* waveform);

#endif	/* M3_H */

