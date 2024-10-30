#include <iostream>
// #include <limits>
// #include <string>
#include <vector>
// #include <algorithm>
#include <map>
// #include <cmath>

using namespace std;

/**
 * Save humans, destroy zombies!
 **/

struct Point {
    int x;
    int y;
};


int main()
{
    const double ash_speed = 1000.0;
    // double min_time_diff = std::numeric_limits<double>::max();
    string next_pos = "";
    map<int, Point> humans;
    map<int, Point> zombies;
    map<int, Point> zombies_next;
    // bool Ash = false;


    // game loop
    while (1) {
        // Position de Ash
        int ash_x;
        int ash_y;
        cin >> ash_x >> ash_y; cin.ignore();
        cerr << "Ash(" << ash_x << "," << ash_y << ")" << endl;

        int human_count;
        cin >> human_count; cin.ignore();

        // Reset the map of humans (pour sortir les morts...)
        humans.clear();
        for (int i = 0; i < human_count; i++) {
            int human_id;
            int human_x;
            int human_y;
            cin >> human_id >> human_x >> human_y; cin.ignore();
            cerr << "human " << human_id << "(" << human_x << "," << human_y << ")" << endl;

            humans[human_id] = {human_x, human_y};
        }

        // Print map humans
        cerr << "affichage map humans : " << endl;
        for (const auto& entry : humans) {
            cerr << "human_id : " << entry.first << " : (" << entry.second.x << "," << entry.second.y << ")" << endl;
        }
        cerr << endl;


        int zombie_count;
        cin >> zombie_count; cin.ignore();
        for (int i = 0; i < zombie_count; i++) {
            int zombie_id;
            int zombie_x;
            int zombie_y;
            int zombie_xnext;
            int zombie_ynext;
            cin >> zombie_id >> zombie_x >> zombie_y >> zombie_xnext >> zombie_ynext; cin.ignore();
            // cerr << "zombie " << zombie_id << "(" << zombie_x << "," << zombie_y << ")" << endl;
            // cerr << "vitesse x = " << zombie_x - zombie_xnext << " | vitesse y = " << zombie_y - zombie_ynext << endl; 

            zombies[zombie_id] = {zombie_x, zombie_y};
            zombies_next[zombie_id] = {zombie_xnext, zombie_ynext};
        }

        // Print map zombies
        cerr << "affichage map zombies : " << endl;
        for (const auto& entry : zombies) {
            cerr << "zombie_id : " << entry.first << " : (" << entry.second.x << "," << entry.second.y << ")" << endl;
        }
        cerr << endl;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        if (human_count == 1) {
            auto it = humans.begin();
            next_pos = to_string(it->second.x) + " " + to_string(it->second.y);
        }
        else {
            // Déterminer les clusters d'humains
            vector<int> cluster;
            vector<vector<int>> clusters_all;
        }
        // Detecter le plus grand cluster humain et déterminer son barycentre
        cout << next_pos << endl;   
    }
}
