#include "m2.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <math.h>
//Want to use the global variable defined in m1.cpp
extern unordered_map<string, vector<unsigned>> Hash_Street;
extern unordered_map<string, intersection> Hash_Intersection;
extern unordered_map<string, vector<string>> Hash_Intersection_Streets;
extern unordered_map<string, vector<unsigned>> Hash_Intersection_Segments;
extern unordered_map<unsigned, string> Hash_Intersection_name;
extern unordered_map<unsigned, t_bound_box>Hash_POI;
extern vector<const char*> intersection_names;
extern vector<const char*> POI_names;
extern vector<unsigned> locations;
extern vector<string> POI_destination_name;
extern zoomlevel zl;
extern zoomstreet zs;
extern string correct_name;
//MUST DELETE
extern vector<node_path*> Hash_Nodes;
//Declare a global structure storing street segments drawn at different zoom-in levels
//Each Intersection corresponds to a district on the map
//vector<int> IntersectionsDistricts;
//Each districts stores all the POI that it owns
//vector<vector<unsigned>> Districts(50,vector<unsigned>(60));
//CAUTIOUS must only been set ONCE and only ONCEin start_map()
bool terminalDisplayOnce = 0;      //boolean used to make sure the text is displayed in the terminal only once
unsigned POI_destination_location; //stores the nearest intersection ID of given POI
Area_Level areas;

//Load the map and close the map properly. In between, the graphic-routine is called
void draw_map(std::string map_path){    
    load_map(map_path);
    start_map();
}

void start_map() {
    //Find the boundary box
    t_bound_box initial_boundary = find_map_boundary();
    
        
    //Initial graph
    init_graphics("Natural Landscapes Locator", WHITE);
    set_visible_world(initial_boundary);
    //Define boundary areas ONCE and ONLY ONCE!!!!!
    float initial_areas = get_visible_world().area();
    areas.level1 = initial_areas * 0.36;
    areas.level2 = initial_areas * pow(0.36, 2.0);
    areas.level3 = initial_areas * pow(0.36, 3.0);
    areas.level4 = initial_areas * pow(0.36, 4.0);
    areas.level5 = initial_areas * pow(0.36, 5.0);
    areas.level6 = initial_areas * pow(0.36, 6.0);
    areas.level7 = initial_areas * pow(0.36, 7.0);
    areas.level8 = initial_areas * pow(0.36, 8.0);

    create_button("Window", "Inter-Inter", find_path_intersection);
    create_button("Inter-Inter", "Inter-POI", find_path_POI);

    //Drawing mechanism
    drawscreen();
    event_loop(act_on_mousebutton, NULL, NULL, drawscreen);
    //Close graph properly
    close_graphics();
    return;
}

void drawscreen() {
    clearscreen();
    draw_natural_green();
    draw_natural_waters();
    draw_natural_land();
    draw_natural_yellow();
    float current_area = get_visible_world().area();
    //Draw streets and preset font size in different zoom-level level
    if (current_area > areas.level1) {
        draw_level0();
        draw_closest_path();
    } else if (current_area > areas.level2) {
        draw_level1();
        draw_closest_path();
    } else if (current_area > areas.level3) {
        draw_level2();
        draw_closest_path();
    } else if (current_area > areas.level4) {
        draw_level3();
        draw_closest_path();
    } else if (current_area > areas.level5) {
        draw_level4();
        draw_closest_path();
    } else if (current_area > areas.level6) {
        setfontsize(8);
        draw_level5();
        draw_level_streetnames(zs.level0);
        draw_closest_path();
    } else if (current_area > areas.level7) {
        setfontsize(8);
        draw_level6();
        draw_level_streetnames(zs.level0);
        draw_level_streetnames(zs.level1);
        draw_closest_path();
    } else if (current_area > areas.level8) {
        setfontsize(9);
        draw_level7();
        draw_level_streetnames(zs.level0);
        draw_level_streetnames(zs.level1);
        draw_level_streetnames(zs.level2);
        draw_level_streetnames(zs.level3);
        draw_closest_path();
    } else {
        setfontsize(9);
        draw_level8();
        draw_level_streetnames(zs.level0);
        draw_level_streetnames(zs.level1);
        draw_level_streetnames(zs.level2);
        draw_level_streetnames(zs.level3);
        draw_level_streetnames(zs.level4);
        draw_level_streetnames(zs.level5);
        draw_level_streetnames(zs.level6);
        draw_level_streetnames(zs.level7);
        draw_level_streetnames(zs.level8);
        draw_points_of_interests();
        draw_closest_path();
    }
}

/********Helper functions for start_map()*********/
t_bound_box find_map_boundary() {
    //Assume the first intersection having the largest and smallest LatLon and 
    //then iterate through the Hash_Intersection hash table and POI to find out
    //the real ones.

    double Max_Lat = Hash_Intersection.cbegin()->second.Position.lat;
    double Max_Lon = Hash_Intersection.cbegin()->second.Position.lon;
    double Min_Lat = Hash_Intersection.cbegin()->second.Position.lat;
    double Min_Lon = Hash_Intersection.cbegin()->second.Position.lon;
    //iter iterates through Hash_Intersection
    for (auto iter = Hash_Intersection.cbegin(); iter != Hash_Intersection.cend(); iter++) {
        if (iter->second.Position.lat > Max_Lat)
            Max_Lat = iter->second.Position.lat;
        else if (iter->second.Position.lat < Min_Lat)
            Min_Lat = iter->second.Position.lat;
        if (iter->second.Position.lat > Max_Lon)
            Max_Lon = iter->second.Position.lon;
        else if (iter->second.Position.lat < Min_Lon)
            Min_Lon = iter->second.Position.lon;
    }

    //i iterates through POI
    unsigned numPOI = getNumberOfPointsOfInterest();
    for (unsigned i = 0; i < numPOI; i++) {
        LatLon latlon = getPointOfInterestPosition(i);
        if (latlon.lat > Max_Lat)
            Max_Lat = latlon.lat;
        else if (latlon.lat < Min_Lat)
            Min_Lat = latlon.lat;
        if (latlon.lon > Max_Lon)
            Max_Lon = latlon.lon;
        else if (latlon.lon < Min_Lon)
            Min_Lon = latlon.lon;
    }
    return t_bound_box(Min_Lon, Min_Lat, Max_Lon, Max_Lat);
}
/*************************************************/

/********Helper functions for drawscreen()********/
void draw_street(int _linestyle, int _linewidth, int _colour, unsigned seg_id) {

    setlinestyle(_linestyle);
    setlinewidth(_linewidth);
    setcolor(_colour);

    StreetSegmentEnds ends;

    ends = getStreetSegmentEnds(seg_id);

    unsigned from = ends.from;
    LatLon point1 = getIntersectionPosition(from);
    LatLon point2;

    unsigned curve_pt_count = getStreetSegmentCurvePointCount(seg_id);

    if (curve_pt_count > 0) {
        for (unsigned curve_pt = 0; curve_pt < curve_pt_count - 1; curve_pt++) {
            point2 = getStreetSegmentCurvePoint(seg_id, curve_pt + 1);
            drawline(point1.lon, point1.lat, point2.lon, point2.lat);
            point1 = point2;
        }
    }

    unsigned to = ends.to;
    point2 = getIntersectionPosition(to);
    drawline(point1.lon, point1.lat, point2.lon, point2.lat);
}

void draw_level0() {
    //i iterates through the streets in level 0
    for(unsigned i=0;i<zl.level0.size();i++){
        draw_street(SOLID,1,BLACK,zl.level0[i]);
        }
}

void draw_level1() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, LIGHTGREY, zl.level1[i]);
}

void draw_level2() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, LIGHTGREY, zl.level2[i]);
}

void draw_level3() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level2[i]);
    //i iterates through the streets in level 3    
    for (unsigned i = 0; i < zl.level3.size(); i++)
        draw_street(SOLID, 1, LIGHTGREY, zl.level3[i]);
}

void draw_level4() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level2[i]);
    //i iterates through the streets in level 3    
    for (unsigned i = 0; i < zl.level3.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level3[i]);
    //i iterates through the streets in level 4    
    for (unsigned i = 0; i < zl.level4.size(); i++)
        draw_street(SOLID, 1, LIGHTGREY, zl.level4[i]);
}

void draw_level5() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level2[i]);
    //i iterates through the streets in level 3    
    for (unsigned i = 0; i < zl.level3.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level3[i]);
    //i iterates through the streets in level 4    
    for (unsigned i = 0; i < zl.level4.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level4[i]);
    //i iterates through the streets in level 5    
    for(unsigned i=0;i<zl.level5.size();i++)
        draw_street(SOLID,1,LIGHTGREY,zl.level5[i]);
   
}

void draw_level6() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level2[i]);
    //i iterates through the streets in level 3    
    for (unsigned i = 0; i < zl.level3.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level3[i]);
    //i iterates through the streets in level 4    
    for (unsigned i = 0; i < zl.level4.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level4[i]);
    //i iterates through the streets in level 5    
    for (unsigned i = 0; i < zl.level5.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level5[i]);
    //i iterates through the streets in level 6    
    for(unsigned i=0;i<zl.level6.size();i++)
        draw_street(SOLID,1,LIGHTGREY,zl.level6[i]); 
}

void draw_level7() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level2[i]);
    //i iterates through the streets in level 3    
    for (unsigned i = 0; i < zl.level3.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level3[i]);
    //i iterates through the streets in level 4    
    for (unsigned i = 0; i < zl.level4.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level4[i]);
    //i iterates through the streets in level 5    
    for (unsigned i = 0; i < zl.level5.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level5[i]);
    //i iterates through the streets in level 6    
    for (unsigned i = 0; i < zl.level6.size(); i++)
        draw_street(SOLID, 1, BLACK, zl.level6[i]);
    //i iterates through the streets in level 7    
    for(unsigned i=0;i<zl.level7.size();i++)
        draw_street(SOLID,1,LIGHTGREY,zl.level7[i]);

}

void draw_level8() {
    //i iterates through the streets in level 0
    for (unsigned i = 0; i < zl.level0.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level0[i]);
    //i iterates through the streets in level 1
    for (unsigned i = 0; i < zl.level1.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level1[i]);
    //i iterates through the streets in level 2
    for (unsigned i = 0; i < zl.level2.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level2[i]);
    //i iterates through the streets in level 3    
    for (unsigned i = 0; i < zl.level3.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level3[i]);
    //i iterates through the streets in level 4    
    for (unsigned i = 0; i < zl.level4.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level4[i]);
    //i iterates through the streets in level 5    
    for (unsigned i = 0; i < zl.level5.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level5[i]);
    //i iterates through the streets in level 6    
    for (unsigned i = 0; i < zl.level6.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level6[i]);
    //i iterates through the streets in level 7    
    for (unsigned i = 0; i < zl.level7.size(); i++)
        draw_street(SOLID, 1, DARKGREY, zl.level7[i]);
    //i iterates through the streets in level 8    
    for(unsigned i=0;i<zl.level8.size();i++)
        draw_street(SOLID,1,LIGHTGREY,zl.level8[i]);

}

void draw_level_streetnames(vector<string> _zoom_level) {
    //Draw street names
    setcolor(27); //FIREBRICK
    t_bound_box currentbox = get_visible_world();
    //j iterates through the street in one of the level in zl which was passed
    //in by copy constructor

    for (unsigned j = 0; j < _zoom_level.size(); j++) {
        string streetname = _zoom_level[j];
        unsigned num_of_seg = Hash_Street.at(streetname).size();
        LatLon maxStart;
        LatLon maxEnd;
        unsigned Init_from=0;
        unsigned Init_to=0;
        double maxDistance = 0;
        //i iterates through the street segments that belong to the street
        //only draw the street names once when the first segment appearing
        //on the screen is found
        if (streetname != "(unknown)") {
            //the first for loop finds the longest segment within the screen
            for (unsigned i = 1; i < num_of_seg; i++) {
                StreetSegmentEnds two_ends = getStreetSegmentEnds(Hash_Street.at(streetname)[i]);
                LatLon start = getIntersectionPosition(two_ends.from);
                LatLon end = getIntersectionPosition(two_ends.to);
                if (currentbox.left() <= (float) start.lon && (float) start.lon <= currentbox.right() &&
                        currentbox.bottom() <= (float) start.lat && (float) start.lat <= currentbox.top() &&
                        currentbox.left() <= (float) end.lon && (float) end.lon <= currentbox.right() &&
                        currentbox.bottom() <= (float) end.lat && (float) end.lat <= currentbox.top()) {
                    double distance = find_distance_between_two_points(start, end);
                    if (distance > maxDistance) {
                        maxDistance = distance;
                        maxStart.lat = start.lat;
                        maxStart.lon = start.lon;
                        maxEnd.lat = end.lat;
                        maxEnd.lon = end.lon;
                        Init_from = two_ends.from;
                        Init_to = two_ends.to;
                    }
                }
            }
            //This second for loop finds the neighbors of the longest segments
            //and change the corresponding extreme points
            for (unsigned i = 1; i < num_of_seg; i++) {
                StreetSegmentEnds two_ends = getStreetSegmentEnds(Hash_Street.at(streetname)[i]);
                if (two_ends.to == Init_from) {
                    LatLon start = getIntersectionPosition(two_ends.from);
                    maxStart.lat = start.lat;
                    maxStart.lon = start.lon;
                } else if (two_ends.from == Init_to) {
                    LatLon end = getIntersectionPosition(two_ends.to);
                    maxEnd.lat = end.lat;
                    maxEnd.lon = end.lon;
                }
            }
            if (maxDistance != 0) {
                float degrees = get_text_rotation(maxStart, maxEnd);
                settextrotation(degrees);
                t_point center = get_text_center(maxStart, maxEnd);
                drawtext(center, streetname, 10, 10);
            }
        }
    }

}

// the function that draws the natural water (lightblue)

void draw_natural_waters() { //for loop to go through each feature
    for (unsigned i = 0; i < getFeatureCount(); i++) {
        //to go through the natural type of water, checked that they are all polygons
        unsigned feature_point = getFeaturePointCount(i);
        if ((getFeatureAttribute(i, "water") != "") || (getFeatureAttribute(i, "natural") == "bay")) {
            t_point water_position[feature_point];
            //for loop to go through the attribute water, and store lat lon into a t_point array
            for (unsigned j = 0; j < feature_point; j++) {
                water_position[j].x = float(getFeaturePoint(i, j).lon);
                water_position[j].y = float(getFeaturePoint(i, j).lat);
            }
            // draw the polygon if first point and the last point are at same position
            if ((water_position[0].x == water_position[feature_point - 1].x)&&
                    (water_position[0].y == water_position[feature_point - 1].y))
 {
                setcolor(16); //LIGHTSKYBLUE
                fillpoly(water_position, feature_point);
            }                // draw multiple lines if not a polygon
            else {
                setcolor(16); //LIGHTSKYBLUE
                setlinewidth(2);
                setlinestyle(SOLID);
                //use a for loop to draw multiple line segments
                for (unsigned f_index = 0; f_index < (feature_point - 1); f_index++) {
                    drawline(water_position[f_index].x, water_position[f_index].y, water_position[f_index + 1].x, water_position[f_index + 1].y);
                }

            }


        }


        // to go through the natural feature type of waterway: attribution to be drawn: boatyard, river but not stream(too small and too many)
        if ((getFeatureAttribute(i, "waterway") != "")&&(getFeatureAttribute(i, "waterway") != "stream")) {
            t_point water_position[feature_point];
            //for loop to go through each waterway, and store lat lon into a t_point array
            for (unsigned j = 0; j < feature_point; j++) {
                water_position[j].x = float(getFeaturePoint(i, j).lon);
                water_position[j].y = float(getFeaturePoint(i, j).lat);
            }
            // draw the polygon if first point and the last point are at same position
            if ((water_position[0].x == water_position[feature_point - 1].x)&&
                    (water_position[0].y == water_position[feature_point - 1].y))
            {
                setcolor(16); //LIGHTSKYBLUE
                fillpoly(water_position, feature_point);
            }                // draw multiple lines if not a polygon
            else {
                setcolor(16); //LIGHTSKYBLUE
                if (correct_name=="newyork")
                setlinewidth(20);
                else
                setlinewidth(2);
                setlinestyle(SOLID);
                //use a for loop to draw multiple line segments
                for (unsigned f_index = 0; f_index < (feature_point - 1); f_index++) {
                    drawline(water_position[f_index].x, water_position[f_index].y, water_position[f_index + 1].x, water_position[f_index + 1].y);
                }

            }

        }

    }
}



//this functions draws all the leisure (green) and natural feature of attribute: grassland and wood

void draw_natural_green() {
    for (unsigned i = 0; i < getFeatureCount(); i++) {
        //show green for grassland,leisure,and wood
        unsigned feature_point = getFeaturePointCount(i);
        if ((getFeatureAttribute(i, "leisure") != "")
                || (getFeatureAttribute(i, "natural") == "grassland")
                || (getFeatureAttribute(i, "natural") == "wood")
                || (getFeatureAttribute(i, "natural") == "wetland")) {
            t_point green_position[feature_point];
            //for loop to go through each green feature, and store lat lon into a t_point array
            for (unsigned j = 0; j < feature_point; j++) {
                green_position[j].x = float(getFeaturePoint(i, j).lon);
                green_position[j].y = float(getFeaturePoint(i, j).lat);
            }
            // draw the polygon if first point and the last point are at same position
            if ((green_position[0].x == green_position[feature_point - 1].x)&&
                    (green_position[0].y == green_position[feature_point - 1].y))
 {
                setcolor(28); //LIMEGREEN
                fillpoly(green_position, feature_point);
            }                // draw multiple lines if not a polygon
            else {

                setcolor(28); //LIMEGREEN
                setlinewidth(2);
                setlinestyle(SOLID);
                //use a for loop to draw multiple line segments
                for (unsigned f_index = 0; f_index < (feature_point - 1); f_index++) {
                    drawline(green_position[f_index].x, green_position[f_index].y, green_position[f_index + 1].x, green_position[f_index + 1].y);
                }

            }

        }
    }
}
// this function draws the land of the map

void draw_natural_land() {
    for (unsigned i = 0; i < getFeatureCount(); i++) {
        //show linegreen for land
        unsigned feature_point = getFeaturePointCount(i);
        if ((getFeatureAttribute(i, "land")) != "") {
            t_point land_position[feature_point];
            //for loop to go through each land feature, and store lat lon into a t_point array
            for (unsigned j = 0; j < feature_point; j++) {
                land_position[j].x = float(getFeaturePoint(i, j).lon);
                land_position[j].y = float(getFeaturePoint(i, j).lat);
            }
            // draw the polygon if first point and the last point are at same position
            if ((land_position[0].x == land_position[feature_point - 1].x)&&
                    (land_position[0].y == land_position[feature_point - 1].y)) {
                setcolor(7); //GREEN
                fillpoly(land_position, feature_point);
            }                // draw multiple lines if not a polygon
            else {
                setcolor(7); //GREEN
                setlinewidth(2);
                setlinestyle(SOLID);
                //use a for loop to draw multiple line segments
                for (unsigned f_index = 0; f_index < (feature_point - 1); f_index++) {
                    drawline(land_position[f_index].x, land_position[f_index].y, land_position[f_index + 1].x, land_position[f_index + 1].y);
                }

            }

        }
    }

}
//this function draws the yellow feature of the map: natural type of attribution sand and beach

void draw_natural_yellow() {
    for (unsigned i = 0; i < getFeatureCount(); i++) {
        //show yellow for sand and beach
        unsigned feature_point = getFeaturePointCount(i);
        if ((getFeatureAttribute(i, "natural") == "sand")
                || (getFeatureAttribute(i, "natural") == "beach")) {
            t_point yellow_position[feature_point];
            //for loop to go through each yellow feature, and store lat lon into a t_point array
            for (unsigned j = 0; j < feature_point; j++) {
                yellow_position[j].x = float(getFeaturePoint(i, j).lon);
                yellow_position[j].y = float(getFeaturePoint(i, j).lat);
            }

            // draw the polygon if first point and the last point are at same position
            if ((yellow_position[0].x == yellow_position[feature_point - 1].x)&&
                    (yellow_position[0].y == yellow_position[feature_point - 1].y))
 {
                setcolor(5); //orange
                fillpoly(yellow_position, feature_point);
            }                // draw multiple lines if not a polygon
            else {
                setcolor(5); //orange
                setlinewidth(2);
                setlinestyle(SOLID);
                //use a for loop to draw multiple line segments
                for (unsigned f_index = 0; f_index < (feature_point - 1); f_index++) {
                    drawline(yellow_position[f_index].x, yellow_position[f_index].y, yellow_position[f_index + 1].x, yellow_position[f_index + 1].y);
                }

            }
        }
    }

}


//this function returns the t_point text center 

t_point get_text_center(LatLon start, LatLon end) {
    float x = (start.lon + end.lon) / 2;
    float y = (start.lat + end.lat) / 2;
    t_point center = t_point(x, y);

    return center;
}

//this function returns the angle that needs to be tilted

float get_text_rotation(LatLon start, LatLon end) {
    float deltax = start.lon - end.lon;
    float deltay = start.lat - end.lat;
    if (deltax == 0)
        return 90;
    else {
        float angle = atan(deltay / deltax) / DEG_TO_RAD;
        return angle;
    }
}


//draws POI using T_bound_box

void draw_points_of_interests() {

    unsigned long long POI_num;

    POI_num = getNumberOfPointsOfInterest();

    LatLon POI_pos;
    t_point bot_left, top_right;

    for (unsigned POI_index = 0; POI_index < POI_num; POI_index++) {

        POI_pos = getPointOfInterestPosition(POI_index);

        bot_left.x = (float) POI_pos.lon;
        bot_left.y = (float) POI_pos.lat;

        top_right.x = bot_left.x + 0.0001;
        top_right.y = bot_left.y + 0.0001;

        t_bound_box POI_region(bot_left, top_right); //sets the parameters for the rectangle

        setcolor(5); //orange
        setlinestyle(SOLID);
        fillrect(POI_region); //draws POI as rectangles
    }
}


//mouse click on POI

void act_on_mousebutton(float x, float y, t_event_buttonPressed button_info) {
    if (button_info.button == 1) {
        
        clearscreen();
        drawscreen();
        LatLon my_pos;
        unsigned POI_ID;
        t_bound_box POI_region;
        string POI_name;
        bool in_region;
        t_point box_center;

        my_pos.lon = x; //x,y coordinates are where mouse clicked
        my_pos.lat = y;

        POI_ID = find_closest_point_of_interestID(my_pos); //gets nearest POI relative to x,y position

        POI_name = getPointOfInterestName(POI_ID);

        POI_region = Hash_POI.at(POI_ID); //gets t_bound_box of the POI from hash table

        in_region = POI_region.intersects(x, y); //check whether mouse clicked within t_bound_box

        if (in_region) {

            box_center = POI_region.get_center();

            draw_POI_name(box_center, POI_name, 1000, 1000); //draws POI_name at t_bound_box center

        }
    }
    else if (button_info.button == 3) {
        
        if (locations.size() == 2){                 //resets the boolean and vectors after closet path has been drawn
            terminalDisplayOnce = 0;
            POI_destination_name.clear();
            locations.clear();
        }

        vector<unsigned> path_segment_ID;

        LatLon my_pos;
        unsigned intersection_ID;


        my_pos.lon = x; //x,y coordinates are where mouse clicked
        my_pos.lat = y;

        intersection_ID = find_closest_intersection(my_pos);        //nearest intersection ID to where the mouse clicked

        locations.push_back(intersection_ID);                       //stores the ID inside vector
        
        if (locations.size() == 2 && POI_destination_name.size() == 0){     //draws closest path when two locations are identified
            
            LatLon int_POS = getIntersectionPosition(locations[0]);
            float int_x = int_POS.lon;
            float int_y = int_POS.lat;
        
            int_POS = getIntersectionPosition(locations[1]);
            float int_x2 = int_POS.lon;
            float int_y2 = int_POS.lat;
            
            set_zoom_level(int_x,int_y, int_x2, int_y2);                //auto zooms to the draw path
        }
        drawscreen();

    }

}

//helper function for drawing POI name(function above)

void draw_POI_name(const t_point& text_center, const std::string& text, float boundx, float boundy) {

    setcolor(26);
    setfontsize(8);
    drawtext(text_center, text, boundx, boundy);
}

/************************************************/

//readline function used to auto complete POI names
char** POI_command_completion(const char* stem_text, int start, int end) {

    char ** matches = NULL;

    if (start != 0) {
        matches = rl_completion_matches(stem_text, POI_name_generator);
    }

    return matches;

}

//readline function that generates possible POI names
char* POI_name_generator(const char* stem_text, int state) {

    static int count;

    if (state == 0) {
        count = -1;
    }

    int text_len = strlen(stem_text);

    while (count < (int) POI_names.size() - 1) {

        count++;
        if (strncmp(POI_names[count], stem_text, text_len) == 0) {

            return strdup(POI_names[count]);
        }
    }

    return NULL;
}

//readline function used to auto complete intersection names
char** intersection_command_completion(const char* stem_text, int start, int end) {

    char ** matches = NULL;

    if (start != 0) {
        matches = rl_completion_matches(stem_text, intersection_name_generator);
    }

    return matches;

}

//readline function that generates possible intersection names
char* intersection_name_generator(const char* stem_text, int state) {
    //Static here means a variable's value persists across function invocations
    static int count;

    if (state == 0) {
        //We initialize the count the first time we are called
        //with this stem_text
        count = -1;
    }

    int text_len = strlen(stem_text);

    //Search through intersection_names until we find a match
    while (count < (int) intersection_names.size() - 1) {
        count++;
        if (strncmp(intersection_names[count], stem_text, text_len) == 0) {
            //Must return a duplicate, Readline will handle
            //freeing this string itself.
            return strdup(intersection_names[count]);
        }
    }

    //No more matches
    return NULL;
}

//draw closest path between two intersections interface
void find_path_intersection(void((*drawscreen) (void))) {

    terminalDisplayOnce = 0;                            //resets boolean & vectors every time button pressed
    POI_destination_name.clear();
    locations.clear();

    string command;

    cout << "Type '1' to enter intersection names directly" << endl;        
    cout << endl;
    cout << "Type '2' to enter intersection names through street names" << endl;
    cout << "prompt>";
    getline(cin, command);

    if (command == "1") {                    //the using readline to find name option

        rl_bind_key('\t', rl_complete);         //press tab to auto complete

        rl_attempted_completion_function = intersection_command_completion;

        rl_completer_quote_characters = strdup("\"\'");         //handles single and double quotes

        vector<unsigned> path_segment_ID;

        unsigned inter_ID1;
        unsigned inter_ID2;
        char* inter_name1;
        char* inter_name2;

        cout << "Type 'help' for more info" << endl;
        cout << endl;
        cout << "Enter the starting intersection name: " << endl;

        inter_name1 = readline("prompt> ");

        string command(inter_name1);

        if (command == "help") {
            cout << endl;
            cout << "Tip 1:    Must add double quotes around the intersection name" << endl;
            cout << endl;
            cout << "Tip 2:    Press 'tab' for auto completion" << endl;
            cout << endl;
            cout << "Tip 3:    First letter of street names have to be capitalized" << endl;

            rl_bind_key('\t', rl_complete);

            rl_attempted_completion_function = intersection_command_completion;

            rl_completer_quote_characters = strdup("\"\'");

            cout << "Enter the starting intersection name: " << endl;

            inter_name1 = readline("prompt> ");
        }

        string name1(inter_name1); //converts char* into string

        name1.erase(std::remove(name1.begin(), name1.end(), '\"'), name1.end()); //removes double quotes from string

        if (Hash_Intersection.find(name1) == Hash_Intersection.end()) {         //intersection not found
            cout << "intersection not found" << endl;
            return;
        }

        inter_ID1 = Hash_Intersection.at(name1).myID;           //returns ID of first intersection


        rl_bind_key('\t', rl_complete);

        rl_attempted_completion_function = intersection_command_completion;

        rl_completer_quote_characters = strdup("\"\'");

        cout << "Enter the destination intersection name: " << endl;

        inter_name2 = readline("prompt> ");

        string name2(inter_name2); //converts char* into string

        name2.erase(std::remove(name2.begin(), name2.end(), '\"'), name2.end()); //removes double quotes from string

        if (Hash_Intersection.find(name2) == Hash_Intersection.end()) {
            cout << "intersection not found" << endl;
            return;
        }

        inter_ID2 = Hash_Intersection.at(name2).myID;               //returns ID of second intersection

        locations.push_back(inter_ID1);             //stores inside vector
        locations.push_back(inter_ID2);
        
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        
        int_POS = getIntersectionPosition(locations[1]);
        float int_x2 = int_POS.lon;
        float int_y2 = int_POS.lat;
        
        set_zoom_level(int_x,int_y, int_x2, int_y2);            //auto zooms to closest path

        drawscreen();

        free(inter_name1);                                  //free variables used by readline
        inter_name1 = NULL;
        free(inter_name2);
        inter_name2 = NULL;
    } else if (command == "2") {

        int entered_times = 0;              

        while (entered_times != 2) {                        //make sure user enters twice for start and destination
            static const size_t npos = -1;
            size_t found;
            size_t pos = 0;
            string name1;
            string name2;
            string inter_name1;
            vector<string> possible_intersections;
            vector<string> resulted_intersections;

            cout << "Enter the name of the first street: ";
            getline(cin, name1);

            if ((int) name1[0] >= 'a' && (int) name1[0] <= 'z') {               //converts first letter to uppercase if it is in lowercase
                name1[0] = char((int) name1[0] - 32);
            }

            for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
                string intersectionName;
                intersectionName = getIntersectionName(i);

                found = intersectionName.find(name1, pos);
                if (found != npos) {
                    possible_intersections.push_back(intersectionName);             //store all intersection names with the given street name
                }
            }

            cout << "Enter the name of the second street: ";
            getline(cin, name2);

            if ((int) name2[0] >= 'a' && (int) name2[0] <= 'z') {                   //converts first letter to uppercase if it is in lowercase
                name2[0] = char((int) name2[0] - 32);
            }

            for (unsigned i = 0; i < possible_intersections.size(); i++) {
                found = possible_intersections[i].find(name2, pos);
                if (found != npos) {
                    resulted_intersections.push_back(possible_intersections[i]);        //stores all intersection names that has both street names
                }
            }

            if (resulted_intersections.size() == 0) {
                cout << "intersection not found" << endl;
                return;
            }

            cout << "Intersection(s) of interest" << endl;
            for (unsigned i = 0; i < resulted_intersections.size(); i++) {              //cout all possible intersections
                cout << i << "." << resulted_intersections[i] << endl;
            }

            if (entered_times == 0) {
                string num;
                unsigned index;
                cout << "Enter the number corresponding to the starting intersection: " << endl;
                cout <<"prompt> "<<endl;
                getline(cin, num);

                index = num[0] - 48; //convert from char to int                        

                inter_name1 = resulted_intersections[index];
                
            } else if (entered_times == 1) {
                string num2;
                unsigned index2;
                cout << "Enter the number corresponding to the destination intersection: " << endl;
                cout <<"prompt> "<<endl;
                getline(cin, num2);

                index2 = num2[0] - 48; //convert from char to int

                inter_name1 = resulted_intersections[index2];
            }

            if (Hash_Intersection.find(inter_name1) == Hash_Intersection.end()) {
                cout << "intersection not found" << endl;
                return;
            }

            unsigned inter_ID1 = Hash_Intersection.at(inter_name1).myID;

            locations.push_back(inter_ID1);

            entered_times++;
        }

        
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        
        int_POS = getIntersectionPosition(locations[1]);
        float int_x2 = int_POS.lon;
        float int_y2 = int_POS.lat;
        
        set_zoom_level(int_x,int_y, int_x2, int_y2);                        //autozooms to the closest path
        
        drawscreen();
    } else {
        cout << "invalid command" << endl;
    }
}





//draw path between intersection and POI interface
//functionality similar to find_path_intersection
void find_path_POI(void(*drawscreen) (void)) {

    terminalDisplayOnce = 0;                                        //resets boolean & vectors every time button pressed    
    locations.clear();
    POI_destination_name.clear();

    string command;

    cout << "Type '1' to enter intersection names directly" << endl;
    cout << endl;
    cout << "Type '2' to enter intersection names through street names" << endl;
    cout << "prompt> ";
    getline(cin, command);

    if (command == "1") {                           //the using readline to find name option

        rl_bind_key('\t', rl_complete);

        rl_attempted_completion_function = intersection_command_completion;

        rl_completer_quote_characters = strdup("\"\'");

        unsigned inter_ID;
        char* inter_name;
        char* POI_name;

        cout << "Type 'help' for more info" << endl;
        cout << endl;
        cout << "Enter the starting intersection name: " << endl;

        inter_name = readline("prompt> ");

        string command(inter_name);

        if (command == "help") {
            cout << endl;
            cout << "Tip 1:    Must add double quotes around the intersection name" << endl;
            cout << endl;
            cout << "Tip 2:    Press 'tab' for auto completion" << endl;
            cout << endl;
            cout << "Tip 3:    First letter of street/POI names have to be capitalized" << endl;

            rl_bind_key('\t', rl_complete);

            rl_attempted_completion_function = intersection_command_completion;

            rl_completer_quote_characters = strdup("\"\'");

            cout << "Enter the starting intersection name: " << endl;

            inter_name = readline("prompt> ");
        }

        string name(inter_name);

        name.erase(std::remove(name.begin(), name.end(), '\"'), name.end()); //removes double quotes from string

        if (Hash_Intersection.find(name) == Hash_Intersection.end()) {
            cout << "intersection not found" << endl;
            return;
        }

        inter_ID = Hash_Intersection.at(name).myID;


        rl_bind_key('\t', rl_complete);

        rl_attempted_completion_function = POI_command_completion;

        rl_completer_quote_characters = strdup("\"\'");

        cout << "Enter the Destination POI name: " << endl;

        POI_name = readline("prompt> ");

        string POIname(POI_name);

        POIname.erase(std::remove(POIname.begin(), POIname.end(), '\"'), POIname.end()); //removes double quotes from string

        locations.push_back(inter_ID);
        POI_destination_name.push_back(POIname);
        
        drawscreen();                                           //calls drawscreen to obtain the intersection ID closest to the POI
        
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        
        int_POS = getIntersectionPosition(POI_destination_location);
        float int_x2 = int_POS.lon;
        float int_y2 = int_POS.lat;
        
        set_zoom_level(int_x,int_y, int_x2, int_y2);            //auto zooms to closest path drawn

        drawscreen();

        free(inter_name);
        inter_name = NULL;
        free(POI_name);
        POI_name = NULL;
    } 
    
    
    else if (command == "2") {
        static const size_t npos = -1;
        size_t found;
        size_t pos = 0;
        //size_t n = 0;
        string name1;
        string name2;
        string inter_name1;
        vector<string> possible_intersections;
        vector<string> resulted_intersections;
        //const char* temp;

        cout << "Enter the name of the first street: ";
        getline(cin, name1);

        if ((int) name1[0] >= 'a' && (int) name1[0] <= 'z') {
            name1[0] = char((int) name1[0] - 32);
        }
        //temp = name1.c_str();

        for (unsigned i = 0; i < getNumberOfIntersections(); i++) {
            string intersectionName;
            intersectionName = getIntersectionName(i);

            found = intersectionName.find(name1, pos);
            if (found != npos) {
                possible_intersections.push_back(intersectionName);
            }
        }

        cout << "Enter the name of the second street: ";
        getline(cin, name2);
        if ((int) name2[0] >= 'a' && (int) name2[0] <= 'z') {
            name2[0] = char((int) name2[0] - 32);
        }
        //temp = name2.c_str();

        for (unsigned i = 0; i < possible_intersections.size(); i++) {
            found = possible_intersections[i].find(name2, pos);
            if (found != npos) {
                resulted_intersections.push_back(possible_intersections[i]);
            }
        }

        if (resulted_intersections.size() == 0) {
            cout << "intersection not found" << endl;
            return;
        }

        cout << "intersection(s) of interest" << endl;
        for (unsigned i = 0; i < resulted_intersections.size(); i++) {
            cout << i << "." << resulted_intersections[i] << endl;
        }

        string num;
        unsigned index;
        cout << "Enter the number corresponding to the starting intersection: " << endl;
        cout <<"prompt> "<<endl;
        getline(cin, num);

        index = num[0] - 48; //convert from char to int

        inter_name1 = resulted_intersections[index];

        if (Hash_Intersection.find(inter_name1) == Hash_Intersection.end()) {
            cout << "intersection not found" << endl;
            return;
        }

        unsigned inter_ID1 = Hash_Intersection.at(inter_name1).myID;

        rl_bind_key('\t', rl_complete);

        rl_attempted_completion_function = POI_command_completion;

        rl_completer_quote_characters = strdup("\"\'");

        cout << "Enter the Destination POI name: " << endl;

        const char* POI_name = readline("prompt> ");

        string POIname(POI_name);

        POIname.erase(std::remove(POIname.begin(), POIname.end(), '\"'), POIname.end()); //removes double quotes from string

        locations.push_back(inter_ID1);
        POI_destination_name.push_back(POIname);                        
        
        drawscreen();                                                   //calls drawscreen to obtain the intersection ID closest to the POI
        
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        
        int_POS = getIntersectionPosition(POI_destination_location);
        float int_x2 = int_POS.lon;
        float int_y2 = int_POS.lat;
       
        set_zoom_level(int_x,int_y, int_x2, int_y2);                    //auto zooms to closest path drawn

        drawscreen();

    } else {
        cout << "invalid command" << endl;
    }

}

//draw closest path function
//called in drawscreen()
void draw_closest_path() {
    if (locations.size() == 1 && POI_destination_name.size() == 0) {            //display the icon for starting point
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        setcolor(30);
        setfontsize(20);
        settextrotation(360);
        drawtext(int_x, int_y, "S", 1000, 1000);
    }

    if (locations.size() == 2 && POI_destination_name.size() == 0) {            //path between two intersection case

        vector<unsigned> path_segment_ID;                                       // displays the icon for destination point and draws path
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        setcolor(30);
        setfontsize(20);
        settextrotation(360);
        drawtext(int_x, int_y, "S", 1000, 1000);

        LatLon int_POS2 = getIntersectionPosition(locations[1]);
        float int_x2 = int_POS2.lon;
        float int_y2 = int_POS2.lat;
        setcolor(30);
        setfontsize(20);
        settextrotation(360);
        drawtext(int_x2, int_y2, "D", 1000, 1000);

        path_segment_ID = find_path_between_intersections(locations[0], locations[1]);
        
        if (path_segment_ID.size() == 0){                                               //returns error message if no route possible
            if (!terminalDisplayOnce){
             cout<<"no route found"<<endl;
             terminalDisplayOnce = 1;
            }
             return;

        }
           
        for (unsigned i = 0; i < path_segment_ID.size(); i++) {
            draw_street(SOLID, 5, 25, path_segment_ID[i]);
        }
        
        if (!terminalDisplayOnce) {                                                             //displays in terminal the driving directions as well as time
     
            unsigned streetID = getStreetSegmentStreetID(path_segment_ID[0]);
            string streetName = getStreetName(streetID);
            cout << "Driving directions: " << endl;
            cout << "   " << "Drive along " << streetName << endl;

            for (unsigned i = 0; i < path_segment_ID.size(); i++) {
                if (i != path_segment_ID.size() - 1) {

                    unsigned stID_curr = getStreetSegmentStreetID(path_segment_ID[i]);
                    unsigned stID_next = getStreetSegmentStreetID(path_segment_ID[i + 1]);

                    if (stID_curr != stID_next) {
                        streetName = getStreetName(stID_next);
                        cout << "   " << "Make a turn and drive along " << streetName << endl;
                    }
                }
            }

            double travelTime = compute_path_travel_time(path_segment_ID);
            cout<<endl;
            cout << "Estimated travel time: " << travelTime << " minutes" << endl;
            terminalDisplayOnce = 1;
        }
    }
    
    else if (locations.size() == 1 && POI_destination_name.size() == 1) {                   //case of closest path between intersection and POI
        
        vector<unsigned> path_segment_ID;
        StreetSegmentEnds segmentEnds;
        LatLon int_POS = getIntersectionPosition(locations[0]);
        float int_x = int_POS.lon;
        float int_y = int_POS.lat;
        setcolor(30);
        setfontsize(20);
        settextrotation(360);
        drawtext(int_x, int_y, "S", 1000, 1000);

        path_segment_ID = find_path_to_point_of_interest(locations[0], POI_destination_name[0]);

        if (path_segment_ID.size() == 0)
            return;
        
        segmentEnds = getStreetSegmentEnds(path_segment_ID[path_segment_ID.size() - 1]);
        
        POI_destination_location = segmentEnds.to;                                          //stores the ID of the closest intersection to given POI
        
        LatLon int_POS2 = getIntersectionPosition(segmentEnds.to);
        float int_x2 = int_POS2.lon;
        float int_y2 = int_POS2.lat;
        setcolor(30);
        setfontsize(20);
        settextrotation(360);
        drawtext(int_x2, int_y2, "D", 1000, 1000);

        for (unsigned i = 0; i < path_segment_ID.size(); i++) {                     //draws path
            draw_street(SOLID, 5, 25, path_segment_ID[i]);
        }

        if (!terminalDisplayOnce) {                                                     //displays driving instructions and travel time
            unsigned streetID = getStreetSegmentStreetID(path_segment_ID[0]);
            string streetName = getStreetName(streetID);
            cout << "Driving directions: " << endl;
            cout << "   " << "Drive along " << streetName << endl;

            for (unsigned i = 0; i < path_segment_ID.size(); i++) {
                if (i != path_segment_ID.size() - 1) {

                    unsigned stID_curr = getStreetSegmentStreetID(path_segment_ID[i]);
                    unsigned stID_next = getStreetSegmentStreetID(path_segment_ID[i + 1]);

                    if (stID_curr != stID_next) {
                        streetName = getStreetName(stID_next);
                        cout << "   " << "Make a turn and drive along " << streetName << endl;
                    }
                }
            }

            double travelTime = compute_path_travel_time(path_segment_ID);
            cout<<endl;
            cout << "Estimated travel time: " << travelTime << " minutes" << endl;
            terminalDisplayOnce = 1;
        }
    }
}

void set_zoom_level(float x1,float y1, float x2, float y2){
    
    float midX, midY;
    double halfwayD;
    LatLon midpoint,point1;
    
    midX = (x1+x2)/2;
    midY = (y1+y2)/2;
    
    midpoint.lon = midX;            //find midpoint location
    midpoint.lat = midY;
    
    point1.lon = x1;        
    point1.lat = y1;
    
    halfwayD = sqrt((x1-midX)*(x1-midX)+(y1-midY)*(y1-midY));       //calculate the distance between midpoint to any of the rear points
    
    set_visible_world(midX-halfwayD, midY-halfwayD, midX+halfwayD, midY+halfwayD);          //set the t_bound box parameters
    
}
