#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <cstdlib> // pour rand()
#include <ctime>   // pour srand()
#include <limits>

using namespace std;

struct Point {
    int x;
    int y;

    // Opérateur d'égalité
    bool operator==(const Point& other) const {
        return (x == other.x && y == other.y);
    }
};

double distance(const Point& a, const Point& b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

Point find_closest_human(const Point& ash_pos, const map<int, Point>& humans) {
    double min_dist = numeric_limits<double>::max();
    Point closest_human_pos = {0, 0};  // Initialisation par défaut

    for (const auto& human : humans) {
        double dist = distance(ash_pos, human.second);
        if (dist < min_dist) {
            min_dist = dist;
            closest_human_pos = human.second;
        }
    }

    return closest_human_pos;
}

Point generate_random_position(const Point& ash_pos, double max_distance) {
    // Générer une position aléatoire autour d'Ash dans un rayon de `max_distance`
    double angle = (rand() % 360) * (M_PI / 180.0); // Angle en radians
    double distance = (rand() % (int)max_distance);
    
    int new_x = ash_pos.x + (int)(distance * cos(angle));
    int new_y = ash_pos.y + (int)(distance * sin(angle));
    
    return {new_x, new_y};
}

int evaluate_scenario(const Point& ash_pos, const map<int, Point>& humans, const map<int, Point>& zombies, const Point& next_ash_pos, double ash_speed) {
    int score = 0;
    const double ash_attack_range = 2000.0;  // Rayon d'action d'Ash
    const double zombie_speed = 400.0;  // Vitesse fixe des zombies

    // Critère 1 : Sauver un humain

    // double min_dist_to_human = numeric_limits<double>::max();
    // for (const auto& human : humans) {
    //     double dist = distance(next_ash_pos, human.second);
    //     if (dist < min_dist_to_human) {
    //         min_dist_to_human = dist;
    //     }
    // }
    // score += (10000 - (int)min_dist_to_human); // Plus c'est proche d'un humain, meilleur est le score

    // // Parcourir chaque humain et estimer leur risque basé sur la proximité des zombies
    // for (const auto& human : humans) {
    //     int human_id = human.first;
    //     const Point& human_pos = human.second;
    //     // Trouver le zombie le plus proche de cet humain
    //     double min_dist_to_zombie = numeric_limits<double>::max();
    //     for (const auto& zombie : zombies) {
    //         double dist_human_to_zombie = distance(human_pos, zombie.second);
    //         if (dist_human_to_zombie < min_dist_to_zombie) {
    //             min_dist_to_zombie = dist_human_to_zombie;
    //         }
    //     }
    //     // cerr << "dist to human = " << min_dist_to_zombie << endl;
    //     // Calculer le nombre de tours avant que le zombie n'atteigne l'humain
    //     const double zombie_speed = 400.0;  // Vitesse fixe des zombies, à adapter si elle change
    //     double turns_before_human_dead = min_dist_to_zombie / zombie_speed;

    //     // Calculer le temps que Ash mettrait pour atteindre cet humain
    //     double dist_ash_to_human = distance(next_ash_pos, human_pos);

    //     // double turns_for_ash_to_reach = dist_ash_to_human / ash_speed;
    //     // Si Ash peut sauver l'humain directement (dans son rayon d'action)
    //     if (dist_ash_to_human <= ash_attack_range) {
    //         score += 10000;  // Bonus élevé pour sauver un humain
    //     }
    //     else if (turns_before_human_dead < 0 && dist_ash_to_human / ash_speed < turns_before_human_dead) {
    //         // Si Ash n'est pas encore à portée mais peut atteindre l'humain avant qu'il ne soit tué
    //         score += 10000;  // Bonus si Ash peut atteindre et sauver l'humain au tour suivant
    //     } else {
    //         // Si l'humain est "condamné" (Ash ne peut pas le sauver)
    //         score -= 100000;  // Ne pas gaspiller d'efforts sur un humain perdu
    //     }
    // }

    // Parcourir chaque humain pour évaluer la distance et le risque
    for (const auto& human : humans) {
        const Point& human_pos = human.second;

        // Calculer la distance d'Ash à l'humain
        double dist = distance(next_ash_pos, human_pos);
        double min_dist_to_zombie = numeric_limits<double>::max();

        // Trouver le zombie le plus proche de cet humain
        for (const auto& zombie : zombies) {
            double dist_human_to_zombie = distance(human_pos, zombie.second);
            if (dist_human_to_zombie < min_dist_to_zombie) {
                min_dist_to_zombie = dist_human_to_zombie;
            }
        }

        // Calculer le nombre de tours avant que le zombie n'atteigne l'humain
        double turns_before_human_dead = min_dist_to_zombie / zombie_speed;

        // Vérifier si Ash peut atteindre l'humain avant qu'il ne soit tué
        double turns_for_ash_to_reach = (dist - 2000) / ash_speed;

        if (turns_for_ash_to_reach < turns_before_human_dead) {
            // L'humain est sauvable, bonus selon la distance
            score += (100000 - static_cast<int>(dist));  // Bonus pour la proximité
        } else {
            // L'humain est condamné, appliquer un malus
            score -= 5000;  // Malus pour essayer de sauver un humain qui ne peut pas être sauvé
        }
    }


    // Critère 2 : Proximité avec le zombie le plus proche
    double min_dist_to_zombie = numeric_limits<double>::max();
    for (const auto& zombie : zombies) {
        double dist = distance(next_ash_pos, zombie.second);
        if (dist < min_dist_to_zombie) {
            min_dist_to_zombie = dist;
        }
    }
    score += (5000 - (int)min_dist_to_zombie); // Bonus pour se rapprocher des zombies
    // if (score > 0)
    //     cerr << "score = " << score << endl;

    // Critère 3 : Abandonner les humains impossible à sauver
    // double min_dist_to_abandon = numeric_limits<double>::max();
    // for (const auto& human : humans) {
    //     double dist = distance(next_ash_pos, human.second);
    //     if (dist < min_dist_to_human) {
    //         min_dist_to_human = dist;
    //     }
    // }
    // score -= (10000 - (int)min_dist_to_human);

    // if (score < 0)
    //     cerr << "score = " << score << endl;

    return score;
}

// int humans_equidistant_to_ash(map<int, Point>& humans, Point& ash_pos) {
//     cerr << "On entre ici ?" << endl;
//     if (humans.empty()) return 0;  // S'il n'y a pas d'humains, retourner 0

//     auto it = humans.begin();
//     double dist_init = distance(ash_pos, it->second);
//     cerr << "dist_init = " << dist_init << endl;

//     for (const auto& human : humans) {
//         double dist = distance(ash_pos, human.second);
//         cerr << dist << endl;
//         if (dist != dist_init)
//             return 0; // Si une distance est différente, ils ne sont pas tous équidistants
//     }
//     return 1; // Tous les humains sont équidistants d'Ash
// }

bool are_humans_few_and_disperse(const map<int, Point>& humans) {
    const int human_count_threshold = 5;  // Moins de 5 humains
    const double dispersion_threshold = 2000.0;  // Distance minimale pour être considéré comme "dispersé"

    // 1. Vérifier si les humains sont peu nombreux
    if (humans.size() >= human_count_threshold) {
        return false;  // Il y a suffisamment d'humains
    }

    // 2. Vérifier si les humains sont très dispersés
    for (auto it1 = humans.begin(); it1 != humans.end(); ++it1) {
        for (auto it2 = next(it1); it2 != humans.end(); ++it2) {
            double dist = distance(it1->second, it2->second);
            if (dist < dispersion_threshold) {
                return false;  // Les humains ne sont pas suffisamment dispersés
            }
        }
    }

    return true;  // Peu nombreux et dispersés
}

Point find_closest_savable_human(const Point& ash_pos, const map<int, Point>& humans, const map<int, Point>& zombies, double ash_speed) {
    const double zombie_speed = 400.0;
    Point closest_human = {-1, -1};  // Valeur par défaut si aucun humain n'est sauvable
    double min_dist_to_human = numeric_limits<double>::max();

    for (const auto& human : humans) {
        const Point& human_pos = human.second;

        // Trouver le zombie le plus proche de cet humain
        double min_dist_to_zombie = numeric_limits<double>::max();
        for (const auto& zombie : zombies) {
            double dist_human_to_zombie = distance(human_pos, zombie.second);
            if (dist_human_to_zombie < min_dist_to_zombie) {
                min_dist_to_zombie = dist_human_to_zombie;
            }
        }

        // Calculer le nombre de tours avant que le zombie n'atteigne cet humain
        double turns_before_human_dead = min_dist_to_zombie / zombie_speed;

        // Calculer le temps que Ash mettrait pour atteindre cet humain
        double dist_ash_to_human = distance(ash_pos, human_pos);
        double turns_for_ash_to_reach = dist_ash_to_human / ash_speed;

        // Si Ash peut atteindre l'humain avant qu'il ne soit tué, et que c'est le plus proche
        if (turns_for_ash_to_reach <= turns_before_human_dead && dist_ash_to_human < min_dist_to_human) {
            min_dist_to_human = dist_ash_to_human;
            closest_human = human_pos;
        }
    }

    return closest_human;  // Retourne les coordonnées de l'humain le plus proche et sauvable
}

int main() {
    srand(time(0)); // Initialiser la graine pour les nombres aléatoires

    const double ash_speed = 1000.0;
    string next_pos = "";
    map<int, Point> humans;
    map<int, Point> zombies;

    while (1) {
        // Position de Ash
        int ash_x, ash_y;
        cin >> ash_x >> ash_y;
        cin.ignore();
        Point ash_pos = {ash_x, ash_y};
        cerr << "Ash("<< ash_x << "," << ash_y << ")" << endl; 
        int human_count;
        cin >> human_count;
        cin.ignore();

        humans.clear();
        for (int i = 0; i < human_count; i++) {
            int human_id, human_x, human_y;
            cin >> human_id >> human_x >> human_y;
            cin.ignore();
            humans[human_id] = {human_x, human_y};
        }

        // Print map humans
        cerr << "affichage map humans : " << endl;
        for (const auto& entry : humans) {
            cerr << "human_id : " << entry.first << " : (" << entry.second.x << "," << entry.second.y << ")" << endl;
        }
        cerr << endl;

        int zombie_count;
        cin >> zombie_count;
        cin.ignore();

        zombies.clear();
        for (int i = 0; i < zombie_count; i++) {
            int zombie_id, zombie_x, zombie_y, zombie_xnext, zombie_ynext;
            cin >> zombie_id >> zombie_x >> zombie_y >> zombie_xnext >> zombie_ynext;
            cin.ignore();
            zombies[zombie_id] = {zombie_x, zombie_y};
        }

        // Print map zombies
        cerr << "affichage map zombies : " << endl;
        for (const auto& entry : zombies) {
            cerr << "zombie_id : " << entry.first << " : (" << entry.second.x << "," << entry.second.y << ")" << endl;
        }
        cerr << endl;

/////////////////////////////////////////////////////////////////

        if (human_count == 1) {
            auto it = humans.begin();
            next_pos = to_string(it->second.x) + " " + to_string(it->second.y);
        }
        // else if(humans_equidistant_to_ash(humans, ash_pos)) {
        //     cerr << "On entre ici ?" << endl;
        //     Point closest_human_pos = find_closest_human({ash_x, ash_y}, humans);
        //     next_pos = to_string(closest_human_pos.x) + " " + to_string(closest_human_pos.y);
        // }
        else if (are_humans_few_and_disperse(humans)) {
            // Si les humains sont peu nombreux et dispersés, Ash priorise le plus proche et sauvable
            Point closest_savable_human = find_closest_savable_human({ash_x, ash_y}, humans, zombies, ash_speed);
            if (closest_savable_human.x != -1) {
                next_pos = to_string(closest_savable_human.x) + " " + to_string(closest_savable_human.y);
            } else {
                // Si aucun humain n'est sauvable, Ash peut se diriger vers le plus proche, même condamné
                Point closest_human_pos = find_closest_human({ash_x, ash_y}, humans);
                next_pos = to_string(closest_human_pos.x) + " " + to_string(closest_human_pos.y);
            }
        } 
        else {
            // Monte Carlo simulation
            int best_score = -1;
            Point best_pos = {ash_x, ash_y};

            for (int i = 0; i < 100; ++i) { // Générer 100 scénarios aléatoires
                Point simulated_ash_pos = generate_random_position(ash_pos, ash_speed);
                int score = evaluate_scenario(ash_pos, humans, zombies, simulated_ash_pos, ash_speed);
                if (score > best_score) {
                    best_score = score;
                    best_pos = simulated_ash_pos;
                }
            }
            cerr << best_score << endl;
            next_pos = to_string(best_pos.x) + " " + to_string(best_pos.y);
            if (best_score == -1) {
                Point closest_human_pos = find_closest_human({ash_x, ash_y}, humans);
                next_pos = to_string(closest_human_pos.x) + " " + to_string(closest_human_pos.y);
            }
        }

        cout << next_pos << endl;
    }
}
