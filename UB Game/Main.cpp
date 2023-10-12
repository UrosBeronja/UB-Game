#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <string>

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
const int NUM_STATES = PLAYER_HP_STATES * PLAYER_SHIELD_STATES * PLAYER_REVOLVER_SHOTS_STATES *
PLAYER_SHIELDED_STATES * AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES *
AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES;
const int NUM_ACTIONS = 4;

// Struct to represent the game state
struct GameState {
    int player_hp;
    bool player_shieldUsed;
    int player_revolverShots;
    bool player_shielded;
    int AI_hp;
    int AI_shots;
    bool AI_shieldUsed;
    bool AI_shielded;
    int AI_attackDmg;
    int damage_dealt_to_player;
};

// Q-table to store Q-values
vector<vector<float>> Q_table;

// Initialize the Q-table with zeros
void initialize_Q_table() {
    Q_table.resize(NUM_STATES, vector<float>(NUM_ACTIONS, 0.0f));
}

// Convert a game state to an index for the Q-table
int state_to_index(GameState state) {
    int playerHpIndex = state.player_hp;
    int playerShieldIndex = state.player_shielded ? 1 : 0;
    int playerRevolverIndex = state.player_revolverShots;
    int playerShielIndex = state.player_shieldUsed ? 1 : 0;
    int AIHpIndex = state.AI_hp;
    int AIShotsIndex = state.AI_shots;
    int AIShieldIndex = state.AI_shieldUsed ? 1 : 0;
    int AIShieldedIndex = state.AI_shielded ? 1 : 0;
    int AIAttackDmgIndex = state.AI_attackDmg;

    return playerHpIndex * (PLAYER_SHIELD_STATES * PLAYER_REVOLVER_SHOTS_STATES * PLAYER_SHIELDED_STATES) *
        (AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) +
        playerShieldIndex * (PLAYER_REVOLVER_SHOTS_STATES * PLAYER_SHIELDED_STATES) *
        (AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) +
        playerRevolverIndex * (PLAYER_SHIELDED_STATES) *
        (AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) +
        playerShielIndex * (AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) +
        AIHpIndex * (AI_SHOTS_STATES * AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) +
        AIShotsIndex * (AI_SHIELD_STATES * AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) +
        AIShieldIndex * (AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES) + AIShieldedIndex * AI_ATTACK_DMG_STATES + AIAttackDmgIndex;
}

// Choose an action based on the current state using epsilon-greedy policy
int choose_action(GameState state) {
    int state_index = state_to_index(state);
    float epsilon = 0.1; // Exploration rate

    if (rand() / (RAND_MAX + 1.0) < epsilon) {
        // Randomly explore with probability epsilon
        return rand() % NUM_ACTIONS;
    }
    else {
        // Choose the action with the highest Q-value
        int best_action = distance(Q_table[state_index].begin(), max_element(Q_table[state_index].begin(), Q_table[state_index].end()));
        return best_action;
    }
}

// Update the Q-table based on the reward received
void update_Q_table(GameState state, int action, float reward, GameState next_state) {
    int state_index = state_to_index(state);
    int next_state_index = state_to_index(next_state);

    float learning_rate = 0.5;
    float discount_factor = 0.9;

    float max_next_Q = *max_element(Q_table[next_state_index].begin(), Q_table[next_state_index].end());

    Q_table[state_index][action] += learning_rate * (reward + discount_factor * max_next_Q - Q_table[state_index][action]);
}

// Calculate the reward for a game state
float calculate_reward(GameState state) {
    float reward = 0;

    reward += state.damage_dealt_to_player;

    if (state.player_hp < 1) {
        reward += WIN_REWARD;
    }

    reward += SURVIVAL_REWARD;

    return reward;
}

// Character class for player and AI
class Character {
public:
    string name;
    int hp;
    int shots;
    bool shieldUsed;
    bool shielded;
    int attackDmg;
    bool isAI;

    Character(string n, int h, int s, bool su, bool sh, int ad, bool ai) : name(n), hp(h), shots(s), shieldUsed(su), shielded(sh), attackDmg(ad), isAI(ai) {
        // Initialization code here
    }
};

// Handle the player's move and AI response
void handle_move(int move, Character& player, Character& enemy) {
    int AIMove;
    bool sh = 0;

    if (player.isAI) {
        AIMove = enemy.do_ai_stuff();
        if (move == 3) {
            sh = true;
        }

        // Handle AI's moves based on AIMove
        // ...

    }
    else {
        // Handle the player's moves based on the selected move
        // ...
    }
}

// Game loop
void GameLoop(Character& player, Character& enemy) {
    bool exit = false;

    while (player.hp > 0 && enemy.hp > 0) {
        // Display the game state and available moves to the player
        cout << "Player HP: " << player.hp << endl;
        cout << "Enemy AI HP: " << enemy.hp << endl;
        cout << "Player Shots: " << player.shots << endl;
        cout << "Enemy AI Shots: " << enemy.shots << endl;
        cout << "Player Shielded: " << (player.shielded ? "Yes" : "No") << endl;
        cout << "Enemy AI Shielded: " << (enemy.shielded ? "Yes" : "No") << endl;
        cout << "Player Shield Used: " << (player.shieldUsed ? "Yes" : "No") << endl;
        cout << "Enemy AI Shield Used: " << (enemy.shieldUsed ? "Yes" : "No") << endl;
        cout << "(1) Attack" << endl;
        cout << "(2) Revolver" << endl;
        cout << "(3) Shield" << endl;
        cout << "(4) Fan the Gun!" << endl;

        // Get the player's move
        int move;
        cin >> move;

        if (move < ATTACK || move > FAN) {
            cout << "Invalid move. Please try again." << endl;
            continue;
        }

        // Process the player's move and AI response
        handle_move(move, player, enemy);

        // Check the game outcome
        if (player.hp <= 0 || enemy.hp <= 0) {
            exit = (player.hp <= 0) ? lose(player, enemy) : win(player, enemy);
        }
    }
}

// Handle the game outcome when the player wins
bool win(Character& player, Character& enemy) {
    string response;
    while (true) {
        cout << "You win. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            // Reset the game for a new round
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

// Handle the game outcome when the player loses
bool lose(Character& player, Character& enemy) {
    string response;
    while (true) {
        cout << "You lose. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            // Reset the game for a new round
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
    // Seed the random number generator
    srand(static_cast<unsigned>(time(0)));

    Character player("Cowboy", 50, 6, false, false, 2, false);
    Character enemy("Bandit", 50, 6, false, false, 2, true);

    while (true) {
        GameLoop(player, enemy);
    }

    return 0;
}
