#include <iostream>
#include <string>
#include <vector>
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
const int PLAYER_HP_STATES = 51;
const int PLAYER_SHIELD_STATES = 2;
const int PLAYER_REVOLVER_SHOTS_STATES = 7;
const int PLAYER_SHIELDED_STATES = 2;
const int AI_HP_STATES = 51;
const int AI_SHOTS_STATES = 7;
const int AI_SHIELD_STATES = 2;
const int AI_SHIELDED_STATES = 2;
const int AI_ATTACK_DMG_STATES = 3;

const int NUM_STATES = PLAYER_HP_STATES * PLAYER_SHIELD_STATES * PLAYER_REVOLVER_SHOTS_STATES * PLAYER_SHIELDED_STATES * AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES;
const int NUM_ACTIONS = 4;

struct GameState {
    int player_hp;
    bool player_shieldUsed;
    int player_shots;
    bool player_shielded;
    int ai_hp;
    int ai_shots;
    bool ai_shieldUsed;
    bool ai_shielded;
    int ai_attackDmg;
    int damage_dealt_to_player;
};

// Initialize the Q-table with zeros
vector<vector<vector<float>> > Q_table;

void initialize_Q_table() {
    Q_table.resize(NUM_STATES, vector<vector<float>>(NUM_ACTIONS, vector<float>(NUM_STATES, 0.0)));
}

// Convert a game state to an index for the Q-table
int state_to_index(GameState state) {
    int index = 0;

    index += state.player_hp;
    index *= PLAYER_SHIELD_STATES;
    index += state.player_shieldUsed ? 1 : 0;
    index *= PLAYER_REVOLVER_SHOTS_STATES;
    index += state.player_shots;
    index *= PLAYER_SHIELDED_STATES;
    index += state.player_shielded ? 1 : 0;
    index *= AI_HP_STATES;
    index += state.ai_hp;
    index *= AI_SHOTS_STATES;
    index += state.ai_shots;
    index *= AI_SHIELD_STATES;
    index += state.ai_shieldUsed ? 1 : 0;
    index *= AI_SHIELDED_STATES;
    index += state.ai_shielded ? 1 : 0;
    index *= AI_ATTACK_DMG_STATES;
    index += state.ai_attackDmg;


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

    Q_table[state_index][action] = Q_table[state_index][action] + learning_rate * (reward + discount_factor * max_next_Q - Q_table[state_index][action]);
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

    Character(string n, int hp, int shots, bool shieldUsed, bool shielded, int attackDmg, bool isAI) : name(n), hp(hp), shots(6), shieldUsed(shieldUsed), shielded(shielded), attackDmg(attackDmg), isAI(isAI) {
    }

    int do_ai_stuff(Character& player, Character& enemy, int move) {
        GameState current_state = get_current_state(player, enemy);
        float epsilon = 0.2; // Set your exploration rate (epsilon) as needed
        int action;

        if (rand() / static_cast<float>(RAND_MAX) < epsilon) {
            action = rand() % NUM_ACTIONS; // Explore (choose a random action)
        }
        else {
            action = choose_action(current_state); // Exploit (choose the action with the highest Q-value)
        }

        GameState next_state = execute_action(action, player, enemy);
        float reward = calculate_reward(next_state);
        update_Q_table(current_state, action, reward, next_state);
        update_internal_state(move, player);

        return action;
    }
};

GameState get_current_state(Character player, Character enemy) {
    GameState current_state;
    current_state.player_hp = player.hp;
    current_state.player_shots = player.shots;
    current_state.player_shieldUsed = player.shieldUsed;
    current_state.player_shielded = player.shielded;
    current_state.ai_hp = enemy.hp;
    current_state.ai_shots = enemy.shots;
    current_state.ai_shieldUsed = enemy.shieldUsed;
    current_state.ai_shielded = enemy.shielded;
    current_state.ai_attackDmg = enemy.attackDmg;

    // Additional game-specific state information can be included here

    return current_state;
}

// Function to execute an action and update the game state
GameState execute_action(int action, Character& player, Character& enemy) {
    GameState next_state = get_current_state(player, enemy);

    if (player.isAI) {
        // AI's action based on Q-learning logic
        action = enemy.do_ai_stuff( player, enemy, 0);
    }

    handleMove(action, enemy, player);

    // Update the AI's state within next_state
    next_state.ai_hp = enemy.hp;
    next_state.ai_shots = enemy.shots;
    next_state.ai_shieldUsed = enemy.shieldUsed;
    next_state.ai_shielded = enemy.shielded;
    next_state.ai_attackDmg = enemy.attackDmg;

    return next_state;
}


float calculate_reward(GameState state) {
    float reward = 0;
    reward += state.damage_dealt_to_player;

    if (state.player_hp <= 0) {
        reward -= WASTE_PENALTY;
    }
    else if (state.ai_hp <= 0) {
        reward += WIN_REWARD;
    }

    reward += SURVIVAL_REWARD;

    return reward;
}

void update_internal_state(int action, Character& player) {
    if (action == SHIELD) {
        // Example: Update internal state when using 'SHIELD'
        player.shieldUsed = true;
        player.shielded = true;
        player.attackDmg = 2;  // Increase attack damage when shielded
    }
    // You can add more logic here for other actions
}

void handleMove(int move, Character& player, Character& enemy) {
    int AIMove;
    bool sh = 0;

    if (player.isAI == 1) {
        AIMove = enemy.do_ai_stuff(player, enemy, move);
        if (move == 3) {
            sh = 1;
        }

        if (AIMove == ATTACK) {
            cout << "A.I. slices you with a sword." << endl;
            enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1));
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == FAN) {
            if (player.shots < 1) {
                cout << "A.I. doesn't have enough shots to Fan the Gun." << endl;
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
            cout << "A.I. executes a devastating Shot!" << endl;
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
    }
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
            cout << "You execute a devastating Shot!" << endl;
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
    initialize_Q_table();
    while (player.hp > 0 && enemy.hp > 0) {
        handleMove(move, enemy, player);
        handleMove(move, player, enemy);

        GameState current_state = get_current_state(player, enemy);
        int action = enemy.do_ai_stuff(player, enemy, move);
        GameState next_state = execute_action(action, player, enemy);
        float reward = calculate_reward(next_state);
        update_Q_table(current_state, action, reward, next_state);
        update_internal_state(action, player);

        if (enemy.hp <= 0 && player.hp > 0) {
            exit = win(player, enemy);
        }
        else if (player.hp <= 0 && enemy.hp > 0) {
            exit = lose(player, enemy);
        }
        else if (player.hp == 0 && enemy.hp == 0) {
            exit = lose(player, enemy);
        }
    }
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
    srand(static_cast<unsigned>(time(0)));
    Character player("Cowboy", 50, 6, false, false, 2, 0);
    Character enemy("Bandit", 50, 6, false, false, 2, 1);
    int move;

    initialize_Q_table();
    initialize_Q_table();

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


