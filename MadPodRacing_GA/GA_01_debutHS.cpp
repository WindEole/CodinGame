#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

// struct Checkpoint {
//     int x;
//     int y;
// };

struct Move {
    int thrust;
    int angle;
};

struct PodState {
    int x;
    int y;
    int vx;
    int vy;
    int angle;
    int next_checkpoint_x;
    int next_checkpoint_y;
    int next_checkpoint_dist;
    int next_checkpoint_angle;
};

struct Individual {
    vector<Move> moves;
    double fitness;
};

bool initialized = false;
int current_checkpoint_index = 0;
bool boosted = false;

const int POPULATION_SIZE = 100;
const int NUM_GENERATIONS = 1000;
const int RADIUS = 400;

vector<pair<int, int>> checkpoints;
vector<Individual> population;
Individual best_individual;

// double distance(int x1, int y1, int x2, int y2) {
//     return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
// }

// bool detect_collision(int x1, int y1, int x2, int y2) {
//     return distance(x1, y1, x2, y2) <= 2 * RADIUS;
// }

bool check_collision(const PodState& pod1, const PodState& pod2) {
    int dx = pod2.x - pod1.x;
    int dy = pod2.y - pod1.y;
    return (dx * dx + dy * dy) <= 160000; // 400^2
}

void generateRandomMove(Move& move) {
    // move.thrust = rand() % 101; // thrust entre 0 et 100
    // move.thrust = rand() % 11 + 90; // thrust entre 75 et 100
    move.thrust = rand() % 4 + 97; // thrust entre 97 et 100
    move.angle = (rand() % 181) - 90; // angle entre -90 et 90 degrés
}

// double evaluate(const Individual& individual, const PodState& pod) {
//     double fitness = 0.0;
//     int x = pod.x;
//     int y = pod.y;
//     int vx = pod.vx;
//     int vy = pod.vy;
//     int angle = pod.angle;

//     for (const auto& move : individual.moves) {
//         angle += move.angle;
//         angle = (angle + 360) % 360;
//         double rad_angle = angle * M_PI / 180.0;

//         vx += move.thrust * cos(rad_angle);
//         vy += move.thrust * sin(rad_angle);

//         x += vx;
//         y += vy;

//         double dist = hypot(checkpoints[current_checkpoint_index].x - x, checkpoints[current_checkpoint_index].y - y);
//         fitness -= dist;
//     }
//     return fitness;
// }

// double evaluate(const Individual& individual, const PodState& pod, const vector<pair<int, int>>& opponent_positions) {
//     double fitness = 0.0;
//     int x = pod.x, y = pod.y;
//     for (const auto& move : individual.moves) {
//         int target_x = x + cos(move.angle * M_PI / 180.0) * move.thrust;
//         int target_y = y + sin(move.angle * M_PI / 180.0) * move.thrust;
//         fitness -= distance(target_x, target_y, pod.next_checkpoint_x, pod.next_checkpoint_y);
//         for (const auto& opponent_pos : opponent_positions) {
//             if (detect_collision(target_x, target_y, opponent_pos.first, opponent_pos.second)) {
//                 fitness += 500; // Reward for causing a collision
//             }
//         }
//         x = target_x;
//         y = target_y;
//     }
//     return fitness;
// }

double evaluate(const Individual& individual, const PodState& pod, const vector<PodState>& opponents) {
    double fitness = 0.0;
    int x = pod.x, y = pod.y;
    int vx = pod.vx, vy = pod.vy;

    for (const auto& move : individual.moves) {
        // Appliquer le mouvement
        double rad_angle = move.angle * M_PI / 180.0;
        vx += cos(rad_angle) * move.thrust;
        vy += sin(rad_angle) * move.thrust;
        x += vx;
        y += vy;

        // Calculer la distance au prochain checkpoint
        double dist = hypot(pod.next_checkpoint_x - x, pod.next_checkpoint_y - y);
        fitness += dist;

        // Pénaliser les collisions avec les adversaires
        for (const auto& opponent : opponents) {
            if (check_collision({x, y, vx, vy, pod.angle}, opponent)) {
                fitness -= 1000; // Valeur à ajuster selon l'importance des collisions
            }
        }
    }

    return -fitness;  // Lower distance is better
}


void mutate(Individual& individual) {
    int mutation_point = rand() % individual.moves.size();
    generateRandomMove(individual.moves[mutation_point]);
}

// Individual crossover(const Individual& parent1, const Individual& parent2) {
//     Individual offspring;
//     for (size_t i = 0; i < parent1.moves.size(); ++i) {
//         if (rand() % 2 == 0) {
//             offspring.moves.push_back(parent1.moves[i]);
//         } else {
//             offspring.moves.push_back(parent2.moves[i]);
//         }
//     }
//     return offspring;
// }

Individual crossover(const Individual& parent1, const Individual& parent2) {
    Individual offspring;
    int crossover_point = rand() % parent1.moves.size();
    for (int i = 0; i < crossover_point; ++i) {
        offspring.moves.push_back(parent1.moves[i]);
    }
    for (int i = crossover_point; i < parent2.moves.size(); ++i) {
        offspring.moves.push_back(parent2.moves[i]);
    }
    return offspring;
}

// Individual select(const vector<Individual>& population) {
//     // Tournament selection
//     Individual best = population[rand() % POPULATION_SIZE];
//     for (int i = 1; i < 3; ++i) {
//         Individual contender = population[rand() % POPULATION_SIZE];
//         if (contender.fitness > best.fitness) {
//             best = contender;
//         }
//     }
//     return best;
// }

Individual select(const vector<Individual>& population) {
    return population[rand() % population.size()];
}

bool compareFitness(const Individual& a, const Individual& b) {
    return a.fitness < b.fitness;
}

// void initialize_population() {
//     for (int i = 0; i < POPULATION_SIZE; ++i) {
//         Individual ind;
//         for (int j = 0; j < 10; ++j)
//             ind.moves.push_back({rand() % 51 + 50, (rand() % 181) - 90});
//         population.push_back(ind);
//     }
// }

// void geneticAlgorithm(int x, int y, int next_checkpoint_x, int next_checkpoint_y, const vector<pair<int, int>>& opponent_positions) {
//     PodState pod = {x, y, 0, 0, 0, next_checkpoint_x, next_checkpoint_y};

//     // Initialize population if not already done
//     if (population.empty()) {
//         initialize_population();
//         for (auto& ind : population) {
//             ind.fitness = evaluate(ind, pod, opponent_positions);
//         }
//         best_individual = *max_element(population.begin(), population.end(), compareFitness);
//     }

//     // Run the genetic algorithm
//     for (int generation = 0; generation < NUM_GENERATIONS; ++generation) {
//         vector<Individual> new_population;
//         for (int i = 0; i < POPULATION_SIZE; ++i) {
//             Individual parent1 = select(population);
//             Individual parent2 = select(population);
//             Individual offspring = crossover(parent1, parent2);
//             mutate(offspring);
//             offspring.fitness = evaluate(offspring, pod, opponent_positions);
//             new_population.push_back(offspring);
//         }
//         population = new_population;
//         best_individual = *max_element(population.begin(), population.end(), compareFitness);
//     }
// }

int main()
{
    srand(time(0));

    // game loop
    while (1) {
        int x;
        int y;
        int next_checkpoint_x; // x position of the next check point
        int next_checkpoint_y; // y position of the next check point
        int next_checkpoint_dist; // distance to the next checkpoint
        int next_checkpoint_angle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> next_checkpoint_x >> next_checkpoint_y >> next_checkpoint_dist >> next_checkpoint_angle; cin.ignore();
        int opponent_x;
        int opponent_y;
        cin >> opponent_x >> opponent_y; cin.ignore();

        cerr << next_checkpoint_x << " " << next_checkpoint_y << endl;

        // Initialisation des checkpoints
        if (!initialized)
        {
            checkpoints.push_back({next_checkpoint_x, next_checkpoint_y});
            initialized = true;
        }
        else
        {
            if (checkpoints[current_checkpoint_index].first != next_checkpoint_x || checkpoints[current_checkpoint_index].second != next_checkpoint_y)
            {
                current_checkpoint_index++;
                if (current_checkpoint_index >= checkpoints.size())
                    checkpoints.push_back({next_checkpoint_x, next_checkpoint_y});
            }
        }
    
       // Afficher les checkpoints
        cerr << "Checkpoints: ";
        for (const auto& checkpoint : checkpoints) {
            cerr << "(" << checkpoint.first << ", " << checkpoint.second << ") ";
        }
        cerr << endl;

       // Informations des pods adverses
        vector<PodState> opponents;
        opponents.push_back({opponent_x, opponent_y, 0, 0, 0, 0, 0, 0, 0}); // Ajouter les vrais états si disponibles


        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"

    // ---------- ALGORITHME GENETIQUE !!!!!! ---------- //

    // Initier l'AG au premier tour
        // if (population.empty())
        // {
        //     for (int i = 0; i < POPULATION_SIZE; i++)
        //     {
        //         Individual ind;
        //         for (int j = 0; j < 10; ++j)
        //         {
        //             Move move;
        //             generateRandomMove(move);
        //             ind.moves.push_back(move);
        //         }
        //         ind.fitness = evaluate(ind, {x, y, 0, 0, 0});
        //         population.push_back(ind);
        //     }
        //     best_individual = *max_element(population.begin(), population.end(), compareFitness);
        // }
        if (population.empty()) {
            for (int i = 0; i < POPULATION_SIZE; i++) {
                Individual ind;
                for (int j = 0; j < 10; ++j)
                    ind.moves.push_back({rand() % 16000, rand() % 9000});
                ind.fitness = evaluate(ind, {x, y, 0, 0, 0, next_checkpoint_x, next_checkpoint_y, next_checkpoint_dist, next_checkpoint_angle}, opponents);
                population.push_back(ind);
            }
            best_individual = *max_element(population.begin(), population.end(), compareFitness);
        }

        // vector<pair<int, int>> opponent_positions = {{opponent_x, opponent_y}};
        // geneticAlgorithm(x, y, next_checkpoint_x, next_checkpoint_y, opponent_positions);

    // Lancer l'AG
        // for (int generation = 0; generation < NUM_GENERATIONS; ++generation)
        // {
        //     vector<Individual> new_population;
        //     for (int i = 0; i < POPULATION_SIZE; ++i)
        //     {
        //         Individual parent1 = select(population);
        //         Individual parent2 = select(population);
        //         Individual offspring = crossover(parent1, parent2);
        //         mutate(offspring);
        //         offspring.fitness = evaluate(offspring, {x, y, 0, 0, 0});
        //         new_population.push_back(offspring);
        //     }
        //     population = new_population;
        //     best_individual = *max_element(population.begin(), population.end(), compareFitness);
        // }
        for (int generation = 0; generation < NUM_GENERATIONS; ++generation) {
            vector<Individual> new_population;
            for (int i = 0; i < POPULATION_SIZE; ++i) {
                Individual parent1 = select(population);
                Individual parent2 = select(population);
                Individual offspring = crossover(parent1, parent2);
                mutate(offspring);
                offspring.fitness = evaluate(offspring, {x, y, 0, 0, 0, next_checkpoint_x, next_checkpoint_y, next_checkpoint_dist, next_checkpoint_angle}, opponents);
                new_population.push_back(offspring);
            }
            population = new_population;
            best_individual = *max_element(population.begin(), population.end(), compareFitness);
        }

        // prendre le best move du best_individual
        // int angle = best_individual.moves[0].angle;
        // int thrust = best_individual.moves[0].thrust;
        // // int thrust = 100;
        // Move best_move = best_individual.moves[0];
        // cerr << best_move.thrust << " " << best_move.angle << endl;
        // int target_x = x + cos(best_move.angle * M_PI / 180.0) * best_move.thrust;
        // int target_y = y + sin(best_move.angle * M_PI / 180.0) * best_move.thrust;
        // int thrust = best_move.thrust;
        // cerr << target_x << " " << target_y << endl;

        // prendre le best move du best_individual
        Move best_move = best_individual.moves[0];
        double rad_angle = best_move.angle * M_PI / 180.0;
        int target_x = x + cos(rad_angle) * 1000; // Distance arbitraire pour visualiser la direction
        int target_y = y + sin(rad_angle) * 1000;

        cerr << best_move.thrust << endl;

        if (!boosted && (next_checkpoint_angle < 5 && next_checkpoint_angle > -5) && next_checkpoint_dist >= 5000)
        {
            boosted = true;
            cout << next_checkpoint_x << " " << next_checkpoint_y << " BOOST" << " BOOST" << endl;
            // cout << target_x << " " << target_y << " BOOST" << endl;
        }
        else
            cout << next_checkpoint_x << " " << next_checkpoint_y << " " << best_move.thrust << " " << best_move.thrust << endl;
            // cout << target_x << " " << target_y << " " << best_move.thrust << endl;

        // update les moves du best_individual avec un shift
        for (size_t i = 1; i < best_individual.moves.size(); ++i)
            best_individual.moves[i - 1] = best_individual.moves[i];
        Move new_move;
        generateRandomMove(new_move);
        best_individual.moves.back() = new_move; // new random move
    }
    return 0;
}



        /* int thrust;

        if (next_checkpoint_angle > 90 || next_checkpoint_angle < -90) {
            thrust = 0;
        } else {
            thrust = 100;
        }

        if (!boosted && (next_checkpoint_angle < 5 && next_checkpoint_angle > -5) && next_checkpoint_dist >= 5000) {
            boosted = true;
            cerr << "BOOST ACTIVATED !" << endl;
            cout << next_checkpoint_x << " " << next_checkpoint_y << " BOOST" << endl;
        } else {
            cout << next_checkpoint_x << " " << next_checkpoint_y << " " << thrust << endl;
        }
        */




// bool boosted = false;

