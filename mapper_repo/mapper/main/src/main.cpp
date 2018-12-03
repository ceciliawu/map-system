#include "StreetsDatabaseAPI.h"
#include "m2.h"
#include <iostream>

extern vector<string> city_names;
string correct_name;
#include "m4.h"
using namespace std;

int main() {
    static const size_t npos = -1;
    size_t found;
    size_t pos = 0;
    size_t n = 5;
    string name;
    
    string map_path = "/cad2/ece297s/public/maps/";
    string extension = ".bin";
    bool ok = 1;
    const char* temp;
    
    while (ok)
    {
        cout << "Enter the name of the city" << endl;

        getline(cin, name);
        
        for (unsigned i=0; i< name.length();i++)                //convert uppercase to lowercase
        {
            int value= name[i];
            if (value >= 'A' && value <= 'Z'){
           
                name[i] = char(value + 32);
            }
                
        }
        
        temp = name.c_str();

        for (unsigned i = 0; i < city_names.size(); i++) {
            found = city_names[i].find(temp, pos, n);
            if (found != npos) {
                correct_name = city_names[i];
                ok = 0;
                break;
            }
        }

        if (found == npos) {
            cout << "City name invalid"<<endl;
        }
    }
    
    map_path.append(correct_name);
    map_path.append(extension);

    cout<<map_path<<endl;
    
    draw_map(map_path);
    vector<unsigned> path;
     clock_t startTime=clock();
    path = find_path_between_intersections(53528, 63019);
    double time=compute_path_travel_time(path);
			path = find_path_between_intersections(115030, 52989);
time=compute_path_travel_time(path);
cout<<time<<endl;
			path = find_path_between_intersections(100684, 134641);
 time=compute_path_travel_time(path);
 cout<<time<<endl;
			path = find_path_between_intersections(26301, 53451);
time=compute_path_travel_time(path);
cout<<time<<endl;
			path = find_path_between_intersections(41530, 77304);
time=compute_path_travel_time(path);
cout<<time<<endl;
			path = find_path_between_intersections(53724, 63762);
time=compute_path_travel_time(path);
cout<<time<<endl;
			path = find_path_between_intersections(139218, 53000);
 time=compute_path_travel_time(path);
 cout<<time<<endl;
			path = find_path_between_intersections(97016, 145419);
     time=compute_path_travel_time(path);
     cout<<time<<endl;
			path = find_path_between_intersections(25959, 153693);
    time=compute_path_travel_time(path);
    cout<<time<<endl;
			path = find_path_between_intersections(154709, 118388);
time=compute_path_travel_time(path);cout<<time<<endl;
    
    close_map();
    return 0;
}
