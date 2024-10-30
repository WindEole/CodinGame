#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

/**
 * Save humans, destroy zombies!
 **/

struct Point {
    int x;
    int y;

    // Opérateur d'égalité
    bool operator==(const Point& other) const {
        return (x == other.x && y == other.y);
    }
};


// Fonction pour calculer la distance euclidienne entre deux Points(x,y)
double distance(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

// Fonction pour calculer la distance euclidienne entre deux points(x1, y1, x2, y2)
double distance_coord(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

// void barycentre_of_closest_humans(map<int, Point>& humans, string& next_pos) {
//     // S'il y a moins de deux humains, on ne peut pas calculer de distances
//     if (humans.size() < 2) {
//         auto it = humans.begin();  // Se déplace vers le seul humain restant
//         next_pos = to_string(it->second.x) + " " + to_string(it->second.y);
//         return;
//     }

//     // Distance minimale trouvée pour former un groupe
//     const double distance_threshold = 4000.0;  // Seuil pour considérer les humains comme proches
//     vector<int> closest_humans_group;  // Identifiant des humains dans le sous-groupe

//     // Variables pour stocker la somme des coordonnées du groupe de proches
//     int sum_x = 0;
//     int sum_y = 0;
//     int count = 0;

//     // Trouver le groupe des humains les plus proches
//     for (auto it1 = humans.begin(); it1 != humans.end(); ++it1) {
//         for (auto it2 = next(it1); it2 != humans.end(); ++it2) {
//             double dist = distance(it1->second, it2->second);
//             if (dist < distance_threshold) {
//                 // Si la distance entre deux humains est inférieure au seuil, on les inclut
//                 if (find(closest_humans_group.begin(), closest_humans_group.end(), it1->first) == closest_humans_group.end()) {
//                     closest_humans_group.push_back(it1->first);
//                     sum_x += it1->second.x;
//                     sum_y += it1->second.y;
//                     count++;
//                 }
//                 if (find(closest_humans_group.begin(), closest_humans_group.end(), it2->first) == closest_humans_group.end()) {
//                     closest_humans_group.push_back(it2->first);
//                     sum_x += it2->second.x;
//                     sum_y += it2->second.y;
//                     count++;
//                 }
//             }
//         }
//     }

//     // Si aucun groupe de proches n'est trouvé, se déplacer vers le premier humain sauvable
//     if (closest_humans_group.empty()) {
//         auto it = humans.begin();  // Si aucun humain proche, va vers le premier humain
//         next_pos = to_string(it->second.x) + " " + to_string(it->second.y);
//     } else {
//         // Calcul du barycentre des humains les plus proches
//         int barycentre_x = sum_x / count;
//         int barycentre_y = sum_y / count;
//         next_pos = to_string(barycentre_x) + " " + to_string(barycentre_y);
//     }
// }


int main()
{
    const double ash_speed = 1000.0;
    double min_time_diff = std::numeric_limits<double>::max();
    string next_pos = "";
    map<int, Point> humans;
    map<int, Point> zombies;
    map<int, Point> zombies_next;
    bool Ash = false;


    // game loop
    while (1) {
        // Position de Ash
        int ash_x;
        int ash_y;
        cin >> ash_x >> ash_y; cin.ignore();
        cerr << "Ash(" << ash_x << "," << ash_y << ")" << endl;

        int human_count;
        cin >> human_count; cin.ignore();

        // Reset the map of humans
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

        // barycentre_of_closest_humans(humans, next_pos);

        vector<Point> sauvable_humans;
        Point target_human;  // Humain vers lequel Ash ira
        bool human_sauvable = false;

        // Boucle pour chaque humain
        for (const auto& [human_id, human] : humans) {
            // Calcul du temps pour Ash pour atteindre cet humain
            double ash_to_human_dist = distance_coord(ash_x, ash_y, human.x, human.y);
            double time_ash_to_human = ash_to_human_dist / ash_speed;

            // Boucle pour chaque zombie pour voir lequel pourrait attaquer cet humain
            for (const auto& [zombie_id, zombie] : zombies) {
                // Calcul de la distance et de la vitesse du zombie
                double zombie_to_human_dist = distance_coord(zombie.x, zombie.y, human.x, human.y);
                double zombie_speed = distance_coord(zombie.x, zombie.y, zombies_next[zombie_id].x, zombies_next[zombie_id].y);

                if (zombie_speed > 0) {  // Si le zombie bouge
                    double time_zombie_to_human = zombie_to_human_dist / zombie_speed;

                    // Comparer les temps pour voir si Ash peut arriver avant le zombie
                    if (time_ash_to_human < time_zombie_to_human) {
                        human_sauvable = true;
                        // double time_difference = time_zombie_to_human - time_ash_to_human;
                        // if (time_difference < min_time_diff) {
                        //     min_time_diff = time_difference;
                        //     target_human = human;
                        // }
                        sauvable_humans.push_back(human); // Ajouter l'humain dans la liste des humains sauvables
                    }
                }
            }
        }

        // Regrouper les humains sauvables proches
        const double distance_threshold = 4000.0;  // Seuil pour considérer les humains comme proches
        vector<Point> closest_humans_group;
        int sum_x = 0;
        int sum_y = 0;
        int count = 0;

        // Recherche de groupe des humains sauvables les plus proches
        for (size_t i = 0; i < sauvable_humans.size(); ++i) {
            for (size_t j = i + 1; j < sauvable_humans.size(); ++j) {
                if (distance(sauvable_humans[i], sauvable_humans[j]) < distance_threshold) {
                    if (find(closest_humans_group.begin(), closest_humans_group.end(), sauvable_humans[i]) == closest_humans_group.end()) {
                        closest_humans_group.push_back(sauvable_humans[i]);
                        sum_x += sauvable_humans[i].x;
                        sum_y += sauvable_humans[i].y;
                        count++;
                    }
                    if (find(closest_humans_group.begin(), closest_humans_group.end(), sauvable_humans[j]) == closest_humans_group.end()) {
                        closest_humans_group.push_back(sauvable_humans[j]);
                        sum_x += sauvable_humans[j].x;
                        sum_y += sauvable_humans[j].y;
                        count++;
                    }
                }
            }
        }

        // Si on trouve des humains proches, calculer le barycentre
        if (!closest_humans_group.empty()) {
            int barycentre_x = sum_x / count;
            int barycentre_y = sum_y / count;
            next_pos = to_string(barycentre_x) + " " + to_string(barycentre_y);
        } 
        // Si un humain sauvable est trouvé, Ash se déplace vers lui
        else if (human_sauvable) {
            next_pos = to_string(target_human.x) + " " + to_string(target_human.y);
        }
        // Si aucun humain sauvable, se déplacer vers le premier humain
        else {
            auto it = humans.begin();
            next_pos = to_string(it->second.x) + " " + to_string(it->second.y);
        }

        // if (humans.size() == 1) {
        //     auto it = humans.begin();
        //     next_pos = to_string(it->second.x) + " " + to_string(it->second.y);
        // }

        // Sécurité si next_pos reste à la position d'Ash
        if (Ash || next_pos == to_string(ash_x) + " " + to_string(ash_y) && humans.size()) {
            cerr << "ICI" << endl;
            human_sauvable = false;
            Ash = true;
            // Trouver l'humain le plus proche
            Point closest_human = humans.begin()->second;
            double closest_distance = distance_coord(ash_x, ash_y, closest_human.x, closest_human.y);
            for (const auto& [human_id, human] : humans) {
                double current_distance = distance_coord(ash_x, ash_y, human.x, human.y);
                if (current_distance < closest_distance) {
                    closest_distance = current_distance;
                    closest_human = human;
                }
            }
            next_pos = to_string(closest_human.x) + " " + to_string(closest_human.y);
        }    
        // barycentre_of_closest_humans(humans, next_pos);
        // cout << "0 0" << endl; // Your destination coordinates
        cout << next_pos << endl;     
    }
}