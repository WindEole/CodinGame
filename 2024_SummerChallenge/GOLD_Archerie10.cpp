#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <climits>
#include <unordered_map>
#include <set>

#pragma GCC optimize("O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

using namespace std;

int tour;
int player_idx;
std::vector<string> score_strings;

enum GameType { HURDLES, ARCHERY, SKATING, DIVING };

enum Move { LEFT, DOWN, RIGHT, UP, INVALID };

Move stringToMove(const string& move) {
    static const unordered_map<string, Move> move_map = {
        {"left", LEFT},
        {"down", DOWN},
        {"right", RIGHT},
        {"up", UP}
    };
    auto it = move_map.find(move);
    return (it != move_map.end()) ? it->second : INVALID;
}

struct GameRegisters {
    std::string gpu;
    int reg_0;
    int reg_1;
    int reg_2;
    int reg_3;
    int reg_4;
    int reg_5;
    int reg_6;
};

struct GameHurdles {
    std::string gpu;
    int my_pos;
    int adv1_pos;
    int adv2_pos;
    int my_stun;
    int adv1_stun;
    int adv2_stun;
    int unused;
    int my_newpos;
    int adv1_newpos;
    int adv2_newpos;
    std::string my_bestseq;
    std::string adv1_bestseq;
    std::string adv2_bestseq;
};

struct GameArchery {
    std::string gpu;
    int my_posx;
    int my_posy;
    int adv1_posx;
    int adv1_posy;
    int adv2_posx;
    int adv2_posy;
    int unused;

    set<char> best_moves;

    // char max_wind;
    // bool max_is_last;
    // int dist_start;
    // int left_endx;
    // int right_endx;
    // int down_endy;
    // int up_endy;
};

struct DataWayArchery {
    int left_posx;
    int left_posy;
    int down_posx;
    int down_posy;
    int right_posx;
    int right_posy;
    int up_posx;
    int up_posy;

    vector<pair<int, string>> result;
};

struct GameSkating {
    std::string gpu;
    int my_way;
    int adv1_way;
    int adv2_way;
    int my_risk;
    int adv1_risk;
    int adv2_risk;
    int turns_left;
};

struct GameDiving {
    std::string gpu;
    int my_point;
    int adv1_point;
    int adv2_point;
    int my_combo;
    int adv1_combo;
    int adv2_combo;
    int unused;

    int diving_is_won;
    int go_silver; // VERIFIER SI BESOIN
    int diving_is_lost;
};

struct Medals {
    int gold;
    int silver;
    int bronze;
};

struct PlayerScore {
    int total_points;
    Medals hurdles;
    Medals archery;
    Medals skating;
    Medals diving;
};

std::vector<PlayerScore> scores;

struct GameInfo {
    GameType game_type;
    union GameData {
        GameHurdles hurdles;
        GameArchery archery;
        GameSkating skating;
        GameDiving diving;

        GameData() {}
        ~GameData() {}
    } data;

    GameInfo() : game_type(HURDLES), data() {}
    ~GameInfo() {}
};

// ----- FONCTIONS DE PONDERATION ---------------------------- //

int ponder_DIVING(const GameDiving& game, const Move test)
{
    int ret = 0;

    if (game.diving_is_lost || game.diving_is_won)
        return 0;
    
    switch (test) {
        case LEFT:
            if (game.gpu[0] == 'L')
                ret += 2 + game.my_combo;
            else
                ret -= game.my_combo;
            break;
        case DOWN:
            if (game.gpu[0] == 'D')
                ret += 2 + game.my_combo;
            else
                ret -= game.my_combo;
            break;
        case RIGHT:
            if (game.gpu[0] == 'R')
                ret += 2 + game.my_combo;
            else
                ret -= game.my_combo;
            break;
        case UP:
            if (game.gpu[0] == 'U')
                ret += 2 + game.my_combo;
            else
                ret -= game.my_combo;
            break;
        default:
            ret = -INT_MAX;
            break;
    }

    if (tour > 17 && scores[player_idx].diving.gold == 0 && ret > 0)
        ret += 5;
    // if (tour < 7 && ret > 0)
    //     ret = 100;

    // Priorité
    int priority = 1;
    if (game.go_silver)
        priority = 2;
    if (scores[player_idx].diving.bronze && (scores[player_idx].diving.gold - scores[player_idx].diving.bronze) < 1)
        priority = 4;

    return (ret * priority);
}

int ponder_SKATING(const GameSkating& game, const Move test)
{
    /*
        4 actions in random order
        action index 0 -> move 1 space + decrease risk by 1
        action index 1 -> move 2 spaces
        action index 2 -> move 2 spaces + increase risk by 1
        action index 3 -> move 3 spaces + increase risk by 2
        if after a move, player is on same space as an opponent -> increase risk by 2
        risk range from 0 to 5. At 5 or more : player stun 2 turns + risk reset to 0
        Cyclical track 10 spaces long !

        A faire : 
        - code pour selectionner le bon index, convertir en action, puis vote
        - si risk = 0 -> privilegier index 3 puis 1 puis 2 (pas 0)
        - si risk = 1 -> privilegier index 3 puis 1 puis 2 (pas 0)
        - si risk = 2 -> privilegier index 1 puis 2 puis 3 (pas 0)
        - si risk = 3 -> privilegier index 0 puis 1 puis 2 (pas 3)
        - si risk = 4 -> privilegier index 0 puis 1 (pas 2 ni 3)
    */

    // transformer la gpu en suite de string (qui correspondra à test)
    // vector<string> suite;
    // for (int i = 0; i < game.gpu.size(); i++)
    // {
    //     if (game.gpu[i] == 'L')
    //         suite.push_back("left");
    //     if (game.gpu[i] == 'D')
    //         suite.push_back("down");
    //     if (game.gpu[i] == 'R')
    //         suite.push_back("right");
    //     if (game.gpu[i] == 'U')
    //         suite.push_back("up");
    // }
    vector<Move> suite;
    for (char ch : game.gpu)
    {
        switch (ch) {
            case 'L':
                suite.push_back(LEFT);
                break;
            case 'D':
                suite.push_back(DOWN);
                break;
            case 'R':
                suite.push_back(RIGHT);
                break;
            case 'U':
                suite.push_back(UP);
            default:
                break;
        }
    }

    // affichage
    // for (auto& action : suite)
    //     cerr << "action = " << action << endl;

    // choix du bon index en fonction du risk
    int ret = 0;

    // Code à l'arrache : option 3 pour les 3 premiers tours
    if (suite[3] == test && (game.turns_left == 15 || game.turns_left == 14 || game.turns_left == 13))
        ret += 5;
    return ret;

    // if (game.my_risk == 0 || game.my_risk == 1)
    // {
    //     if (suite[3] == test)
    //         ret += 3;
    //     else if (suite[1] == test)
    //         ret += 2;
    //     else if (suite[2] == test)
    //         ret += 1;
    // }
    // else if (game.my_risk == 2)
    // {
    //     if (suite[1] == test)
    //         ret += 3;
    //     else if (suite[2] == test)
    //         ret += 2;
    //     else if (suite[3] == test)
    //         ret += 1;
    // }
    // else if (game.my_risk == 3)
    // {
    //     if (suite[0] == test)
    //         ret += 3;
    //     else if (suite[1] == test)
    //         ret += 2;
    //     else if (suite[2] == test)
    //         ret += 1;
    // }
    // else if (game.my_risk == 4)
    // {
    //     if (suite[0] == test)
    //         ret += 3;
    //     else if (suite[1] == test)
    //         ret += 2;
    // }

    // Priorité
    int priority = 1;
    // if (scores[player_idx].skating.bronze && (scores[player_idx].skating.gold - scores[player_idx].skating.bronze) < 1)
    //     priority = 4;

    return (ret * priority);
}

int ponder_ARCHERY(const GameArchery& game, const Move test)
{

    int ret = 0;

    switch (test) {
        case LEFT:
            if (game.best_moves.find('L') != game.best_moves.end())
                ret += 10;
            break;
        case DOWN:
            if (game.best_moves.find('D') != game.best_moves.end())
                ret += 10;
            break;
        case RIGHT:
            if (game.best_moves.find('R') != game.best_moves.end())
                ret += 10;
            break;
        case UP:
            if (game.best_moves.find('U') != game.best_moves.end())
                ret += 10;
            break;
        default:
            cerr << "Error in ponder_Archery" << endl;
            break;
    }
    return ret;


    // int ret = 0;
    // int wind_force = game.gpu[0] - '0';
    // int wind_max = game.max_wind - '0';

    // // cerr << "posx = " << game.my_posx << " || posy = " << game.my_posy << " || wind = " << wind_force << " || max_wind = " << game.max_wind << endl;
    // switch (test) {
    //     case LEFT:
    //         if (game.left_endx < game.dist_start)
    //         {
    //             ret += 3;
    //             if (game.left_endx < game.right_endx || game.left_endx < game.down_endy || game.left_endx < game.up_endy)
    //                 ret += 2;
    //             if (wind_force == wind_max)
    //                 ret += 3;
    //         }
    //         break;
    //     case RIGHT: 
    //         if (game.right_endx < game.dist_start)
    //         {
    //             ret += 3;
    //             if (game.right_endx < game.left_endx || game.right_endx < game.down_endy || game.right_endx < game.up_endy)
    //                 ret += 2;
    //             if (wind_force == wind_max)
    //                 ret += 3;
    //         }
    //         break;
    //     case DOWN: 
    //         if (game.down_endy < game.dist_start)
    //         {
    //             ret += 3;
    //             if (game.down_endy < game.up_endy || game.down_endy < game.left_endx || game.down_endy < game.right_endx)
    //                 ret += 2;
    //             if (wind_force == wind_max)
    //                 ret += 3;
    //         }
    //         break;
    //     case UP: 
    //         if (game.up_endy < game.dist_start)
    //         {
    //                 ret += 3;
    //                 if (game.up_endy < game.down_endy || game.up_endy < game.left_endx || game.up_endy < game.right_endx)
    //                     ret += 2;
    //                 if (wind_force == wind_max)
    //                     ret += 3;
    //         }
    //         break;
    //     default:
    //         break;
    // }

    // // Priorité
    // int priority = 1;
    // if (scores[player_idx].archery.bronze && (scores[player_idx].archery.gold - scores[player_idx].archery.bronze) < 1)
    // {
    //     if (ret == 0)
    //         ret = 2;
    //     priority = 4;
    // }
    // return (ret * priority);
}

int best_parcours(const GameHurdles& game, const string& bestseq)
{
    int dist = 0;

    // cerr << bestseq << endl;
    for (int i = 0; i < bestseq.size(); i++)
    {
        if (bestseq[i] == 'L')
            dist += 1;
        else if (bestseq[i] == 'D')
            dist += 2;
        else if (bestseq[i] == 'R')
            dist += 3;
        else if (bestseq[i] == 'U')
            dist += 2;
    }
    return dist ;
}


int ponder_HURDLES(const GameHurdles& game, const Move test)
{
    int ret = 0;

    // cerr << "Hurdles : best_sequence = " << game.my_bestseq << endl;
    // cerr << "Hurdles : best_sequence = " << game.adv1_bestseq << endl;
    // cerr << "Hurdles : best_sequence = " << game.adv2_bestseq << endl;


    // Conditions d'abandon
    int my_dist = best_parcours(game, game.my_bestseq);
    int adv1_dist = best_parcours(game, game.adv1_bestseq);
    int adv2_dist = best_parcours(game, game.adv2_bestseq);
    // cerr << my_dist << " " << adv1_dist << " " << adv2_dist << " tour restant = " << 100 - tour << " seq size = "<< game.my_bestseq.size() << endl;
    if (game.my_bestseq.size() > (100 - tour)) // je n'ai plus le temps (dernier jeu) -> arret vote !
    {
        // cerr << "Hurdles : abandon, plus assez de tours !" << endl;
        return 0;
    }

    if ((game.my_pos + my_dist) < (game.adv1_pos + adv1_dist) && (game.my_pos + my_dist) < (game.adv2_pos + adv2_dist))
    {
        // cerr << "Hurdles : abandon !" << endl;
        return 0;
    }

    switch (test) {
        case LEFT:
            if (game.gpu[game.my_pos + 1] == '#')
                ret -= 1;
            else
                ret += 3;
            break;
        case DOWN:
            if (game.gpu[game.my_pos + 1] == '#' || game.gpu[game.my_pos + 2] == '#')
                ret -= 1;
            else
                ret += 4;
            break;
        case RIGHT:
            if (game.gpu[game.my_pos + 1] == '#' || game.gpu[game.my_pos + 2] == '#' || game.gpu[game.my_pos + 3] == '#')
                ret -= 1;
            else
                ret += 10;
            break;
        case UP:
            if (game.gpu[game.my_pos + 2] == '#')
                ret -= 2;
            else
                ret += 5;
            break;
        default:
            ret = -INT_MAX;
    }

    // Priorité
    int priority = 1;
    if (scores[player_idx].hurdles.bronze && (scores[player_idx].hurdles.gold - scores[player_idx].hurdles.bronze) < 1)
    {
        if (ret <= 0)
            ret = 2;
        priority = 4;
    }
    return (ret * priority);
}

// ----- FIN DES FONCTIONS DE PONDERATION -------------------- //

int eval(const GameInfo& game, const Move test)
{
    int ret = 0;
    switch (game.game_type)
    {
        case HURDLES:
            ret += ponder_HURDLES(game.data.hurdles, test);
            break;
        case ARCHERY:
            ret += ponder_ARCHERY(game.data.archery, test);
            break;
        case SKATING:
            ret += ponder_SKATING(game.data.skating, test);
            break;
        case DIVING:
            ret += ponder_DIVING(game.data.diving, test);
            break;
        default:
            cerr << "Error in eval()" << endl;
            break;
    }
    // cerr << "eval partiel " << test << " = " << ret << endl;
    return ret;
}

// ----- FONCTIONS DE FILL GAMES ---------------------- //

int best_HURDLES(const GameHurdles& game, Move test, const int& newpos)
{
    int ret = 0;
    switch (test) {
        case LEFT:
            if (game.gpu[newpos + 1] == '#')
                ret -= 1;
            else
                ret += 3;
            break;
        case DOWN:
            if (game.gpu[newpos + 1] == '#' || game.gpu[newpos + 2] == '#')
                ret -= 1;
            else
                ret += 4;
            break;
        case RIGHT:
            if (game.gpu[newpos + 1] == '#' || game.gpu[newpos + 2] == '#' || game.gpu[newpos + 3] == '#')
                ret -= 1;
            else
                ret += 10;
            break;
        case UP:
            if (game.gpu[newpos + 2] == '#')
                ret -= 2;
            else
                ret += 5;
            break;
        default:
            ret = -INT_MAX;
    }
    return (ret);
}

void find_BestSeq(GameHurdles& hurdles, int& newpos, string& seq)
{
    vector<string> possible_moves = {"left", "down", "right", "up"};
    string best_move;
    if (!seq.empty())
        seq.clear();

    for (int i = 0; i < hurdles.gpu.size(); i++)
    {
        if (newpos < 0 || newpos >= hurdles.gpu.size())
            break;

        int best_score = 0;
        for(const std::string& move : possible_moves)
        {
            Move enum_move = stringToMove(move);
            int score = best_HURDLES(hurdles, enum_move, newpos);
            // cerr << "hurdles : score after ponder : " << move << " -> " << score << endl;
            if (score > best_score)
            {
                best_score = score;
                best_move = move;
            }
            // cerr << "hurdles : best_move : " << best_move << endl;
        }

        if (best_move == "left")
        {
            seq += 'L';
            newpos += 1;
        }
        else if (best_move == "down")
        {
            seq += 'D';
            newpos += 2;
        }
        else if (best_move == "right")
        {
            seq += 'R';
            newpos += 3;
        }
        else if (best_move == "up")
        {
            seq += "U";
            newpos += 4;
        }
    }
    return;
}

void fillHurdles(GameHurdles& hurdles, const GameRegisters& regs)
{
    hurdles.gpu = regs.gpu;
    hurdles.unused = regs.reg_6;
    switch (player_idx) {
        case 0:
            hurdles.my_pos = regs.reg_0;
            hurdles.adv1_pos = regs.reg_1;
            hurdles.adv2_pos = regs.reg_2;
            hurdles.my_stun = regs.reg_3;
            hurdles.adv1_stun = regs.reg_4;
            hurdles.adv2_stun = regs.reg_5;
            break;
        case 1:
            hurdles.adv1_pos = regs.reg_0;
            hurdles.my_pos = regs.reg_1;
            hurdles.adv2_pos = regs.reg_2;
            hurdles.adv1_stun = regs.reg_3;
            hurdles.my_stun = regs.reg_4;
            hurdles.adv2_stun = regs.reg_5;
            break;
        case 2:
            hurdles.adv1_pos = regs.reg_0;
            hurdles.adv2_pos = regs.reg_1;
            hurdles.my_pos = regs.reg_2;
            hurdles.adv1_stun = regs.reg_3;
            hurdles.adv2_stun = regs.reg_4;
            hurdles.my_stun = regs.reg_5;
            break;
    }

    // etablir la meilleure sequence d'actions
    hurdles.my_newpos = hurdles.my_pos;
    hurdles.adv1_newpos = hurdles.adv1_pos;
    hurdles.adv2_newpos = hurdles.adv2_pos;
    find_BestSeq(hurdles, hurdles.my_newpos, hurdles.my_bestseq);
    // cerr << "Hurdles : best_sequence = " << hurdles.my_bestseq << endl;
    find_BestSeq(hurdles, hurdles.adv1_newpos, hurdles.adv1_bestseq);
    // cerr << "Hurdles : best_sequence = " << hurdles.adv1_bestseq << endl;
    find_BestSeq(hurdles, hurdles.adv2_newpos, hurdles.adv2_bestseq);
    // cerr << "Hurdles : best_sequence = " << hurdles.adv2_bestseq << endl;

}

DataWayArchery searchWayArchery(const int& posx, const int& posy, const string& wind, const string& prev_way)
{
    DataWayArchery data;

    data.left_posx = posx - (wind[0] - '0');
    data.left_posy = posy;
    data.down_posx = posx;
    data.down_posy = posy + (wind[0] - '0');
    data.right_posx = posx + (wind[0] - '0');
    data.right_posy = posy;
    data.up_posx = posx;
    data.up_posy = posy - (wind[0] - '0');

    int left_euc = pow(data.left_posx, 2) + pow(data.left_posy, 2);
    int down_euc = pow(data.down_posx, 2) + pow(data.down_posy, 2);
    int right_euc = pow(data.right_posx, 2) + pow(data.right_posy, 2);
    int up_euc = pow(data.up_posx, 2) + pow(data.up_posy, 2);

    string left_way = prev_way + "L";
    string down_way = prev_way + "D";
    string right_way = prev_way + "R";
    string up_way = prev_way + "U";

    vector<pair<int, string>> result = {
        {left_euc, left_way},
        {down_euc, down_way},
        {right_euc, right_way},
        {up_euc, up_way}
    };

    // for (const auto& elem : result) {
    //     cerr << "Distance: " << elem.first << ", Direction: " << elem.second << endl;
    // }

    data.result = result;

    return data;
}

vector<pair<int, string>> recWayArchery(const DataWayArchery& data_now, const string& wind, const string& prev_way, const int& r)
{
    vector<pair<int, string>> all_data;
    DataWayArchery data_temp;

    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0:
                data_temp = searchWayArchery(data_now.left_posx, data_now.left_posy, &wind[1], prev_way + "L");
                if (data_temp.result.begin()->second.size() < r && wind.size() > 1)
                    data_temp.result = recWayArchery(data_temp, &wind[1], prev_way + "L", r);
                break;
            case 1:
                data_temp = searchWayArchery(data_now.down_posx, data_now.down_posy, &wind[1], prev_way + "D");
                if (data_temp.result.begin()->second.size() < r && wind.size() > 1)
                    data_temp.result = recWayArchery(data_temp, &wind[1], prev_way + "D", r);
                break;
            case 2:
                data_temp = searchWayArchery(data_now.right_posx, data_now.right_posy, &wind[1], prev_way + "R");
                if (data_temp.result.begin()->second.size() < r && wind.size() > 1)
                    data_temp.result = recWayArchery(data_temp, &wind[1], prev_way + "R", r);
                break;
            case 3:
                data_temp = searchWayArchery(data_now.up_posx, data_now.up_posy, &wind[1], prev_way + "U");
                if (data_temp.result.begin()->second.size() < r && wind.size() > 1)
                    data_temp.result = recWayArchery(data_temp, &wind[1], prev_way + "U", r);
                break;
            default:
                cerr << "nique, je suis ou encore la - Er[bestWayArchery]" << endl;
                break;
        }
        all_data.insert(all_data.end(), data_temp.result.begin(), data_temp.result.end());
    }
    return all_data;
};

vector<pair<int, string>> allWayArchery(const int& posx, const int& posy, const string& wind, int r)
{
    // cerr << "Pos : " << posx << ";" << posy << " | Wind : " << wind <<endl;

    DataWayArchery data_now = searchWayArchery(posx, posy, wind, "");

    if (wind.size() <= 1) {
        return data_now.result;
    }

    if (r > wind.size()) {r = wind.size();}
    vector<pair<int, string>> all_data = recWayArchery(data_now, wind, "", r);

    // cerr << endl;
    // for (const auto& elem : all_data) {
    //     cerr << "Distance: " << elem.first << ", Direction: " << elem.second << endl;
    // }
    // cerr << endl;

    return all_data;
};

void fillArchery(GameArchery& archery, const GameRegisters& regs)
{
    archery.gpu = regs.gpu;
    archery.unused = regs.reg_6;
    // archery.max_is_last = false;
    switch (player_idx) {
        case 0:
            archery.my_posx = regs.reg_0;
            archery.my_posy = regs.reg_1;
            archery.adv1_posx = regs.reg_2;
            archery.adv1_posy = regs.reg_3;
            archery.adv2_posx = regs.reg_4;
            archery.adv2_posy = regs.reg_5;
            break;
        case 1:
            archery.adv1_posx = regs.reg_0;
            archery.adv1_posy = regs.reg_1;
            archery.my_posx = regs.reg_2;
            archery.my_posy = regs.reg_3;
            archery.adv2_posx = regs.reg_4;
            archery.adv2_posy = regs.reg_5;
            break;
        case 2:
            archery.adv1_posx = regs.reg_0;
            archery.adv1_posy = regs.reg_1;
            archery.adv2_posx = regs.reg_2;
            archery.adv2_posy = regs.reg_3;
            archery.my_posx = regs.reg_4;
            archery.my_posy = regs.reg_5;
            break;
    }

    vector<pair<int, string>> data = allWayArchery(archery.my_posx, archery.my_posy, archery.gpu, 6);
    auto best_it = min_element(data.begin(), data.end());

    set<char> bestMoves;
    for (const auto& elem : data) {
        if (elem.first == best_it->first) {
            // cerr << "Possibility = " << elem.second << " | Dist euc = " << elem.first << endl;
            bestMoves.insert(elem.second[0]);
        }
    }

    archery.best_moves = bestMoves;

    /* AFFICHAGE */
    /* cerr << endl;
    for (const auto& elem : bestMoves) {
        cerr << "Stocked : " << elem << endl;
    }
    cerr << endl; */
    // cerr << "Best combo = " << best_it->second << " | Dist euc = " << best_it->first << endl;

    // ----- fin code E ----- // 

    // // je cherche le vent maximum
    // archery.max_wind = *max_element(regs.gpu.begin(), regs.gpu.end()); // ATTENTION c'est un char
    // if (*(regs.gpu.end() - 1) == archery.max_wind)
    //     archery.max_is_last = true;
    // // cerr << "Max Wind for Archery = " << archery.max_wind << endl;

    // // calcul des distances
    // archery.dist_start = pow(archery.my_posx, 2) + pow(archery.my_posy, 2);
    // archery.left_endx = pow((archery.my_posx - (archery.gpu[0] - '0')), 2) + pow(archery.my_posy, 2); // (left)
    // archery.right_endx = pow((archery.my_posx + (archery.gpu[0] - '0')), 2) + pow(archery.my_posy, 2); // (right)
    // archery.down_endy = pow(archery.my_posx, 2) + pow((archery.my_posy + (archery.gpu[0] - '0')), 2); // (down)
    // archery.up_endy = pow(archery.my_posx, 2) + pow((archery.my_posy - (archery.gpu[0] - '0')), 2); // (up)
    // // cerr << archery.dist_start << " " << archery.left_endx << " " << archery.right_endx << " " << archery.down_endy << " " << archery.up_endy << endl;
}

void fillSkating(GameSkating& skating, const GameRegisters& regs)
{
    skating.gpu = regs.gpu;
    skating.turns_left = regs.reg_6;
    switch (player_idx) {
        case 0:
            skating.my_way = regs.reg_0;
            skating.adv1_way = regs.reg_1;
            skating.adv2_way = regs.reg_2;
            skating.my_risk = regs.reg_3;
            skating.adv1_risk = regs.reg_4;
            skating.adv2_risk = regs.reg_5;
            break;
        case 1:
            skating.adv1_way = regs.reg_0;
            skating.my_way = regs.reg_1;
            skating.adv2_way = regs.reg_2;
            skating.adv1_risk = regs.reg_3;
            skating.my_risk = regs.reg_4;
            skating.adv2_risk = regs.reg_5;
            break;
        case 2:
            skating.adv1_way = regs.reg_0;
            skating.adv2_way = regs.reg_1;
            skating.my_way = regs.reg_2;
            skating.adv1_risk = regs.reg_3;
            skating.adv2_risk = regs.reg_4;
            skating.my_risk = regs.reg_5;
            break;
    }
}

void fillDiving(GameDiving& diving, const GameRegisters& regs)
{
    diving.gpu = regs.gpu;
    diving.unused = regs.reg_6;
    switch (player_idx) {
        case 0:
            diving.my_point = regs.reg_0;
            diving.adv1_point = regs.reg_1;
            diving.adv2_point = regs.reg_2;
            diving.my_combo = regs.reg_3;
            diving.adv1_combo = regs.reg_4;
            diving.adv2_combo = regs.reg_5;
            break;
        case 1:
            diving.adv1_point = regs.reg_0;
            diving.my_point = regs.reg_1;
            diving.adv2_point = regs.reg_2;
            diving.adv1_combo = regs.reg_3;
            diving.my_combo = regs.reg_4;
            diving.adv2_combo = regs.reg_5;
            break;
        case 2:
            diving.adv1_point = regs.reg_0;
            diving.adv2_point = regs.reg_1;
            diving.my_point = regs.reg_2;
            diving.adv1_combo = regs.reg_3;
            diving.adv2_combo = regs.reg_4;
            diving.my_combo = regs.reg_5;
            break;
    }

    diving.diving_is_won = false;
    diving.diving_is_lost = false;
    diving.go_silver = false;

    int best_adv1 = diving.adv1_point;
    int best_adv2 = diving.adv2_point;
    int combo1 = diving.adv1_combo;
    int combo2 = diving.adv2_combo;
    int my_best = diving.my_point;
    int my_combo = diving.my_combo;

    for (int i = 0; i < diving.gpu.size(); i++)
    {
        combo1++;
        combo2++;
        my_combo++;
        best_adv1 += combo1;
        best_adv2 += combo2;
        my_best += my_combo++;
    }
    if (diving.my_point >= best_adv1 && diving.my_point >= best_adv2) // j'ai deja gagne -> stop vote !
    {
        // cerr << "Diving : Je gagne quoi que je fasse ! Stop voting donc ret = 0 !" << endl;
        diving.diving_is_won = true;
    }
    if (my_best < best_adv1 && my_best < best_adv2) // j'ai perdu quoi que je fasse -> stop vote
    {
        // cerr << "Diving : Je perds quoi que je fasse ! Stop voting donc ret = 0 !" << endl;
        diving.diving_is_lost = true;
    }
    if ((diving.my_point >= best_adv1 && my_best <= best_adv2) || (diving.my_point >= best_adv2 && my_best <= best_adv1))
    {
        // cerr << "Diving : On vise l'argent" << endl;
        diving.go_silver = true;
    }
}

// ----- FIN DES FONCTIONS DE FILL GAMES --------------- //

vector<PlayerScore> parseScores(const vector<string>& score_strings) {
    vector<PlayerScore> player_scores;
    for (const auto& score_str : score_strings) {
        PlayerScore scores;
        istringstream iss(score_str);
        iss >> scores.total_points
            >> scores.hurdles.gold >> scores.hurdles.silver >> scores.hurdles.bronze
            >> scores.archery.gold >> scores.archery.silver >> scores.archery.bronze
            >> scores.skating.gold >> scores.skating.silver >> scores.skating.bronze
            >> scores.diving.gold >> scores.diving.silver >> scores.diving.bronze;

        player_scores.push_back(scores);
    }
    return player_scores;
}

int main()
{
    // int player_idx;
    cin >> player_idx; cin.ignore();
    int nb_games;
    cin >> nb_games; cin.ignore();
    
    // on initialise un vecteur de structure de nb_games lines, pour enregistrer toutes les info des jeux
    vector<GameInfo> arcades(nb_games);
    GameHurdles hurdles;
    GameArchery archery;
    GameSkating skating;
    GameDiving diving;

    score_strings.resize(3); //3 players
    string action; // LEFT DOWN RIGHT UP
    tour = 0;

    // game loop
    while (1) {
        tour++;
        for (int i = 0; i < 3; i++) {
            string score_info;
            getline(cin, score_info);

            // on remplit notre tableau de score (var globale)
            score_strings[i] = score_info;
        }
        // Convertir les chaînes de scores en structures PlayerScores
        scores = parseScores(score_strings);

        for (int i = 0; i < nb_games; i++) {
            GameRegisters regs;
            cin >> regs.gpu >> regs.reg_0 >> regs.reg_1 >> regs.reg_2 >> regs.reg_3 >> regs.reg_4 >> regs.reg_5 >> regs.reg_6; cin.ignore();

            switch (i) {
                case 0:
                    arcades[i].game_type = HURDLES;
                    fillHurdles(arcades[i].data.hurdles, regs);
                    break;
                case 1:
                    arcades[i].game_type = ARCHERY;
                    fillArchery(arcades[i].data.archery, regs);
                    break;
                case 2:
                    arcades[i].game_type = SKATING;
                    fillSkating(arcades[i].data.skating, regs);
                    break;
                case 3:
                    arcades[i].game_type = DIVING;
                    fillDiving(arcades[i].data.diving, regs);
                    break;
            }
        }

        // Affichage du tableau des scores
        // cerr << "\nTABLEAU DES SCORES" << endl;
        // for (auto& score : score_strings)
        //     cerr << "score = " << score << endl;
        // for (const auto& score : scores)
        // {
        //     std::cerr << "Total Points: " << score.total_points << "\n";
        //     std::cerr << "HURDLES - Gold: " << score.hurdles.gold
        //               << ", Silver: " << score.hurdles.silver
        //               << ", Bronze: " << score.hurdles.bronze << "\n";
        //     std::cerr << "ARCHERY - Gold: " << score.archery.gold
        //               << ", Silver: " << score.archery.silver
        //               << ", Bronze: " << score.archery.bronze << "\n";
        //     std::cerr << "SKATING - Gold: " << score.skating.gold
        //               << ", Silver: " << score.skating.silver
        //               << ", Bronze: " << score.skating.bronze << "\n";
        //     std::cerr << "DIVING - Gold: " << score.diving.gold
        //               << ", Silver: " << score.diving.silver
        //               << ", Bronze: " << score.diving.bronze << "\n";
        // }

        // Évaluer les actions pour chaque partie et choisir la meilleure
        int eval_left = 0;
        int eval_down = 0;
        int eval_right = 0;
        int eval_up = 0;

        for (int i = 0; i < nb_games; i++)
        {
            // if (i == 0)
            //     cerr << "JEU HURDLE :   " << arcades[i].data.hurdles.gpu << endl;
            // else if (i == 1)
            //     cerr << "JEU ARCHERY :  " << arcades[i].data.archery.gpu << endl;
            // else if (i == 2)
            //     cerr << "JEU SKATING :  " << arcades[i].data.skating.gpu << endl;
            // else if (i == 3)
            //     cerr << "JEU DIVING :   " << arcades[i].data.diving.gpu << endl;
            
            if (hurdles.gpu == "GAME_OVER" || archery.gpu == "GAME_OVER" || skating.gpu == "GAME_OVER" || diving.gpu == "GAME_OVER") {
                continue;
            }

            if (arcades[i].game_type == HURDLES && arcades[i].data.hurdles.my_stun != 0) {
                continue;
            }
            if (arcades[i].game_type == SKATING && arcades[i].data.skating.my_risk < 0) {
                continue;
            }
            // On va faire une évaluation pour chaque jeu !
            eval_left += eval(arcades[i], LEFT);
            eval_down += eval(arcades[i], DOWN);
            eval_right += eval(arcades[i], RIGHT);
            eval_up += eval(arcades[i], UP);
        }

        // cerr << "Evaluations : " << endl;
        // cerr << "LEFT = " << eval_left << endl;
        // cerr << "DOWN = " << eval_down << endl;
        // cerr << "RIGHT = " << eval_right << endl;
        // cerr << "UP = " << eval_up << endl;

        int best_eval = eval_left;
        action = "LEFT";
        if (eval_down > best_eval)
        {
            best_eval = eval_down;
            action = "DOWN";
        }
        if (eval_right > best_eval)
        {
            best_eval = eval_right;
            action = "RIGHT";
        }
        if (eval_up > best_eval)
        {
            best_eval = eval_up;
            action = "UP";
        }
        // cerr << "BEST_EVAL suivant ret = " << best_eval << " || ACTION = " << action << endl;
        
        std::cout << action << std::endl;
    }
}