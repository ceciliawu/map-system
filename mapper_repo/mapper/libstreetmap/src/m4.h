/* 
 * File:   m4.h
 * Author: leimiche
 *
 * Created on March 24, 2015, 4:03 PM
 */

#ifndef M4_H
#define	M4_H
#pragma once
#include <vector>
#include <pthread.h>
#include "m3.h"
#include <algorithm>
#include <time.h>
#include "find_path_travelling_salesman.h"

    // This routine takes in a vector of N intersections that must be 
    // traversed in a path, and another vector of M intersections that 
    // are legal start and end points for the path. The first vector 
    // gives the intersection ids of a set of locations at which 
    // deliveries must be made. The second vector gives the intersection
    // ids of courier company depots containing trucks; you start at any
    // one of these depots and end at any one of the depots.
    // You can assume that N is always at least 1, and M is always at 
    // least one (i.e. both input vectors are non-empty).  You can also
    // assume that no intersection is repeated in either list and that
    // no intersection is both a delivery location and a depot (i.e. no
    // intersection will appear in both vectors).
    // This routine returns a vector of street segment ids that form a 
    // path, where the first street segment id is connected to a depot
    // intersection, and the last street segment id also connects to a
    // depot intersection.  If no path connecting all the delivery locations
    // and a start and end depot exists, this routine should return an 
    // empty (size == 0) vector.
struct thread_data{
    std::vector<unsigned> intersections_to_traverse;
    std::vector<unsigned> depot_locations;
    std::vector<unsigned> path;
    double traveltime;
    std::vector<unsigned> seg_path;
    clock_t starttime;
    vector<node_path> Hash_Nodes_T;
};
    std::vector<unsigned> traveling_salesman(std::vector<unsigned> intersections_to_traverse, std::vector<unsigned> depot_locations);
    //by Michelle
    void* greedy_algorithm1(void *threadarg);
    //by Cecilia
    void* greedy_algorithm2(void *threadarg);
    void twoopt(struct thread_data * my_data,unordered_map<unsigned,vector<double>>& paths,
                unordered_map<unsigned,unsigned>& IDtoIndex,
                vector<unsigned> intersections_to_traverse);
    void twoopt2(struct thread_data * my_data,unordered_map<unsigned,vector<double>>& paths,
                unordered_map<unsigned,unsigned>& IDtoIndex,
                vector<unsigned> intersections_to_traverse);
    void add_depo(std::vector<unsigned>& chosen_path, std::vector<unsigned> depot_locations,clock_t startTime);
        //genetic algorithm
    void* genetic_algorithm(void *threadarg);
 

#endif	/* M4_H */
