/* 
 * File:   m2.h
 * Author: leimiche
 *
 * Created on February 11, 2015, 6:11 PM
 */

#ifndef M2_H
#define	M2_H
#pragma once
#include <string>
#include <cmath>
#include "m1.h"
#include "graphics.h"
#include "m3.h"
using namespace std;
// Draws the map whose at map_path; this should be a .bin file.
void draw_map(std::string map_path);
//The inner function called by draw_map. It will initialize and close the graph 
//properly and call drawscreen.
void start_map();
//Defining a new structure to store Areas depth level
struct Area_Level{
    float level1;
    float level2;
    float level3;
    float level4;
    float level5;
    float level6;
    float level7;
    float level8;
};
/*********Helper functions for start_map()*********/

//find the initial boundary of the given map
t_bound_box find_map_boundary();
/*************************************************/

//the draw screen function that is recursively called in the event_loop
void drawscreen();

/*********Help functions for drawscreen()*********/
//This function draw the full street. Need to pass in street name.
//can specify linestyle, linewidth,colour
void draw_street(int _linestyle,int _linewidth, int _colour,string name);
void draw_level0();
void draw_level1();
void draw_level2();
void draw_level3();
void draw_level4();
void draw_level5();
void draw_level6();
void draw_level7();
void draw_level8();
void draw_level_streetnames(vector<string> _zoom_level);

//this function draws the natural waters (lightblue)
void draw_natural_waters();

//this function draws the leisure(only 2 types):parks and nature reserves(limegreen)
//as well as natural type attribute grassland and wood
void draw_natural_green();


//this function draws the naturals attributes of sand and beach (yellow)
void draw_natural_yellow();

//this function draws the POI(grey)
void draw_points_of_interests();

//this function draws all the lands attributes of land(green)
void draw_natural_land();
//implement mouse click
void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info);

void draw_POI_name (const t_point& text_center, const std::string& text, float boundx, float boundy);

//returns t_bound_box for the draw text function
t_bound_box get_text_bound (LatLon start, LatLon end);

//returns the t_point text center
t_point get_text_center(LatLon start, LatLon end);

//returns the rotation angle
float get_text_rotation(LatLon start, LatLon end);
/*************************************************/
//readline auto completion function for intersection
char** intersection_command_completion(const char* stem_text, int start, int end);
//readline function that generates possible intersection names
char* intersection_name_generator(const char* stem_text, int state);
//readline auto completion function for POI
char** POI_command_completion(const char* stem_text, int start, int end);
//readline function that generates possible POI names
char* POI_name_generator(const char* stem_text, int state);
//user interface for intersection-intersection closest paths
void find_path_intersection(void(*drawscreen) (void));
//user interface for intersection-POI closest paths
void find_path_POI (void(*drawscreen) (void));
//function that draws the path
void draw_closest_path();
//function that auto zooms to the closest path
void set_zoom_level(float x1,float y1, float x2, float y2);         //auto zooms when finding closest path
#endif	/* M2_H */

