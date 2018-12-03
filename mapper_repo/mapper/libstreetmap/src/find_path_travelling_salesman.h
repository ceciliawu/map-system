/* 
 * File:   find_path_travelling_salesman.h
 * Author: leimiche
 *
 * Created on March 30, 2015, 1:03 AM
 */

#ifndef FIND_PATH_TRAVELLING_SALESMAN_H
#define	FIND_PATH_TRAVELLING_SALESMAN_H
#include "m3.h"
std::vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end,vector<node_path>& Hash_Nodes_T);
bool findpath(unsigned ID, unsigned segment,unsigned intersect_id_end,
        bool round1, priority_queue<node_path, vector<node_path>, CompareCost>* waveform,vector<node_path>& Hash_Nodes_T);


#endif	/* FIND_PATH_TRAVELLING_SALESMAN_H */

