#include <iostream>
#include <string.h>
#include <boost/math_fwd.hpp>
#include "openai.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

// Constants for the different moves

const int ATTACK = 1;
const int REVOLVER = 2;
const int SHIELD = 3;
const int FAN = 4;

const int WIN_REWARD = 10;
const int SURVIVAL_REWARD = 1;
const int WASTE_PENALTY = 3;


// Define the state and action spaces
const int PLAYER_HP_STATES = 51; // Player's HP can be from 0 to 50
const int PLAYER_SHIELD_STATES = 2; // Player's shield can be used or not used
const int PLAYER_REVOLVER_SHOTS_STATES = 7; // Player's revolver shots can be from 0 to 6
const int PLAYER_SHIELDED_STATES = 2; // Player can be shielded or not shielded
const int AI_HP_STATES = 51; // AI's HP can be from 0 to 50
const int AI_SHOTS_STATES = 7; // AI's shots can be from 0 to 6
const int AI_SHIELD_STATES = 2; // AI's shield can be used or not used
const int AI_SHIELDED_STATES = 2; // AI can be shielded or not shielded
const int AI_ATTACK_DMG_STATES = 3; // AI's attack damage can be from 1 to 3

const int NUM_STATES = PLAYER_HP_STATES * PLAYER_SHIELD_STATES * PLAYER_REVOLVER_SHOTS_STATES * PLAYER_SHIELDED_STATES * AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES;
const int NUM_ACTIONS = 4; // ATTACK, REVOLVER, SHIELD, FAN




void initialize_Q_table() {
    Q_table.resize(NUM_STATES, std::vector<float>(NUM_ACTIONS, 0));
}

// Choose an action based on the current state
int choose_action(GameState state) {
    // Convert the state to an index
    int state_index = state_to_index(state);

    // Find the action with the highest Q-value for this state
    int best_action = std::distance(Q_table[state_index].begin(), std::max_element(Q_table[state_index].begin(), Q_table[state_index].end()));

    return best_action;
}

// Update the Q-table based on the reward received
void update_Q_table(GameState state, int action, float reward, GameState next_state) {
    // Convert the states to indices
    int state_index = state_to_index(state);
    int next_state_index = state_to_index(next_state);

    // Find the maximum Q-value for the next state
    float max_next_Q = *std::max_element(Q_table[next_state_index].begin(), Q_table[next_state_index].end());

    // Update the Q-value for the current state and action
    float learning_rate = 0.5;
    float discount_factor = 0.9;
    Q_table[state_index][action] = Q_table[state_index][action] + learning_rate * (reward + discount_factor * max_next_Q - Q_table[state_index][action]);
}
// Convert a game state to an index for the Q-table
int state_to_index(GameState state) {
    // This function needs to be implemented based on your specific game state
    // It should convert the game state to a unique index between 0 and NUM_STATES - 1
}


class Character {
public:
    string name;
    int hp;
    int shots;
    bool shieldUsed;
    bool shielded;
    int attackDmg;
    bool isAI;

    // Add properties for other stats and abilities

    Character(string n, int hp, int shots, bool shieldUsed, bool shielded, int attackDmg, bool isAI) : name(n), hp(hp), shots(6) {
        // Initialization code here
    }

    int do_ai_stuff() {
        GameState current_state = get_current_state();

        int action;
        if (rand() / static_cast<float>(RAND_MAX) < epsilon) {
            // Explore (choose a random action)
            action = rand() % NUM_ACTIONS;
        }
        else {
            // Exploit (choose the action with the highest Q-value)
            int state_index = state_to_index(current_state);
            action = choose_action(current_state, state_index);
        }

        // Execute the chosen action and get the resulting state and reward
        GameState next_state = execute_action(action);  // Implement this function
        float reward = calculate_reward(next_state);    // Implement this function

        // Update the Q-table
        update_Q_table(current_state, action, reward, next_state);

        // Update the AI's internal state
        update_internal_state(action);

        return action;
    }

};

struct GameState {
    int player_hp;
    bool player_shieldUsed;
    int player_Shots;
    bool player_shielded;
    int AI_hp;
    int AI_shots;
    bool AI_shieldUsed;
    bool AI_shielded;
    int AI_attackDmg;
    int damage_dealt_to_player;
};


GameState get_current_state(Character player, Character enemy) {
    GameState current_state;
    current_state.player_hp = player.hp;
    current_state.player_Shots = player.shots;
    current_state.player_shieldUsed = player.shieldUsed;
    current_state.player_shielded = player.shielded;
    current_state.AI_hp = enemy.hp;
    current_state.AI_shots = enemy.shots;
    current_state.AI_shieldUsed = enemy.shieldUsed;
    current_state.AI_shielded = enemy.shielded;
    current_state.AI_attackDmg = enemy.attackDmg;

    // Include any additional game-specific state information

    return current_state;
}


float calculate_reward(GameState state) {
    float reward = 0;

    // Reward for damage dealt to player
    reward += state.damage_dealt_to_player;

    // Extra reward for winning
    if (state.player_hp < 1) {
        reward += WIN_REWARD;
    }

    // Reward for survival
    reward += SURVIVAL_REWARD;

    return reward;
}

std::vector<std::vector<float>> Q_table;

// Initialize the Q-table with zeros
void initialize_Q_table(int num_states, int num_actions) {
    Q_table.resize(num_states, std::vector<float>(num_actions, 0));
}

// Choose an action based on the current state
int choose_action(GameState state) {
    // Convert the state to an index
    int state_index = state_to_index(state);

    // Find the action with the highest Q-value for this state
    int best_action = std::distance(Q_table[state_index].begin(), std::max_element(Q_table[state_index].begin(), Q_table[state_index].end()));

    return best_action;
}

// Update the Q-table based on the reward received
void update_Q_table(GameState state, int action, float reward, GameState next_state) {
    // Convert the states to indices
    int state_index = state_to_index(state);
    int next_state_index = state_to_index(next_state);

    // Find the maximum Q-value for the next state
    float max_next_Q = *std::max_element(Q_table[next_state_index].begin(), Q_table[next_state_index].end());

    // Update the Q-value for the current state and action
    float learning_rate = 0.5;
    float discount_factor = 0.9;
    Q_table[state_index][action] = Q_table[state_index][action] + learning_rate * (reward + discount_factor * max_next_Q - Q_table[state_index][action]);
}

// Convert a game state to an index for the Q-table
int state_to_index(GameState state) {
    // This function needs to be implemented based on your specific game state
    // It should convert the game state to a unique index between 0 and num_states - 1
}

// Calculate the reward for a game state
float calculate_reward(GameState state) {
    // This function needs to be implemented based on your specific game
    // It should calculate the reward for the AI based on the current game state
}


void handleMove(int move, Character& player, Character& enemy) {
    int AIMove;
    bool sh = 0;
    if (player.isAI == 1) {
        //AI generates its moves
        AIMove = enemy.do_ai_stuff();
        if (move == 3) {
            sh = 1;
        }
        //AI MOVES

        if (AIMove == ATTACK) {
            cout << "A.I. slices you with a sword." << endl;
            enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1));
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == FAN) {
            if (player.shots < 1) {
                cout << "A.I. doesnt have enough shots to Fan the Gun." << endl;
                return;
            }
            cout << "A.I. Fans the Gun!" << endl;
            while (player.shots != 0) {
                enemy.hp -= 1.5 * (player.attackDmg * (sh ? 0 : 1));
                player.shots -= 1;
                cout << "A Shot hits you!" << endl;
            }
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == REVOLVER) {
            if (player.shots < 1) {
                cout << "A.I. doesn't have enough shots to use the Revolver." << endl;
                return;
            }
            cout << "A.I. executes a devestating Shot!" << endl;
            enemy.hp -= 1.5 * (player.attackDmg * (sh ? 0 : 1));
            player.shots -= 1;
            player.attackDmg = 1;
            player.shielded = false;
        }

        else if (AIMove == SHIELD) {
            if (player.shieldUsed) {
                cout << "You have already used your shield." << endl;
                return;
            }
            cout << "You raised your shield." << endl;
            player.shieldUsed = true;
            player.shielded = true;
            player.attackDmg = 2;
        }
    }// Players Moves
    else {
        if (move == ATTACK) {
            cout << "You sliced the A.I. with your sword." << endl;
            enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1));
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == FAN) {
            if (player.shots < 1) {
                cout << "You don't have enough shots to Fan the Gun." << endl;
                return;
            }
            cout << "You Fan the Gun!" << endl;
            while (enemy.shots != 0) {
                player.hp -= 1.5 * (enemy.attackDmg * (sh ? 0 : 1));
                enemy.shots -= 1;
                cout << "A Shot hits the A.I.!" << endl;
            }
            enemy.attackDmg = 1;
            enemy.shielded = false;
        }
        else if (move == REVOLVER) {
            if (player.shots < 1) {
                cout << "You don't have enough shots to use the Revolver." << endl;
                return;
            }
            cout << "You execute a devestating Shot!" << endl;
            enemy.hp -= 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1));
            player.shots -= 1;
            player.attackDmg = 1;
            player.shielded = false;
        }

        else if (move == SHIELD) {
            if (player.shieldUsed) {
                cout << "You have already used your shield." << endl;
                return;
            }
            cout << "You raised your shield." << endl;
            player.shieldUsed = true;
            player.shielded = true;
            player.attackDmg = 2;
        }
    }
}

void GameLoop(int move, Character player, Character enemy) {
    bool exit;
    // Initialize characters, environment, and other game elements

    // Main game loop
    while (player.hp > 0 && enemy.hp > 0) {
        //processing the AI turn first, but giving it the players chosen move to apply the shied and dodge correctly
        handleMove(move, enemy, player);
        handleMove(move, player, enemy);

        if (enemy.hp < 0 && player.hp>0) {
            exit = win(player, enemy);
        }
        else if (player.hp < 0 && enemy.hp>0) {
            exit = lose(player, enemy);
        }
        else if (player.hp == 0 && enemy.hp == 0) {
            exit = lose(player, enemy);
        }
    }

    // Display game outcome and handle game over logic
}

bool win(Character& player, Character& enemy) {
    string response;
    while (true) {
        cout << "You win. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            player.hp = 50;
            enemy.hp = 50;
            player.shots = 6;
            enemy.shots = 6;
            player.shieldUsed = false;
            enemy.shieldUsed = false;
            player.shielded = false;
            enemy.shielded = false;
            player.attackDmg = 1;
            enemy.attackDmg = 1;

            return true;
        }
        else if (response == "no") {
            return false;
        }
        else {
            cout << "Invalid response. Please enter 'yes' or 'no'." << endl;
        }
    }
}

bool lose(Character& player, Character& enemy) {
    string response;
    while (true) {
        cout << "You lose. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            player.hp = 50;
            enemy.hp = 50;
            player.shots = 6;
            enemy.shots = 6;
            player.shieldUsed = false;
            enemy.shieldUsed = false;
            player.shielded = false;
            enemy.shielded = false;
            player.attackDmg = 1;
            enemy.attackDmg = 1;
            return true;
        }
        else if (response == "no") {
            return false;
        }
        else {
            cout << "Invalid response. Please enter 'yes' or 'no'." << endl;
        }
    }
}

int main() {
    // Main program here
    // Seed the random number generator


    srand(static_cast<unsigned>(time(0)));
    Character player{ "Cowboy", 50, 6, false, false, 2, 0 };
    Character enemy{ "Bandit", 50, 6, false, false, 2, 1 };
    int move;
    while (true) {
        while (true) {
            cout
                << "Player HP: " << player.hp << endl
                << "Enemy A.I. HP: " << enemy.hp << endl
                << "Player Shots: " << player.shots << endl
                << "Enemy A.I. Shots: " << enemy.shots << endl
                << "Player Shielded: " << player.shielded << endl
                << "Enemy A.I. Shielded: " << enemy.shielded << endl
                << "Player Shield Used: " << player.shieldUsed << endl
                << "Enemy A.I. Shield Used: " << enemy.shieldUsed << endl
                << "(1) Attack" << endl
                << "(2) Magic Missile" << endl
                << "(3) Shield" << endl
                << "(4) Fan the Gun!" << endl;
            cin >> move;
            if (move < ATTACK || move > FAN) {
                cout << "Invalid move. Please try again." << endl;
                continue;
            }
            GameLoop(move, player, enemy);
            if (player.hp <= 0 || enemy.hp <= 0) {
                break;
            }
        }
        return 0;
    }
}

