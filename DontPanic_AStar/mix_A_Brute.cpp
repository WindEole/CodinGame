#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int addElev;
int noElev = 0;
string firstCloneDir = "";
string pathCloneDir = "";

void print(map<int, vector<int>> elevator) {
    // Affichage du contenu de la map
    cerr << "MAP : " << endl; 
    for (const auto& entry : elevator) {
        cerr << "Étage : " << entry.first << " : ";
        for (int pos : entry.second) { //-> affichage du vecteur
            cerr << pos << " ";
        }
        cerr << endl;
    }
    cerr << endl;
}

int findClosestValue(const vector<int>& vec, int target) {
    // Vérifier si le vecteur est vide
    // if (vec.empty()) {
    //     std::cerr << "Le vecteur est vide." << std::endl;
    //     return -1; // Valeur invalide pour indiquer une erreur
    // }
    // Initialiser la valeur la plus proche avec le premier élément du vecteur
    int closest = vec[0];
    int minDiff = abs(target - closest); // Calculer la différence absolue entre la cible et la valeur la plus proche

    // Parcourir le vecteur pour trouver la valeur la plus proche
    for (int value : vec) {
        int diff = abs(target - value); // Calculer la différence absolue entre la cible et la valeur actuelle
        if (diff < minDiff) {
            minDiff = diff; // Mettre à jour la différence minimale
            closest = value; // Mettre à jour la valeur la plus proche
        }
    }
    return closest;
}


// ------------------- EN DESSOUS VALABLE POUR LES 5 PREMIERS TESTS ---------------- //

// string check_if_dir_is_ok(int clone_pos, int goal) {
//     if (clone_pos < goal)
//         return "RIGHT";
//     else if (clone_pos > goal)
//         return "LEFT";
//     return "NONE";
// }

// string selectAction(int start_pos, int goal, string direction) {
//     string good_direction;
//     good_direction = check_if_dir_is_ok(start_pos, goal);
//     cerr << "good_Direction = " << good_direction << endl;
//     if (direction != good_direction && good_direction != "NONE") {
//         return "BLOCK";
//     } else {
//         return "WAIT";
//     } 
// }

// ------------------- AU DESSUS VALABLE POUR LES 5 PREMIERS TESTS ---------------- //

// string selectAction(string instruction, string direction) {
//     if (direction != instruction) {
//         return "BLOCK";
//     } else {
//         return "WAIT";
//     } 
// }

string selectAction(string instruction, string direction) {
    if (direction != instruction) {
        return "WAIT WAIT BLOCK";
    } else {
        return "WAIT";
    } 
}

// ---------------------- ALGO A* ------------------------ // 

// Cette structure et le vecteur direction permettent de tester les points autour du perso
struct Point {
    int x;
    int y;

    // Surcharge de l'opérateur < pour la comparaison des points (sinon, impossible de l'utiliser comme clé dans la map)
    bool operator<(const Point& other) const {
        // Comparaison d'abord sur l'axe x, puis sur l'axe y si x est égal
        if (x == other.x) {
            return y < other.y;
        }
        return x < other.x;
    }

    // Surcharge de l'opérateur != pour comparer deux points
    bool operator!=(const Point& other) const {
        return (x != other.x) || (y != other.y);
    }
};

vector<Point> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; // UP DOWN RIGHT, LEFT, 

struct Noeud {
    float cout_g; // le cout pour aller du point de départ au nœud considéré
    float cout_h; // le cout pour aller du nœud considéré au point de destination
    float cout_f; // somme des précédents, mais mémorisé pour ne pas le recalculer à chaque fois
    Point parent; // le parent, représenté par ses coordonnées
};

typedef map<Point, Noeud> l_noeud;

// calcul des distances
float distance(int x1, int y1, int x2, int y2) {
    // Euclidian distance -> A privilégier si on utilise les mouvements en diagonale
    // float scale = 1.0f;
    // int dx = abs(x1 - x2);
    // int dy = abs(y1 - y2);
    // return scale * sqrt(dx * dx + dy * dy);
    // return sqrt((x1-x2) * (x1-x2) + (y1-y2) * (y1-y2));
    // Manhattan distance -> A privilégier si on n'a que 4 mouvements up/down/right/left
    float scale = 1.0f; // -> scale must match the cost function
    int dx = abs(x1 - x2);
    int dy = abs(y1 - y2);
    return scale * (dx + dy);
}

// Cette fonction s'assure que le point que l'on teste est bien dans le maze et que l'on a le droit d'aller dessus : '.' / 'D' / 'E' / 'S'
bool isValid(const Point& goal, const Point& current, const Point& target, const Point& dir, int rows, int cols, const vector<string>& maze, const map<int, vector<int>>& elevator) {
    // cerr << "Is Valid ? dir.x = " << dir.x << "goal.x = " << goal.x << "current.x  = " << current.x << endl;

    // ATTENTION ! x = floors (rows) / y = pos (cols) !!!

    if (goal.x < 0 || goal.x > rows - 1 || goal.y <= 0 || goal.y > cols - 1 || maze[goal.x][goal.y] == 'x') { // on exclut les pos 0 et cols-1 -> clone détruit
        // cerr << "ICI ? goal.x = " << goal.x << " rows = " << rows << " goal.y = " << goal.y << " cols = " << cols << endl;
        return false;
    }

    if (dir.x == 0 && maze[current.x][current.y] == 'E') //-> on est sur un elevator donc on doit changer de floor !
        return false;

    if (dir.x == -1) //-> on essaie d'aller vers le bas : NON !
        return false;

    if (dir.x == 1) { //-> on essaie d'aller vers le haut
    // cerr << "ICI" << endl;
        if (goal.x > target.x) { // on n'a pas besoin de monter au-delà du floor exit
            return false;
        } else if (goal.x < target.x) { // Si notre point à tester est en dessous du niveau de sortie
            auto it = elevator.find(current.x); // On cherche si on a un ascenseur au current level
            // le niveau current a un ou pls ascenseur mais le current pos n'est pas un E et on ne peut pas en construire un
            if (!it->second.empty() && maze[current.x][current.y] != 'E' && !addElev)
                return false;
        }
        // else if (goal.x == target.y) { // si notre point à tester est au niveau de la sortie
        //     // s'il y a ascenseurs au niveau du exitFloor : pb !
        //     auto it = elevator.find(target.x);
        //     if (!it->second.empty()) {
        //         const vector<int> elevExitFloor = it->second;
        //         for (size_t i = 0; i < elevExitFloor.size() - 1; ++i) {
        //             if (target.x >= elevExitFloor[i] && target.x <= elevExitFloor[i + 1]) {
        //                 if (goal.x == i || goal.x == i + 1)
        //                     return true;
        //                 else
        //                     return false;
        //             }
        //         }
        //     }
        // } 
    } 



// goal.x >= 0 && goal.x < rows && goal.y >= 0 && goal.y < cols
    return true;
}

// ------------- RECONSTRUCT PATH SAVE -------------- // 

// // Fonction pour reconstruire le chemin optimal une fois la cible trouvée
// vector<string> reconstructPath(const Point& start, const Point& target, const map<Point, Point>& came_from) {
//     Point current = target;
//     vector<string> instructions;
//     // cerr << "reconstruct path" << endl;

//     // Remonter à travers les parents à partir de la cible jusqu'au point de départ
//     while (current != start) {
//         // Ajouter l'instruction de mouvement pour aller du parent au nœud actuel
//         Point parent = came_from.at(current);

//         if (parent.x < current.x) {
//             // cerr << "on monte" << endl;
//             instructions.push_back("UP");
//         } else if (parent.y < current.y) {
//             // cerr << "on va à droite ?" << endl;
//             instructions.push_back("RIGHT");
//         } else if (parent.y > current.y) {
//             // cerr << "on va à gauche ?" << endl;
//             instructions.push_back("LEFT");
//         }

//         // Mettre à jour le nœud actuel avec son parent
//         current = parent;
//     }

//     cerr << "reconstruct path = ";
//     for (const string& instruction : instructions) {
//         cerr << instruction << " ";
//     }
//     cerr << endl;
//     cerr << "nb d'instructions dans le path : " << instructions.size() << endl << endl;

//     return instructions;
// }

// ------------- RECONSTRUCT PATH SAVE -------------- //

vector<string> translateInstructions(vector<string>& instructions, const map<Point, Point>& came_from, const vector<string>& maze, const Point& start, const Point& target) {

    Point current = start;
    // On parcours le vecteur en sens inverse
    for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
        if (*it == "UP") {
            if (maze[current.x][current.y] != 'E' && addElev > 0) {

            }
        }
    }

    while (current != target) {
        // Ajouter l'instruction de mouvement pour aller du parent au nœud actuel
        Point parent = came_from.at(current);

        if (parent.x < current.x) {
            // cerr << "on monte" << endl;
            instructions.push_back("UP");
        } else if (parent.y < current.y) {
            // cerr << "on va à droite ?" << endl;
            instructions.push_back("RIGHT");
        } else if (parent.y > current.y) {
            // cerr << "on va à gauche ?" << endl;
            instructions.push_back("LEFT");
        }

        // Mettre à jour le nœud actuel avec son parent
        current = parent;
    }





    // if instructions = UP 
//         if (parent.x < current.x) {
//             // cerr << "on monte" << endl;
//             instructions.push_back("UP");

//             if (maze[current.x][current.y] != 'E' && addElev > 0) {
//                 // actions.push_back("WAIT");
//                 actions.push_back("WAIT");
//                 actions.push_back("WAIT");
//                 actions.push_back("WAIT");
//                 actions.push_back("ELEVATOR");
//             } else {
//                 actions.push_back("WAIT");
//             }
//             inst++;
//             cerr << inst << " | UP " << endl;


//         } else if (parent.y < current.y) {
//             // cerr << "on va à droite ?" << endl;
//             instructions.push_back("RIGHT");

//             action = selectAction("RIGHT", pathCloneDir);
//             // cerr << "select action = " << action << endl;
//             if (action != "WAIT") {
//                 pathCloneDir = "RIGHT";
//                 // actions.push_back("WAIT");
//                 // actions.push_back("WAIT");
//             }
//             actions.push_back(action);
//             inst++;
//             cerr << inst << " | RIGHT " << action << endl;


//         } else if (parent.y > current.y) {
//             // cerr << "on va à gauche ?" << endl;
//             instructions.push_back("LEFT");

//             // cerr << "Clone direction = " << pathCloneDir << endl;
//             action = selectAction("LEFT", pathCloneDir);
//             // cerr << action << endl;
//             actions.push_back(action);
//             if (action != "WAIT") {
//                 pathCloneDir = "LEFT";
//                 // actions.push_back("BLOCK");
//                 // actions.push_back("WAIT");
//                 // actions.push_back("WAIT");
//             }

//             // cerr << "Clone direction after action = " << pathCloneDir << endl;
//             inst++;
//             cerr << inst << " | LEFT " << action << endl;
//         }

//         // Mettre à jour le nœud actuel avec son parent
//         current = parent;
//     }

//     cerr << "reconstruct path = ";
//     for (const string& instruction : instructions) {
//         cerr << instruction << " ";
//     }
//     cerr << endl;
//     cerr << "nb d'instructions dans le path : " << instructions.size() << endl << endl;


//     // On termine la séquence d'actions avec un WAIT
//     // actions.push_back("WAIT");
//     cerr << "actions officielles = ";
//     for (const string& action : actions) {
//         cerr << action << " ";
//     }
//     cerr << endl;
//     cerr << "nb d'actions : " << actions.size() << endl << endl;

    return instructions;
    // return actions;
}




// Fonction pour reconstruire le chemin optimal une fois la cible trouvée
vector<string> reconstructPath(const Point& start, const Point& target, const map<Point, Point>& came_from, const vector<string>& maze) {
    Point current = target;
    vector<string> instructions;
    // cerr << "reconstruct path" << endl;

    // Remonter à travers les parents à partir de la cible jusqu'au point de départ
    while (current != start) {
        // Ajouter l'instruction de mouvement pour aller du parent au nœud actuel
        Point parent = came_from.at(current);

        if (parent.x < current.x) {
            // cerr << "on monte" << endl;
            instructions.push_back("UP");
        } else if (parent.y < current.y) {
            // cerr << "on va à droite ?" << endl;
            instructions.push_back("RIGHT");
        } else if (parent.y > current.y) {
            // cerr << "on va à gauche ?" << endl;
            instructions.push_back("LEFT");
        }

        // Mettre à jour le nœud actuel avec son parent
        current = parent;
    }

    cerr << "reconstruct path = ";
    for (const string& instruction : instructions) {
        cerr << instruction << " ";
    }
    cerr << endl;
    cerr << "nb d'instructions dans le path : " << instructions.size() << endl << endl;

/*
  On a les instructions dans l'ordre EXIT -> START
  On va parcourir le vecteur dans le sens inverse et transformer chaque instruction en une série d'actions
*/
    vector<string> actions;
    actions = translateInstructions(instructions, came_from, maze, start, target);

    // return instructions;
    return actions;
}










// // Fonction pour reconstruire le chemin optimal une fois la cible trouvée
// vector<string> reconstructPath(const Point& start, const Point& target, const map<Point, Point>& came_from, const vector<string>& maze) {
//     Point current = target;
//     vector<string> instructions;
//     cerr << "reconstruct path / first dir = " << firstCloneDir << endl;
//     // string cloneDir = firstCloneDir;
//     pathCloneDir = firstCloneDir;
//     vector<string> actions;
//     string action;

//     int inst = 0;

//     // On commence la séquence d'actions avec un WAIT
//     actions.push_back("WAIT");

//     // Remonter à travers les parents à partir de la cible jusqu'au point de départ
//     while (current != start) {
//         // Ajouter l'instruction de mouvement pour aller du parent au nœud actuel
//         Point parent = came_from.at(current);
//         cerr << "Parent (" << parent.x << "," << parent.y << ")" << endl;
//         if (parent.x < current.x) {
//             // cerr << "on monte" << endl;
//             instructions.push_back("UP");

//             if (maze[current.x][current.y] != 'E' && addElev > 0) {
//                 // actions.push_back("WAIT");
//                 actions.push_back("WAIT");
//                 actions.push_back("WAIT");
//                 actions.push_back("WAIT");
//                 actions.push_back("ELEVATOR");
//             } else {
//                 actions.push_back("WAIT");
//             }
//             inst++;
//             cerr << inst << " | UP " << endl;


//         } else if (parent.y < current.y) {
//             // cerr << "on va à droite ?" << endl;
//             instructions.push_back("RIGHT");

//             action = selectAction("RIGHT", pathCloneDir);
//             // cerr << "select action = " << action << endl;
//             if (action != "WAIT") {
//                 pathCloneDir = "RIGHT";
//                 // actions.push_back("WAIT");
//                 // actions.push_back("WAIT");
//             }
//             actions.push_back(action);
//             inst++;
//             cerr << inst << " | RIGHT " << action << endl;


//         } else if (parent.y > current.y) {
//             // cerr << "on va à gauche ?" << endl;
//             instructions.push_back("LEFT");

//             // cerr << "Clone direction = " << pathCloneDir << endl;
//             action = selectAction("LEFT", pathCloneDir);
//             // cerr << action << endl;
//             actions.push_back(action);
//             if (action != "WAIT") {
//                 pathCloneDir = "LEFT";
//                 // actions.push_back("BLOCK");
//                 // actions.push_back("WAIT");
//                 // actions.push_back("WAIT");
//             }

//             // cerr << "Clone direction after action = " << pathCloneDir << endl;
//             inst++;
//             cerr << inst << " | LEFT " << action << endl;
//         }

//         // Mettre à jour le nœud actuel avec son parent
//         current = parent;
//     }

//     cerr << "reconstruct path = ";
//     for (const string& instruction : instructions) {
//         cerr << instruction << " ";
//     }
//     cerr << endl;
//     cerr << "nb d'instructions dans le path : " << instructions.size() << endl << endl;


//     // On termine la séquence d'actions avec un WAIT
//     // actions.push_back("WAIT");
//     cerr << "actions officielles = ";
//     for (const string& action : actions) {
//         cerr << action << " ";
//     }
//     cerr << endl;
//     cerr << "nb d'actions : " << actions.size() << endl << endl;

//     // return instructions;
//     return actions;
// }


// Fonction pour le chemin de start vers target le plus rapide (clock !)
vector<string> pathfinding(const Point& start, const Point& target, int rows, int cols, const vector<string>& maze, const map<int, vector<int>>& elevator) {

/*
        ALGORITHME A* 
    There are two sets, OPEN and CLOSED. The OPEN set contains those nodes that are candidates for
    examining. Initially, the OPEN set contains only one element: the starting position. 
    The CLOSED set contains those nodes that have already been examined. Initially, the CLOSED set 
    is empty. Graphically, the OPEN set is the “frontier” and the CLOSED set is the “interior” of
    the visited areas. Each node also keeps a pointer to its parent node so that we can determine
    how it was found.
*/

    // Définition des nœuds de départ et d'arrivée
    cerr << "\nPathFinding !" << endl;
    vector<string> instructions;

    // cerr << "Point ["<< target.x << "]["<< target.y << "] = "<<maze[target.x][target.y] << endl;

    // Implémentation de l'algorithme A*
    l_noeud open_set; // file de priorité pour les noeuds à explorer
    l_noeud closed_set; // noeuds déjà explorés
    map<Point, Point> came_from; // stocke les info sur les parents

    // ajouter le noeud de départ à open_set
    Noeud start_node;
    start_node.cout_g = 0;
    start_node.cout_h = distance(start.x, start.y, target.x, target.y);
    start_node.cout_f = start_node.cout_g + start_node.cout_h;
    start_node.parent = Point{-1, -1}; // pas de parent pour le noeud de départ
    open_set[start] = start_node;

    cerr << "target : (" << target.x << "," << target.y << ")" << endl << endl;

    while (!open_set.empty()) {
        // Trouver le nœud avec le coût total le plus bas dans open_set
        auto current_it = min_element(open_set.begin(), open_set.end(),
            [](const pair<Point, Noeud>& a, const pair<Point, Noeud>& b) {
                return a.second.cout_f < b.second.cout_f;
            });

        // Extraire le nœud actuel
        Point current_point = current_it->first;
        Noeud current_node = current_it->second;
        open_set.erase(current_it);

        // Afficher le contenu de la map open_set
        // cerr << "Contenu de open_set : " << endl;
        // for (const auto& pair : open_set) {
        //     cerr << "Clé : (" << pair.first.x << ", " << pair.first.y << "), ";
        //     cerr << "Valeur : cout_g = " << pair.second.cout_g << ", ";
        //     cerr << "cout_h = " << pair.second.cout_h << ", ";
        //     cerr << "cout_f = " << pair.second.cout_f << ", ";
        //     cerr << "Parent : (" << pair.second.parent.x << ", " << pair.second.parent.y << ")" << endl;
        // }

        // Vérifier si le nœud actuel est le nœud cible
        if (current_point.x == target.x && current_point.y == target.y) {
            // cerr << "path reconst" << endl;
            // Reconstruire le chemin optimal
            // instructions = reconstructPath(start, target, came_from);
            instructions = reconstructPath(start, target, came_from, maze);
            return instructions;
        }

        // Ajouter le nœud actuel à closed_set
        closed_set.insert({current_point, current_node});

        // Explorer les voisins du nœud actuel
        for (const auto& dir : directions) {
            // cerr << "boucle for" << endl;
            int new_x = current_point.x + dir.x;
            int new_y = current_point.y + dir.y;
            Point goal = {new_x, new_y};

            // Vérifier si le voisin est valide (donc si on peut aller UP !)
            if (isValid(goal, current_point, target, dir, rows, cols, maze, elevator)) {
                if (maze[current_point.x][current_point.y] == 'S') {
                    cerr << "current : " << maze[current_point.x][current_point.y] << "(" << current_point.x << "," << current_point.y << ")";
                    cerr << " | next : " << maze[new_x][new_y] << "(" << new_x << "," << new_y << ")";
                    cerr << " | dir x = " << dir.x << " | dir y = " << dir.y << endl << endl;
                }
                Point neighbor = {new_x, new_y};

                // Vérifier si le voisin est déjà dans closed_set
                if (closed_set.find(neighbor) != closed_set.end()) {
                    continue; // Ignorer ce voisin, déjà exploré
                }

// cost function : g'(n) = 1 + alpha * (g(n) - 1) alpha must match scale in distance()
// heuristic function : h(n) = h'(n, w1) + distance(w1, w2) + h'(w2, goal)

                // Calculer les coûts pour le voisin
                float neighbor_g = current_node.cout_g + 1; // Coût du mouvement vers ce voisin
                float neighbor_h = distance(new_x, new_y, target.x, target.y);
                float neighbor_f = neighbor_g + neighbor_h;

                // Vérifier si le voisin est déjà dans open_set
                if (open_set.find(neighbor) == open_set.end() || neighbor_g < open_set[neighbor].cout_g) {
                    // Mettre à jour les informations pour ce voisin
                    Noeud neighbor_node;
                    neighbor_node.cout_g = neighbor_g;
                    neighbor_node.cout_h = neighbor_h;
                    neighbor_node.cout_f = neighbor_f;
                    neighbor_node.parent = Point{current_point.x, current_point.y};
                    open_set[neighbor] = neighbor_node;
                    came_from[neighbor] = current_point; // On enregistre le parent
                }
            }
        }
    }
    // Si aucun chemin trouvé, retourner une chaîne vide
    return instructions;
}

// ---------------------- FIN ALGO A* ------------------------ //

int main()
{
    int nb_floors; // number of floors
    int width; // width of the area
    int nb_rounds; // maximum number of rounds
    int exit_floor; // floor on which the exit is found
    int exit_pos; // position of the exit on its floor
    int nb_total_clones; // number of generated clones
    int nb_additional_elevators; // number of additional elevators that you can build
    int nb_elevators; // number of elevators
    cin >> nb_floors >> width >> nb_rounds >> exit_floor >> exit_pos >> nb_total_clones >> nb_additional_elevators >> nb_elevators; cin.ignore();

    cerr << "LES DONNEES DE DEPART : " << endl;

    cerr << "nb_floors = " << nb_floors << endl;
    cerr << "width = " << width << endl;
    cerr << "nb_rounds = " << nb_rounds << endl;
    cerr << "exit_floor = " << exit_floor << endl;
    cerr << "exit_pos = " << exit_pos << endl;
    cerr << "nb_total_clones = " << nb_total_clones << endl;
    cerr << "nb_additional_elevators = " << nb_additional_elevators << endl;
    cerr << "nb_elevators = " << nb_elevators << endl;

    cerr << endl;

    // màj variables globales
    addElev = nb_additional_elevators;

    // On enregistre les ascenseurs par floor
    map<int, vector<int>> elevator;
    // On va mettre le plan général dans un vecteur de string
    vector<string> maze(nb_floors, string(width, '.'));

    for (int i = 0; i < nb_elevators; i++) {
        int elevator_floor; // floor on which this elevator is found
        int elevator_pos; // position of the elevator on its floor
        cin >> elevator_floor >> elevator_pos; cin.ignore();
        // cerr << "elev_floor = " << elevator_floor << " | elev_pos = " << elevator_pos << endl;

        // ON REMPLIT NOTRE MAP ELEVATOR
        if (elevator.find(elevator_floor) == elevator.end()) {
            // si l'étage n'existe pas, on crée un nouveau vecteur pour cet étage
            vector<int> positions;
            positions.push_back(elevator_pos); //-> on ajoute la positions de l'elevator au vecteur
            elevator[elevator_floor] = positions; //-> on met notre vecteur en value sur la clé de map
        } else { 
            // si l'étage existe déjà : on ajoute la position de l'elevator au vecteur
            elevator[elevator_floor].push_back(elevator_pos);
        }

        // ON REMPLIT NOTRE VECTOR MAZE
        // Mise à jour du labyrinthe pour marquer la présence de l'ascenseur
        maze[elevator_floor][elevator_pos] = 'E';
    }

    // ON COMPLETE LA MAP ELEVATOR
    // Déterminons le ou les étages sans ascenseurs et mise à jour de la map avec des vecteurs empty
    if (elevator.size() != nb_floors) {
        for (int missing_floor = 0; missing_floor < nb_floors; missing_floor++) {
            auto it = elevator.find(missing_floor);
            if (it == elevator.end()) { //-> clé non trouvée
                // on crée la clé avec un vecteur d'int vide en value
                elevator.emplace(missing_floor, vector<int>());
                if (missing_floor != exit_floor)
                    noElev++;
            }
        }
    }

    // DANS MAP ELEVATOR : ON TRIE LES VECTEURS POUR AVOIR LES POSITIONS EN ORDRE CROISSANT
    for (int i = 0; i < nb_floors; i++) {
        auto it = elevator.find(i);
        if (!it->second.empty())
            sort(it->second.begin(), it->second.end());
    }


    // ON COMPLETE LE MAZE
    // Mise à jour du labyrinthe pour marquer la position de la sortie
    maze[exit_floor][exit_pos] = 'S';
    // Si niveau au-dessus du exit_floor -> inaccessible x
    if (exit_floor != nb_floors - 1) {
        for (int i = exit_floor + 1; i < nb_floors; i++) {
            for (int j = 0; j < width; j++) {
                maze[i][j] = 'x';
            }
        }
    }
    // Si on a des ascenseurs au niveau de l'exit -> inacessible x puis on descend...
    for (int floor = exit_floor; floor >= 0; floor--) {
        auto it = elevator.find(floor);
        if (!it->second.empty()) {
            for (int j = exit_pos; j < width; j++) {
                if (maze[floor][j] == 'E' && maze[floor + 1][j] == 'x') {
                    maze[floor][j] = 'x';
                    for (int f = j; f < width; f++) {
                        maze[floor][f] = 'x';
                    }
                }
            }
            for (int j = exit_pos; j >= 0; j--) {
                if (maze[floor][j] == 'E' && maze[floor + 1][j] == 'x') {
                    maze[floor][j] = 'x';
                    for (int f = j; f >= 0; f--) {
                        maze[floor][f] = 'x';
                    }
                }
            }    
        }
    }

    print(elevator);

    // // Affichage du labyrinthe
    // for (int i = nb_floors - 1; i >= 0; i--) {
    //     cerr << maze[i] << endl;
    // }

// ------------------- CODE CI-DESSOUS VALABLE POUR LES 5 PREMIERS TESTS PAS A* ------------------- //

    // string action; // action à effectuer (BLOCK WAIT ELEVATOR)
    // string good_direction;
    // // game loop
    // while (1) {
    //     int clone_floor; // floor of the leading clone
    //     int clone_pos; // position of the leading clone on its floor
    //     string direction; // direction of the leading clone: LEFT or RIGHT
    //     cin >> clone_floor >> clone_pos >> direction; cin.ignore();

    //     cerr << "clone_floor : " << clone_floor << endl;
    //     cerr << "clone_pos : " << clone_pos << endl;
    //     cerr << "direction prise par le clone : " << direction << endl;

    //     if (clone_pos != -1 || clone_floor != -1) {
    //         if (clone_floor != exit_floor) {
    //             auto it = elevator.find(clone_floor);
    //             // 1) On vérifie si on a un tableau vide (= 0 elevator) dans la map
    //             if (it->second.empty()) { //-> pas d'ascenseur
    //                 // 1.a) action : on crée l'elevator
    //                 action = "ELEVATOR";
    //                 // 1.b) on entre sa position dans la map en utilisant la position du clone
    //                 it->second.push_back((clone_pos));
    //                 print(elevator);
    //             } else { //-> on a un ou plusieurs ascenceurs 
    //                 // 2) On prend la bonne direction !
    //                 int closestValue = findClosestValue(elevator[clone_floor], clone_pos);
    //                 action = selectAction(clone_pos, closestValue, direction);
    //             }
    //         } else if (clone_floor == exit_floor) {
    //             action = selectAction(clone_pos, exit_pos, direction);
    //         } else {
    //             action = "WAIT";
    //         }
    //     } else {
    //         action = "WAIT";
    //     }

    //     cout << action << endl;

    //     // Write an action using cout. DON'T FORGET THE "<< endl" / action: WAIT BLOCK ELEVATOR
    //     // To debug: cerr << "Debug messages..." << endl;
    // }

// ------------------- CODE CI-DESSUS VALABLE POUR LES 5 PREMIERS TESTS ------------------- //

    /*
      Il faudrait intégrer le nb d'ascenseurs constructibles au pathfinding... 
      Ainsi que le nombre de tours !
    */

    /*
      A FAIRE :
      Essayons un pathfinding avec A*, des trous, des elevator constructibles, et un nb de tours...
      Et du pruning, ex : les ascenseurs à l'étage de sortie ne servent à rien et doivent être évités !
    */

    string action; // action à effectuer (BLOCK WAIT ELEVATOR)
    string good_direction;
    Point start;
    Point target;
    bool maj = false;
    vector<string> path;
    int lastFloor = -1; // pos et floor du dernier clone
    int lastPos = -1;
    bool cloneDestroyed = false;
    vector<string> actions;

    // game loop
    while (1) {
        int clone_floor; // floor of the leading clone
        int clone_pos; // position of the leading clone on its floor
        string direction; // direction of the leading clone: LEFT or RIGHT
        cin >> clone_floor >> clone_pos >> direction; cin.ignore();

        cerr << "clone_floor : " << clone_floor << endl;
        cerr << "clone_pos : " << clone_pos << endl;
        cerr << "direction prise par le clone : " << direction << endl;

        firstCloneDir = direction;

        // On met à jour la position de départ ATTENTION : une seule fois !
        if (!maj) {
            maze[clone_floor][clone_pos] = 'D';
            // on met maj à true pour ne pas refaire cette opération
            maj = true;
            // Affichage du labyrinthe complet
            for (int i = nb_floors - 1; i >= 0; i--) {
                cerr << maze[i] << endl;
            }
            cerr << "nb de niveaux sans elevator : " << noElev << endl;
            // Et maintenant on peut lancer l'algo A*
            start = {clone_floor, clone_pos};
            target = {exit_floor, exit_pos};
            path = pathfinding(start, target, nb_floors, width, maze, elevator);
            cerr << path.size() << " vs nb_rounds " << nb_rounds << endl;
        }





        // Exécution d'une instruction à chaque itération NON ! Pas à chaque itération !
        // Seulement quand le clone en cours arrive sur le dernier floor/pos visité ! 

/*
  string instructions : UP RIGHT LEFT ... 
  chaque instruction correspond à un mouvement (de case en case dans le maze)
  quand une instruction est effectuée : 
  - soit le clone est détruit ou abandonné : BLOCK - UP/avec création d'ELEVATOR
     |-> conserver la position de ce clone sacrifié, et mettre les instructions en pause jusqu'à 
         ce que le clone suivant arrive à cet endroit / pas de retrait de l'instruction
  - soit il continue : RIGHT LEFT UP/sans création
     |-> on retire l'instruction de la string
*/

// -------- SAVE EN COURS DE MODIF --------------- //

        // if (!path.empty()) {
        //     string next_instruction = path.back();
        //     // path.pop_back();
        //     cerr << "instruction en cours = " << next_instruction << endl;

        //     if (next_instruction == "UP") {
        //         // if (lastFloor < 0 && lastPos < 0) {
        //         if (!cloneDestroyed) { // first clone
        //             auto it = elevator.find(clone_floor);
        //             // On n'est pas sur 1 ascenseur, il faut le créer -> pas de delete instruction !
        //             // 1) On vérifie si on a un tableau vide (= 0 elevator) dans la map
        //             // if (it->second.empty()) { //-> pas d'ascenseur
        //             if (maze[clone_floor][clone_pos] != 'E') { // first clone + On n'est pas sur un E
        //                 cerr << "instruction UP sans E -> donc Elevator" <<endl;
        //                 // 1.a) action : on crée l'elevator
        //                 action = "ELEVATOR";
        //                 // 1.b) on entre sa position dans la map en utilisant la position du clone
        //                 it->second.push_back((clone_pos));
        //                 // 1.c) màj du maze
        //                 maze[clone_floor][clone_pos] = 'E';
        //                 // 1.d) on retient la position de ce clone et on indique qu'il a été détruit
        //                 lastFloor = clone_floor;
        //                 lastPos = clone_pos;
        //                 cloneDestroyed = true;
        //             } else { // first clone + On est sur un E
        //                 cerr << "instruction UP sur E -> donc Wait" << endl;
        //                 action = "WAIT";
        //                 cerr << "on delete l'instruction" << endl;
        //                 path.pop_back();
        //             }
        //         } else { // le premier clone a été détruit, on attend que le suivant arrive à la bonne position
        //             if (clone_floor == lastFloor && clone_pos == lastPos) {
        //                 cloneDestroyed = false;
        //                 cerr << "on delete l'instruction" << endl;
        //                 path.pop_back();
        //             }
        //             action = "WAIT";
        //         }
        //     }

        //     if (next_instruction == "RIGHT" || next_instruction == "LEFT") {
        //         if (!cloneDestroyed) { // first clone
        //             cerr << "instruction = " << next_instruction << endl;
        //             action = selectAction(next_instruction, direction);
        //             if (action == "BLOCK") {
        //                 lastFloor = clone_floor;
        //                 lastPos = clone_pos;
        //                 cloneDestroyed = true;
        //             } else {
        //                 cerr << "on delete l'instruction" << endl;
        //                 path.pop_back();                        
        //             }
        //         } else { // le premier clone a été détruit, on attend que le suivant arrive à la bonne pos
        //             if (clone_floor == lastFloor && clone_pos == lastPos) {
        //                 cloneDestroyed = false;
        //                 cerr << "on delete l'instruction" << endl;
        //                 path.pop_back();
        //             }
        //             action = "WAIT";
        //         }
        //         // if (action != "BLOCK"){ // DONC ici pareil, il faut le système cloneDestroyed et Lastfloor/pos
        //         //     cerr << "on delete l'instruction" << endl;
        //         //     path.pop_back();
        //         // }
                
        //     }
        // } else {
        //     action = "WAIT";
        // }
        // cout << action << endl;

// -------- SAVE EN COURS DE MODIF --------------- //

        if (!path.empty()) {
            string action = path.back();
            path.pop_back();
            cerr << "instruction en cours = " << action << endl;
            cout << action << endl;
        }

        // Write an action using cout. DON'T FORGET THE "<< endl" / action: WAIT BLOCK ELEVATOR
        // To debug: cerr << "Debug messages..." << endl;
    }
}

// ----------------------------------------------------------------------------------------------------
