#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <boost/asio.hpp>
#include <boost/winapi/pipes.hpp>
#include <boost/log/utility/setup/console.hpp>

using namespace std;

// Constants for the different moves
const int ATTACK = 1;
const int REVOLVER = 2;
const int RELOAD = 3;
const int FAN = 4;
const int HEAL = 5;
const int POISON = 6;
const int SHIELD = 7;
const int COVER = 8;
const int WIN_REWARD = 20;
const int SURVIVAL_REWARD = 2;
const int WASTE_PENALTY = 6;

struct GameState;
class Character;
GameState get_current_state(Character player, Character enemy);
GameState execute_action(int action, int move, Character& player, Character& enemy);
float calculate_reward(GameState state, int minus);
void handleMove(int move, Character& player, Character& enemy);
int handleAIMove(int move, Character& player, Character& enemy, int enMove);
bool win(Character& player, Character& enemy);
bool lose(Character& player, Character& enemy);
void reset_player(Character& player);

// Define the state and action spaces
const int PLAYER_HP_STATES = 11;
const int PLAYER_SHIELDED_STATES = 2;
const int PLAYER_HEALING_STATUS = 4;
const int PLAYER_COVER_STATUS = 2;
const int AI_HP_STATES = 11;
const int AI_SHOTS_STATES = 7;
const int AI_SHIELD_STATES = 2;
const int AI_SHIELDED_STATES = 2;
const int AI_HEALING_STATUS = 4;
const int AI_POISON_STATUS = 2;
const int AI_COVER_STATUS = 2;
const int AI_COVERED_STATUS = 2;

const float NUM_STATES = PLAYER_HP_STATES *PLAYER_SHIELDED_STATES *PLAYER_HEALING_STATUS 
*PLAYER_COVER_STATUS *AI_HP_STATES *AI_SHOTS_STATES *AI_SHIELD_STATES *AI_SHIELDED_STATES *AI_HEALING_STATUS *AI_POISON_STATUS
*AI_COVER_STATUS;// 1.6436706e+15, 1,6 quadrilion, 14 quadrilion options
const int NUM_ACTIONS = 8;

// Initialize the Q-table with zeros
vector<vector<float>> Q_table;

void initialize_Q_table() {
    Q_table.resize(NUM_STATES, vector<float>(NUM_ACTIONS));
    if (Q_table.size() != NUM_STATES || Q_table[0].size() != NUM_ACTIONS) {
        cout << "Error: Q_table initialization failed!" << endl;
    }
    else {
        for (int i = 0; i < NUM_STATES; ++i) {
            for (int j = 0; j < NUM_ACTIONS; ++j) {
                Q_table.at(i).at(j) = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            }
        }
    }

}

struct GameState {
    int player_hp;
    bool player_shielded;
    int player_healing;
    int player_Cover;
    int ai_hp;
    int ai_shots;
    bool ai_shieldUsed;
    bool ai_shielded;
    int ai_healing;
    int ai_poison;
    int ai_Cover;
    int ai_Covered;
    int damage_dealt_to_player;
};

// Convert a game state to an index for the Q-table
int state_to_index(GameState state) {
    int index = 0;
    // Linear mapping of state variables to an index
    index += state.player_hp;
    index *= PLAYER_SHIELDED_STATES;
    index += state.player_shielded ? 1 : 0;
    index += state.player_healing;
    index *= AI_HP_STATES;
    index += state.ai_hp;
    index *= AI_SHOTS_STATES;
    index += state.ai_shots;
    index *= AI_SHIELD_STATES;
    index += state.ai_shieldUsed ? 1 : 0;
    index *= AI_SHIELDED_STATES;
    index += state.ai_shielded ? 1 : 0;
    index *= AI_HEALING_STATUS;
    index += state.ai_healing;
    index *= AI_POISON_STATUS;    
    index += state.ai_poison;
    index *= AI_COVER_STATUS;    
    index += state.ai_Cover;
    index *= AI_COVERED_STATUS;
    index += state.ai_Covered;

    return index;
}

// Choose an action based on the current state
int choose_action(GameState state) {
    int state_index = state_to_index(state);
    int best_action = -1;
    float best_q_value = -1;

    for (int action = 0; action < NUM_ACTIONS; ++action) {
        if (Q_table[state_index][action] > best_q_value) {
            best_q_value = Q_table[state_index][action];
            best_action = action;
        }
    }
    return best_action;
}

// Update the Q-table based on the reward received
void update_Q_table(GameState state, int action, float reward, GameState next_state) {
    int state_index = state_to_index(state);
    int next_state_index = state_to_index(next_state);

    float max_next_Q = 0;
    for (int next_action = 0; next_action < NUM_ACTIONS; ++next_action) {
        if ((Q_table[next_state_index][next_action]) > (max_next_Q)) {
            max_next_Q = Q_table[next_state_index][next_action];
        }
    }
    float learning_rate = 0.5;
    float discount_factor = 0.9;
    Q_table[state_index][action] = Q_table[state_index][action] + learning_rate * (reward + discount_factor * max_next_Q 
        - Q_table[state_index][action]);
}

class Character {
public:
    int hp;
    int shots;
    bool shieldUsed;
    bool shielded;
    int attackDmg;
    int healing;
    int poison;
    int poisoned;
    int Covered;
    int CoverT;
    bool ActiveCover;
    bool isAI;

    Character( int hp, int shots, bool shieldUsed, bool shielded, int attackDmg, int healing, int poison, int poisoned,
        int Covered, int CoverT, bool ActiveCover, bool isAI): hp(hp), 
        shots(shots), shieldUsed(shieldUsed), shielded(shielded), attackDmg(attackDmg), healing(healing), poison(poison), poisoned(poisoned), 
         Covered(Covered), CoverT(CoverT), ActiveCover(ActiveCover), isAI(isAI) {
    }

    int do_ai_stuff(Character& player, Character& enemy, int move) {
        int AIMove;
        int minus = 0;
        int no[8]{};
        int j = 0;
        bool e=1;
        bool r = 1;
       
        for (int i = 0; i < 8; i++) {
            no[i] = 10;
        }
        GameState current_state = get_current_state(player, enemy);
        float epsilon = 0.2; // Set your exploration rate (epsilon) as needed
        while (e!=0){
            if (rand() / static_cast<float>(RAND_MAX) < epsilon) {
                AIMove = rand() % NUM_ACTIONS; // Explore (choose a random action)
            }
            else {
                AIMove = choose_action(current_state); // Exploit (choose the action with the highest Q-value)
            }
            for (int i = 0; i < 8; i++) {
                if (AIMove == no[i]) {
                    minus += 20;
                }
            }
            cout << "The AI generated the Move: " << 1+AIMove << endl;
            GameState next_state = execute_action(AIMove, move, player, enemy);
            if (next_state.ai_hp==-100) {
                minus += 5;
                e = 1;
                for (int i = 0; i < 8; i++) {
                    if (AIMove == no[i]) {
                        r = 0;
                    }
                }
                if (r) {
                    no[j] = AIMove;
                    j++;
                }
            }
            else {
                float reward = calculate_reward(next_state, minus);
                update_Q_table(current_state, AIMove, reward, next_state);
                e = 0;
            }
        }
        return AIMove;
    }
};

GameState get_current_state(Character player, Character enemy) {
    GameState current_state;
    current_state.player_hp = (player.hp/10);
    current_state.player_shielded = player.shielded;
    current_state.player_healing = player.healing;
    current_state.player_Cover = player.ActiveCover;
    current_state.ai_hp = (enemy.hp/10);
    current_state.ai_shots = enemy.shots;
    current_state.ai_shieldUsed = enemy.shieldUsed;
    current_state.ai_shielded = enemy.shielded;
    current_state.ai_healing = enemy.healing;
    current_state.ai_poison = enemy.poison;
    current_state.ai_Cover = enemy.ActiveCover;
    current_state.ai_Covered = enemy.Covered;

    // Additional game-specific state information can be included here

    return current_state;
}

// Function to execute an action and update the game state
GameState execute_action(int action, int move, Character& player, Character& enemy) {
    GameState next_state = get_current_state(player, enemy);
    GameState Error;

    if (player.isAI) {
        // AI's action based on Q-learning logic
        action = enemy.do_ai_stuff(enemy, player, move);
    }

    int reward = handleAIMove(action, player, enemy, move);
    if (reward == 100) {
        Error.ai_hp = -100;
        return Error;
    }

    // Update the AI's state within next_state
    next_state = get_current_state(player, enemy);
    next_state.damage_dealt_to_player = reward;

    return next_state;
}

float calculate_reward(GameState state, int minus) {
    float reward = 0 - minus;
    reward += state.damage_dealt_to_player;

    if (state.player_hp <= 0) {
        reward -= WASTE_PENALTY;
    }
    else if (state.ai_hp <= 0) {
        reward += WIN_REWARD;
    }

    reward += SURVIVAL_REWARD;

    reward += (state.ai_hp);

    return reward;
}

int handleAIMove(int move, Character& chara1, Character& chara2, int enMove) {
    bool sh = 0;
    bool cv = 0;
    int reward = chara1.hp;
    int hp = chara2.hp;

    if (chara2.isAI == 1) {

        if (enMove == SHIELD) {
            sh = 1;
        }if (enMove == COVER) {
            cv = 1;
        }

        ////AI MOVES
        if (move+1 == ATTACK) {
            if ((chara2.Covered < 1) && (chara2.CoverT > 0)) {
                cout << "[AI tried to use the Saber while in Cover.]" << endl;
                return 100;
            }
            cout << "[The Cyborg sliced you with its sword. It deals you " << 1 * (chara2.attackDmg * (sh ? 0 : 1) *
                (chara1.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1)) << " damage.]" << endl;
            chara1.hp -= 1 * chara2.attackDmg * (sh ? 0 : 1) * (chara1.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1);
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
        else if (move+1 == FAN) {
            int i = 4;
            if (chara2.shots < i) {
                cout << "[The Cyborg tried to Fan the Gun while not having enough shots.]" << endl;
                return 100;
            }

            cout << "[The Cyborg Fans their Gun!]" << endl;
            while (i > 0) {
                chara1.hp -= 1.5 * chara2.attackDmg * (sh ? 0 : 1) *(chara1.ActiveCover ? 0.5 : 1) *(cv ? 0.5 : 1);
                chara2.shots -= 1;
                i--;
                cout << "[A Shot hits you for " << 1.5 * chara2.attackDmg * (sh ? 0 : 1) *(chara1.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1)
                    << " damage!]" << endl;
            }
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
        else if (move+1 == REVOLVER) {
            if (chara2.shots < 1) {
                cout << "[The Cyborg tries to use the Revolver without having enough shots.]" << endl;
                return 100;
            }
            cout << "[The Cyborg shoots you with its Revolver! It deals you " << 1.5 * chara2.attackDmg * (sh ? 0 : 1)
                * (chara1.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1) << " damage.]" << endl;
            chara1.hp -= 1.5 * chara2.attackDmg * (chara1.shielded ? 0 : 1) * (chara1.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1);
            chara2.shots -= 1;
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
        else if (move+1 == RELOAD) {
            cout << "[The Cyborg Reloads their Revolver.]" << endl;
           if(chara2.shots > 3) {
                reward -= 5;
            }
            chara2.shots = 6;
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
        else if (move+1 == HEAL) {
            if (chara2.healing < 1) {
                cout << "[The Cybrog tried to heal without having any Healing Packs left.]" << endl;
                return 100;
            }
            cout << "[The Cyborg Heal its wounds! It heals " << 2 * chara2.attackDmg << " wounds.]" << endl;
            chara2.hp += 2 * chara2.attackDmg;
            if (chara2.hp > 100) {
                reward = reward + 100 - chara2.hp;
                chara2.hp = 100;
            }
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
        else if (move+1 == POISON) {
            if (chara2.poison < 1) {
                cout << "[The Cyborg tries to poison you without having any more Poisons.]" << endl;
                return 100;
            }
            cout << "[The Cyborg Poisons you! You will be poisoned for 3 Turns. This damage bypasses Cover and Reflexes.]" << endl;
            chara1.poisoned = 3 * (chara1.shielded ? 0 : 1);
            chara2.poison -= 1;
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
        else if (move+1 == SHIELD) {
            if (chara2.shieldUsed) {
                cout << "[The Cyborg tried to use its Lightning Reflexes again but failed.]" << endl;
                return 100;
            }
            cout << "[The Cyborg is incredibly quick, it evade any attacks and set up for a devestating counter attack or heal!]" << endl;
            chara2.shieldUsed = true;
            chara2.shielded = true;
            chara2.attackDmg = 8;
        }
        else if (move+1 == COVER) {
            if (chara2.Covered < 1) {
                cout << "[The Cyborg tries to find Cover but fails.]" << endl;
                return 100;
            }
            cout << "[The Cyborg jump into Cover! It will take half damage for 3 turns, but it wont be able to use its Saber.]" << endl;
            chara2.Covered -= 1;
            chara2.ActiveCover = 1;
            chara2.attackDmg = 4;
            chara2.shielded = false;
        }
    }
    reward -= chara1.hp;
    int newhp = chara2.hp;
    newhp -= hp;
    reward += newhp;
    return reward;
}

void handleMove(int move, Character& char1, Character& char2) {
    ////PLAYER MOVES
    if (move == ATTACK) {
        if ((char1.Covered < 1) && (char1.CoverT > 0)) {
            cout << "[You cant use your Saber while in Cover. You are in cover for "<< char1.CoverT <<" more turns.]" << endl;
            return;
        }
        cout << "[You sliced the Cyborg with your sword. You deal " << 1 * char1.attackDmg * (char2.shielded ? 0 : 1) *
            (char2.ActiveCover ? 0.5 : 1) << " damage.]" << endl;
        char2.hp -= 1 * char1.attackDmg * (char2.shielded ? 0 : 1) * (char2.ActiveCover ? 0.5 : 1);
        char1.attackDmg = 4;
        char1.shielded = false;
    }
    else if (move == FAN) {
        int i = 4;
        if (char1.shots < i) {
            cout << "[You don't have enough shots to Fan the Gun.]" << endl;
            return;
        }

        cout << "[You Fan the Gun!]" << endl;
        while (i > 0) {
            char2.hp -= 1.5 * char1.attackDmg * (char2.shielded ? 0 : 1) * (char2.ActiveCover ? 0.5 : 1);
            char1.shots -= 1;
            i -= 1;
            cout << "[A Shot hits the Cyborg for " << 1.5 * char1.attackDmg * (char2.shielded ? 0 : 1) * (char2.ActiveCover ? 0.5 : 1) << " damage!]" << endl;
        }
        char1.attackDmg = 4;
        char1.shielded = false;
    }
    else if (move == RELOAD) {
        cout << "[You Reload your Revolver.]" << endl;
        char1.shots = 6;
        char1.attackDmg = 4;
        char1.shielded = false;
    }
    else if (move == REVOLVER) {
        if (char1.shots < 1) {
            cout << "[You don't have any shots in the Revolver.]" << endl;
            return;
        }
       
        cout << "[Direect Hit! You deal " << 1.5 * char1.attackDmg * (char2.shielded ? 0 : 1) * (char2.ActiveCover ? 0.5 : 1)
            << " damage to the Cyborg.]" << endl;
        char2.hp -= 1.5 * char1.attackDmg * (char2.shielded ? 0 : 1) * (char2.ActiveCover ? 0.5 : 1);
        char1.shots -= 1;
        char1.attackDmg = 4;
        char1.shielded = false;
    }
    else if (move == HEAL) {
        if (char1.healing < 1) {
            cout << "[You don't have any Healing Packs left.]" << endl;
            return;
        }
        cout << "[You Heal your wounds! You heal " << 2 * char1.attackDmg << " wounds, up to your mximum.]" << endl;
        char1.hp += 2 * char1.attackDmg;
        if (char1.hp > 100) {
            char1.hp = 100;
        }
        char1.healing -= 1;
        char1.attackDmg = 4;
        char1.shielded = false;
    }
    else if (move == POISON) {
        if (char1.poison < 1) {
            cout << "[You don't have any more Poisons.]" << endl;
            return;
        }
        cout << "[You manage to Poison your Enemy! He will be poisoned for 3 Turns. This damage bypasses Cover.]" << endl;
        char2.poisoned = 3 * (char2.shielded ? 0 : 1);
        char1.poison -= 1;
        char1.attackDmg = 4;
        char1.shielded = false;
    }
    else if (move == SHIELD) {
        if (char1.shieldUsed) {
            cout << "[You have already used your Lightning Reflexes.]" << endl;
            return;
        }
        cout << "[You use the burst of speed to evade any attack and set up for a devestating counter attack or heal!]" << endl;
        char1.shieldUsed = true;
        char1.shielded = true;
        char1.attackDmg = 8;
    }
    else if (move == COVER) {
        if (char1.Covered < 1) {
            cout << "[You have already used your Cover.]" << endl;
            return;
        }
        cout << "[You expertly jump into Cover! You will take half damage for 3 turns, but you wont be able to use your Saber.]" << endl;
        char1.Covered -= 1;
        char1.ActiveCover = 1;
        char1.attackDmg = 4;
        char1.shielded = false;
    }
}

bool win(Character& player, Character& enemy, int score) {
    string response;
    while (true) {
        cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "You win this battle, but more are coming. Would you like to play on? (yes/no)" << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            score = score + player.hp + 8*player.healing;
            reset_player(player);
            reset_player(enemy);
            cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
                << "You have defeated the enemy, but the Cyborgs have adapted, a new, better Cyborg takes his place he next noon, ready to fight." << endl
                << "Your Current Score is: "<< score << endl
                << "----------------------------------------------------------------------------------------------------------------------------------" << endl << endl;
            return true;
        }
        else if (response == "no") {
            cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
                << "That's alright Cowboy, the Frontier will wait. See you next time." << endl
                << "Your Final Score is: " << score << endl
                << "----------------------------------------------------------------------------------------------------------------------------------" << endl << endl;
            return false;
        }
        else {
            cout << "Invalid response. Please enter 'yes' or 'no'." << endl;
        }
    }
}

bool lose(Character& player, Character& enemy, int score) {
    string response;
    while (true) {
        cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "You have lost the battle, but we have recovered you and rebuild you. Would you like to play on? (yes/no)" << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            score = 0;
            cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
                << "That is the spirit Cowboy! Be prepared, your first duel will begin soon." << endl
                << "Your Current Score is: " << score << endl
                << "----------------------------------------------------------------------------------------------------------------------------------" << endl << endl;
            reset_player(player);
            reset_player(enemy);
            return true;
        }
        else if (response == "no") {
            cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
                << "That's alright Cowboy, the Frontier will wait. See you next time." << endl
                << "Your Final Score was: " << score << endl
                << "----------------------------------------------------------------------------------------------------------------------------------" << endl << endl;
            return false;
        }
        else {
            cout << "Invalid response. Please enter 'yes' or 'no'." << endl;
        }
    }
}

void reset_player(Character& chara) {
    chara.hp = 100;
    chara.shots = 6;
    chara.shieldUsed = false;
    chara.shielded = false;
    chara.attackDmg = 2;
    chara.healing = 3;
    chara.poison = 1;
    chara.poisoned = 0;
    chara.Covered = 1;
    chara.CoverT = 3;
}

int getValidMove() {
    int input;
    while (true) {
        cout << "Enter a move (1-8): ";
        if (cin >> input) {
            if (input >= 1 && input <= 8) {
                // Valid input
                break;
            }
            else {
                cout << "Invalid input. Please enter a number between 1 and 8." << endl;
            }
        }
        else {
            cout << "Invalid input. Please enter a valid number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
    return input;
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    Character player(100, 6, false, false, 4, 3, 1, 0, 1, 3, false, false);
    Character enemy(100, 6, false, false, 4, 3, 1, 0, 1, 3, false, true);
    int move;
    bool exit=false;
    string response1;
    int score = 0;

    initialize_Q_table();
    
    cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl
        << endl
        /*<< "⠀⠀⠀⠀⠀⠀⠀⠀ ⠀⠀⠀⢀⣀⣠⣤⣀⠀⠀⠀⠀⠀⠀⠀ " << endl
        << "⠀ ⠀⠀⠀⠀⣀⣀⣀⣠⠔⠊⠑⠒⣷⠆⢸⢳⠀⠀⠀⠀⠀⠀ " << endl
        << " ⠀⠀⢀⠔⠋⠁⠀⠀⠉⠁⠒⠤⣄⠋⠀⠈⢧⡇⠀⠀⣰⣶⢆⠀" << endl
        << " ⠀⠀⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢉⣐⠒⠼⠿⠔⣊⣿⣿⢸⠀" << endl
        << " ⠀⠀⡇⠀⠀⠀⠀⢀⣤⡒⠋⣩⢉⠙⠛⢿⣿⡶⣾⣿⣿⣿⡌⠀" << endl
        << "⠀⠀⠀⠹⡄⠀⠀⢀⣾⣾⠁⢘⣭⣷⣶⠃⣿⣾⣷⣿⣿⣟⠝⠀⠀" << endl
        << " ⠀⠀⠀⠈⠢⣀⣸⢳⠛⡄⠀⠀⠀⠁⠀⣧⡀⢸⡽⠗⠁⠀⠀⠀" << endl
        << "⠀⠀⠀⠀⠀⠀⠀⠈⠳⡄⠁⢠⠀⠀⠋⢱⢿⣷⡿⠁⠀⠀⠀⠀⠀" << endl
        << "⠀ ⠀⠀⠀⠀⠀⠀⠀⡿⠀⢀⠐⠀⢭⣭⣽⣩⡇⠀⠀⠀⠀⠀⠀" << endl
        << "⠀⠀⠀⠀⠀⠀⠀⢀⣴⡇⠑⢬⣀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⠀" << endl
        << "⠀⠀⠀⠀⠀⠀⢀⠎⠹⡕⠠⢀⣈⠻⢿⣿⣿⣷⣄⠀⠀⠀⠀⠀⠀" << endl
        << " ⠀⣀⠤⠒⠒⡇⠀⠀⠈⠢⣔⣭⣙⣛⣿⣽⣇⡏⣣⣀⠀⠀⠀⠀" << endl
        << " ⠉⠀⠀⠀⠀⠘⡄⠀⠀⣠⠴⡟⠍⡻⠟⣿⠘⣷⣇⢧⡉⠉⠉⠁" << endl
        << " ⠀⠀⠀⠀⠀⠀⣇⣠⠾⠃⠀⠓⠤⢔⣄⢣⠃⡇⣿⠚⠉⠀⠀⠀" << endl
        << " ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⠉⢧⠁⡇⠀⠀⠀⠀⠀" << endl
        << " ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠃⠀⠛⠀⠀⠀⠀⠀⠀" << endl*/
        << "        ____ ___  __________         ________                                        " << endl
        << "        |    |   \\ \\______   \      / _____ / _____   _____   ____                 " << endl
        << "        |    |   /  |    |  _/     /   \\  ___\\__   \\ /     \\_/ __ \\             " << endl
        << "        |    |  /   |    |   \\     \\    \\_\\  \\ / __ \\| Y Y  \\  ___/           " << endl
        << "        |______/ /\\ | ______  / /\\   \\______  (____ /__|_|  /\\___  >             " << endl
        << "                 \\/         \\/  \\/          \\/    \\/      \\/     \\/           " << endl
        <<endl
        <<endl
        << "As the sun sets on the unforgiving Froniter, you'll face off against a series of increasingly powerful cyborg adversaries, armed " << endl
        << "with quick-draw skills, nerves of steel, and your trusty six-shooter. Survive the challenges, explore treacherous landscapes, and " << endl
        << "make choices that define your fate in this unforgiving frontier." << endl
        << "It's a battle for survival, redemption, and legendary status, where only the fastest draw and sharpest mind will determine who " << endl
        << "triumphs in the high noon duel. Will you outgun the relentless cyborg hunters or become a legend etched in the annals of this " << endl
        << "new Frontier ? " << endl
        << "The choice is yours, partner." << endl
        << "----------------------------------------------------------------------------------------------------------------------------------" << endl
        << "Your First Duel Awaits. Face them at noon or chichen out? (yes = play game/no = exit game)" << endl
        << "----------------------------------------------------------------------------------------------------------------------------------" << endl << endl;
    cin >> response1;
    for (char& c : response1) {
        c = tolower(c);
    }
    if (response1 == "yes") {
        cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "Come on then Cowboy, Lets Ride." << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl;
        cout << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "The First Cyborg Approaches, challenging you to a duel at the peak of day." << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl;

        exit = true;
    }
    else if (response1 == "no") {
        cout << endl << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "That's alright Cowboy, the Frontier will wait. See you next time." << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl << endl;
        exit= false;
    }

    while (exit) {

        cout
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "Your Stats:" << endl
            << "Player HP: " << player.hp << endl
            << "Player Shots: " << player.shots << endl
            << "Player Reflex Buff (1 is active, 0 is inactive): " << (int)player.shielded << endl
            << "Player Reflex Left: " << (int)(!player.shieldUsed) << endl
            << "Player Cover Timer (In Turns):" << player.CoverT << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "The Enemy Cyborgs Stats (Shown through your trusty scanner):" << endl
            << "Enemy Cyborg HP: " << enemy.hp << endl
            << "Enemy Cyborg Shots: " << enemy.shots << endl
            << "Enemy Cyborg Reflex Buff (1 is active, 0 is inactive): " << (int)enemy.shielded << endl
            << "Enemy Cyborg Reflex Left: " << (int)(!enemy.shieldUsed) << endl
            << "Enemy Cyborg Cover Timer (In Turns):" << enemy.CoverT << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            << "(1) Saber                                                                                         [A Basic attack, does 4 damage.]" << endl
            << "(2) Revolver (" << player.shots << " Shots left)                                                      [A more powerful attack that uses up your Shots.]" << endl
            << "(3) Reload Revolver                                                                                        [Regain up to 6 Shots.]" << endl
            << "(4) Fan the Gun!                            [Fire off 4 consecutive shots! It uses up 4 of your shots and cant be done with less.]" << endl
            << "(5) Healing Pack (" << player.healing << " Packs left)                                [Will heal you for double the amount of damage the Saber would do.]" << endl
            << "(6) Poison Dart (" << player.poison << " Darts left)                                          [Deal 4 damage for 3 turns to your enemy. Bypasses Cover.]" << endl
            << "(7) Lightning Reflexes (" << (int)(player.shieldUsed ? 0 : 1) << " Use left)    [A powerful Manouver. Ignore all incoming damage this turn and do double damage next turn.]" << endl
            << "(8) Take Cover! (" << player.Covered << " Cover left)           [Shield yourself behind cover, take half damage for 3 turns, but you cant use the Saber.]" << endl
            << "----------------------------------------------------------------------------------------------------------------------------------" << endl
            ;
        int move = getValidMove();
        if (player.poisoned != 0) {
            player.hp -= 4;
            player.poisoned -= 1;
            cout << "[You take 4 damage. You are poisoned for " << player.poisoned << " more turn(s).]" << endl;
        }
        if (enemy.poisoned != 0) {
            enemy.hp -= 4;
            enemy.poisoned -= 1;
            cout << "[The Enemy Cyborg takes 4 damage. They are poisoned for " << enemy.poisoned << " more turn(s).]" << endl;
        }
        if ((player.Covered == 0) && (player.CoverT > 0)) {
            player.CoverT -= 1;
        }
        else if ((player.Covered == 0) && (player.CoverT == 0)) {
            player.ActiveCover = 0;
        }
        if ((enemy.Covered != 1) && (enemy.CoverT > 0)) {
            enemy.CoverT -= 1;
        }
        else if ((enemy.Covered == 0) && (enemy.CoverT == 0)) {
            enemy.ActiveCover = 0;
        }

        enemy.do_ai_stuff(player, enemy, move);
        handleMove(move, player, enemy);

        if (enemy.hp <= 0 && player.hp > 0) {
            exit = win(player, enemy, score);
        }
        else if (player.hp <= 0 && enemy.hp > 0) {
            exit = lose(player, enemy, score);
        }
        else if (player.hp == 0 && enemy.hp == 0) {
            exit = lose(player, enemy, score);
        }
    }

    return 0;
}


