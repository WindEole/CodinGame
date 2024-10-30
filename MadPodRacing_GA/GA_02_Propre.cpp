#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

struct Checkpoint {
    int x;
    int y;
};

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
bool finished = false;
int current_checkpoint_index = 0;
bool boosted = false;

const int POPULATION_SIZE = 100;
const int NUM_GENERATIONS = 1000;
const int RADIUS = 400;

vector<pair<int, int>> checkpoints;
vector<Individual> population;
Individual best_individual;

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

double evaluate(const Individual& individual, const PodState& pod, const vector<PodState>& opponents) {
    double fitness = 0.0;
    int x = pod.x;
	int y = pod.y;
    int vx = pod.vx;
	int vy = pod.vy;
	int current_angle = pod.angle;

    for (auto move : individual.moves) {
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
            if (check_collision({x, y, vx, vy, current_angle}, opponent)) {
                fitness -= 1000; // Valeur à ajuster selon l'importance des collisions
            }

			// Si l'adversaire est derrière moi : je veux qu'il me pousse via collision donc je dois ralentir
			double opponent_angle = atan2(opponent.y - y, opponent.x - x) * 180 / M_PI;
			double relative_angle = current_angle - opponent_angle;
			if (relative_angle < -180)
				relative_angle += 360;
			else if (relative_angle > 180)
				relative_angle -= 360;

			if (abs(relative_angle) > 120) {
                // Reduce thrust to provoke a collision
                if (move.thrust > 20) {
                    move.thrust -= 20;
                } else {
                    move.thrust = 0;
                }
            }
        }

		// On ralentit à l'approche des checkpoints pour moins d'inertie
		if (dist < 3000) {
            move.thrust = static_cast<int>(move.thrust * dist / 3000.0); // Proportionally reduce thrust
        }
    }
    return -fitness;  // Lower distance is better
}


void mutate(Individual& individual) {
    int mutation_point = rand() % individual.moves.size();
    generateRandomMove(individual.moves[mutation_point]);
}

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

Individual select(const vector<Individual>& population) {
    return population[rand() % population.size()];
}

bool compareFitness(const Individual& a, const Individual& b) {
    return a.fitness < b.fitness;
}

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
            checkpoints.push_back({next_checkpoint_x, next_checkpoint_y}); // on met le premier checkpoint
            initialized = true;
        }
        else
        {
            if (current_checkpoint_index >= checkpoints.size() || checkpoints[current_checkpoint_index].first != next_checkpoint_x || checkpoints[current_checkpoint_index].second != next_checkpoint_y)
            {
                if (current_checkpoint_index >= checkpoints.size())
                	current_checkpoint_index = 0;
            	checkpoints[current_checkpoint_index] = {next_checkpoint_x, next_checkpoint_y};
				current_checkpoint_index++;
            }
        }
    
       // Afficher les checkpoints
        cerr << "Checkpoints: ";
        for (const auto& checkpoint : checkpoints) {
            cerr << "(" << checkpoint.first << ", " << checkpoint.second << ") ";
        }
        cerr << " taille du vecteur Checkpoints = " << checkpoints.size() << endl;

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
        if (population.empty()) {
            for (int i = 0; i < POPULATION_SIZE; i++)
			{
                Individual ind;
                for (int j = 0; j < 10; ++j)
                    ind.moves.push_back({rand() % 16000, rand() % 9000});
                ind.fitness = evaluate(ind, {x, y, 0, 0, 0, next_checkpoint_x, next_checkpoint_y, next_checkpoint_dist, next_checkpoint_angle}, opponents);
                population.push_back(ind);
            }
            best_individual = *max_element(population.begin(), population.end(), compareFitness);
        }

    	// Lancer l'AG
        for (int generation = 0; generation < NUM_GENERATIONS; ++generation)
		{
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
        Move best_move = best_individual.moves[0];
        double rad_angle = best_move.angle * M_PI / 180.0;
        int target_x = x + cos(rad_angle) * 1000; // Distance arbitraire pour visualiser la direction
        int target_y = y + sin(rad_angle) * 1000;

        // cerr << best_move.thrust << endl;

        if (!boosted && (next_checkpoint_angle < 5 && next_checkpoint_angle > -5) && next_checkpoint_dist >= 8000)
        {
            boosted = true;
            cout << next_checkpoint_x << " " << next_checkpoint_y << " BOOST" << " BOOST" << endl;
            // cout << target_x << " " << target_y << " BOOST" << endl;
        }
        else
		{
			// Adjust thrust real-time near checkpoints
			int real_thrust = best_move.thrust;
			if (next_checkpoint_dist < 2000)
				real_thrust = std::max(0, real_thrust * next_checkpoint_dist / 2000);
			cout << next_checkpoint_x << " " << next_checkpoint_y << " " << real_thrust << " " << real_thrust << endl;


            // cout << next_checkpoint_x << " " << next_checkpoint_y << " " << best_move.thrust << " " << best_move.thrust << endl;
            // cout << target_x << " " << target_y << " " << best_move.thrust << endl;
		}

        // update les moves du best_individual avec un shift
        for (size_t i = 1; i < best_individual.moves.size(); ++i)
            best_individual.moves[i - 1] = best_individual.moves[i];
        Move new_move;
        generateRandomMove(new_move);
        best_individual.moves.back() = new_move; // new random move
    }
    return 0;
}