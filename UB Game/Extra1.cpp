//#include <iostream>
//#include <vector>
//#include <algorithm>
//#include <ctime>
//#include <cstdlib>
//#include <string>
//
//using namespace std;
//
//// Constants for the different moves
//const int ATTACK = 1;
//const int REVOLVER = 2;
//const int SHIELD = 3;
//const int FAN = 4;
//
//const int WIN_REWARD = 10;
//const int SURVIVAL_REWARD = 1;
//const int WASTE_PENALTY = 3;
//
//// Define the state and action spaces
//const int PLAYER_HP_STATES = 51;
//const int PLAYER_SHIELD_STATES = 2;
//const int PLAYER_REVOLVER_SHOTS_STATES = 7;
//const int PLAYER_SHIELDED_STATES = 2;
//const int AI_HP_STATES = 51;
//const int AI_SHOTS_STATES = 7;
//const int AI_SHIELD_STATES = 2;
//const int AI_SHIELDED_STATES = 2;
//const int AI_ATTACK_DMG_STATES = 3;
//const int NUM_STATES = PLAYER_HP_STATES * PLAYER_SHIELD_STATES * PLAYER_REVOLVER_SHOTS_STATES *
//PLAYER_SHIELDED_STATES * AI_HP_STATES * AI_SHOTS_STATES * AI_SHIELD_STATES *
//AI_SHIELDED_STATES * AI_ATTACK_DMG_STATES;
//const int NUM_ACTIONS = 4;
//
//// Struct to represent the game state
//struct GameState {
//    int player_hp;
//    bool player_shieldUsed;
//    int player_Shots;
//    bool player_shielded;
//    int AI_hp;
//    int AI_shots;
//    bool AI_shieldUsed;
//    bool AI_shielded;
//    int AI_attackDmg;
//    int damage_dealt_to_player;
//};
//
//// Q-table to store Q-values
//vector<vector<float>> Q_table;
//
//// Initialize the Q-table with zeros
//void initialize_Q_table() {
//    Q_table.resize(NUM_STATES, vector<float>(NUM_ACTIONS, 0.0f));
//}
//
//// Convert a game state to an index for the Q-table
//int state_to_index(GameState state) {
//    int index = 0;
//
//    // Linear mapping of state variables to an index
//    index += state.player_hp;
//    index *= PLAYER_SHIELD_STATES;
//    index += state.player_shieldUsed ? 1 : 0;
//    index *= PLAYER_REVOLVER_SHOTS_STATES;
//    index += state.player_Shots;
//    index *= PLAYER_SHIELDED_STATES;
//    index += state.player_shielded ? 1 : 0;
//    index *= AI_HP_STATES;
//    index += state.AI_hp;
//    index *= AI_SHOTS_STATES;
//    index += state.AI_shots;
//    index *= AI_SHIELD_STATES;
//    index += state.AI_shieldUsed ? 1 : 0;
//    index *= AI_SHIELDED_STATES;
//    index += state.AI_shielded ? 1 : 0;
//    index *= AI_ATTACK_DMG_STATES;
//    index += state.AI_attackDmg;
//
//    return index;
//}
//
//// Choose an action based on the current state
//int choose_action(GameState state, int state_index) {
//    // Convert the state to an index
//
//    // Find the action with the highest Q-value for this state
//    int best_action = std::distance(Q_table[state_index].begin(), std::max_element(Q_table[state_index].begin(), Q_table[state_index].end()));
//
//    return best_action;
//}
//
//// Update the Q-table based on the reward received
//void update_Q_table(GameState state, int action, float reward, GameState next_state) {
//    int state_index = state_to_index(state);
//    int next_state_index = state_to_index(next_state);
//
//    float learning_rate = 0.5;
//    float discount_factor = 0.9;
//
//    float max_next_Q = *max_element(Q_table[next_state_index].begin(), Q_table[next_state_index].end());
//
//    Q_table[state_index][action] += learning_rate * (reward + discount_factor * max_next_Q - Q_table[state_index][action]);
//}
//
//// Calculate the reward for a game state
//float calculate_reward(GameState state) {
//    float reward = 0;
//
//    reward += state.damage_dealt_to_player;
//
//    if (state.player_hp < 1) {
//        reward += WIN_REWARD;
//    }
//
//    reward += SURVIVAL_REWARD;
//
//    return reward;
//}
//
//// Character class for player and AI
//class Character {
//public:
//    string name;
//    int hp;
//    int shots;
//    bool shieldUsed;
//    bool shielded;
//    int attackDmg;
//    bool isAI;
//
//    Character(string n, int hp, int shots, bool shieldUsed, bool shielded, int attackDmg, bool isAI) : name(n), hp(hp), shots(6) {
//        // Initialization code here
//    }
//
//    int do_ai_stuff(int move, Character& player, Character& enemy) {
//        GameState current_state = get_current_state(player, enemy);
//        float epsilon = 0.2; // Set your exploration rate (epsilon) as needed
//
//        int action;
//        if (rand() / static_cast<float>(RAND_MAX) < epsilon) {
//            // Explore (choose a random action)
//            action = rand() % NUM_ACTIONS;
//        }
//        else {
//            // Exploit (choose the action with the highest Q-value)
//            int state_index = state_to_index(current_state);
//            action = choose_action(current_state, state_index);
//        }
//
//        // Execute the chosen action and get the resulting state and reward
//        GameState next_state = execute_action(action, player, enemy); // Implement this function
//        float reward = calculate_reward(next_state); // Implement this function
//
//        // Update the Q-table
//        update_Q_table(current_state, action, reward, next_state);
//
//        // Update the AI's internal state
//        update_internal_state(action, player);
//
//        return action;
//    }
//};
//
//GameState get_current_state(Character player, Character enemy) {
//    GameState current_state;
//    current_state.player_hp = player.hp;
//    current_state.player_Shots = player.shots;
//    current_state.player_shieldUsed = player.shieldUsed;
//    current_state.player_shielded = player.shielded;
//    current_state.AI_hp = enemy.hp;
//    current_state.AI_shots = enemy.shots;
//    current_state.AI_shieldUsed = enemy.shieldUsed;
//    current_state.AI_shielded = enemy.shielded;
//    current_state.AI_attackDmg = enemy.attackDmg;
//
//    // Include any additional game-specific state information
//
//    return current_state;
//}
//
//// Handle the player's move and AI response
//void handle_move(int move, Character& player, Character& enemy) {
//    int AIMove;
//    bool sh = 0;
//
//    if (player.isAI == true) {
//        //AI generates its moves
//        if (move == 3) {
//            sh = 1;
//        }
//        AIMove = enemy.do_ai_stuff(move, player, enemy);
//        //AI MOVES
//
//        if (AIMove == ATTACK) {
//            cout << "A.I. slices you with a sword." << endl;
//            enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1));
//            player.attackDmg = 1;
//            player.shielded = false;
//        }
//        else if (AIMove == FAN) {
//            if (player.shots < 1) {
//                cout << "A.I. doesnt have enough shots to Fan the Gun." << endl;
//                return;
//            }
//            cout << "A.I. Fans the Gun!" << endl;
//            while (player.shots != 0) {
//                enemy.hp -= 1.5 * (player.attackDmg * (sh ? 0 : 1));
//                player.shots -= 1;
//                cout << "A Shot hits you!" << endl;
//            }
//            player.attackDmg = 1;
//            player.shielded = false;
//        }
//        else if (AIMove == REVOLVER) {
//            if (player.shots < 1) {
//                cout << "A.I. doesn't have enough shots to use the Revolver." << endl;
//                return;
//            }
//            cout << "A.I. executes a devestating Shot!" << endl;
//            enemy.hp -= 1.5 * (player.attackDmg * (sh ? 0 : 1));
//            player.shots -= 1;
//            player.attackDmg = 1;
//            player.shielded = false;
//        }
//
//        else if (AIMove == SHIELD) {
//            if (player.shieldUsed) {
//                cout << "You have already used your shield." << endl;
//                return;
//            }
//            cout << "You raised your shield." << endl;
//            player.shieldUsed = true;
//            player.shielded = true;
//            player.attackDmg = 2;
//        }
//    }// Players Moves
//    else {
//        if (move == ATTACK) {
//            cout << "You sliced the A.I. with your sword." << endl;
//            enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1));
//            player.attackDmg = 1;
//            player.shielded = false;
//        }
//        else if (move == FAN) {
//            if (player.shots < 1) {
//                cout << "You don't have enough shots to Fan the Gun." << endl;
//                return;
//            }
//            cout << "You Fan the Gun!" << endl;
//            while (enemy.shots != 0) {
//                player.hp -= 1.5 * (enemy.attackDmg * (sh ? 0 : 1));
//                enemy.shots -= 1;
//                cout << "A Shot hits the A.I.!" << endl;
//            }
//            enemy.attackDmg = 1;
//            enemy.shielded = false;
//        }
//        else if (move == REVOLVER) {
//            if (player.shots < 1) {
//                cout << "You don't have enough shots to use the Revolver." << endl;
//                return;
//            }
//            cout << "You execute a devestating Shot!" << endl;
//            enemy.hp -= 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1));
//            player.shots -= 1;
//            player.attackDmg = 1;
//            player.shielded = false;
//        }
//
//        else if (move == SHIELD) {
//            if (player.shieldUsed) {
//                cout << "You have already used your shield." << endl;
//                return;
//            }
//            cout << "You raised your shield." << endl;
//            player.shieldUsed = true;
//            player.shielded = true;
//            player.attackDmg = 2;
//        }
//    }
//}
//
//
//// Game loop
//void GameLoop(Character& player, Character& enemy) {
//    bool exit = false;
//
//    while (player.hp > 0 && enemy.hp > 0) {
//        // Display the game state and available moves to the player
//        cout << "Player HP: " << player.hp << endl;
//        cout << "Enemy AI HP: " << enemy.hp << endl;
//        cout << "Player Shots: " << player.shots << endl;
//        cout << "Enemy AI Shots: " << enemy.shots << endl;
//        cout << "Player Shielded: " << (player.shielded ? "Yes" : "No") << endl;
//        cout << "Enemy AI Shielded: " << (enemy.shielded ? "Yes" : "No") << endl;
//        cout << "Player Shield Used: " << (player.shieldUsed ? "Yes" : "No") << endl;
//        cout << "Enemy AI Shield Used: " << (enemy.shieldUsed ? "Yes" : "No") << endl;
//        cout << "(1) Attack" << endl;
//        cout << "(2) Revolver" << endl;
//        cout << "(3) Shield" << endl;
//        cout << "(4) Fan the Gun!" << endl;
//
//        // Get the player's move
//        int move;
//        cin >> move;
//
//        if (move < ATTACK || move > FAN) {
//            cout << "Invalid move. Please try again." << endl;
//            continue;
//        }
//
//        // Process the player's move and AI response
//        handle_move(move, player, enemy);
//        handle_move(move, enemy, player);
//
//        // Check the game outcome
//        if (player.hp <= 0 || enemy.hp <= 0) {
//            exit = (player.hp <= 0) ? lose(player, enemy) : win(player, enemy);
//        }
//    }
//}
//
//// Handle the game outcome when the player wins
//bool win(Character& player, Character& enemy) {
//    string response;
//    while (true) {
//        cout << "You win. Would you like to play again? (yes/no)" << endl;
//        cin >> response;
//        for (char& c : response) {
//            c = tolower(c);
//        }
//        if (response == "yes" || response == "y") {
//            // Reset the game for a new round
//            player.hp = 50;
//            enemy.hp = 50;
//            player.shots = 6;
//            enemy.shots = 6;
//            player.shieldUsed = false;
//            enemy.shieldUsed = false;
//            player.shielded = false;
//            enemy.shielded = false;
//            player.attackDmg = 1;
//            enemy.attackDmg = 1;
//            return true;
//        }
//        else if (response == "no" || response == "n") {
//            cout << "Good luck next time!" << endl;
//
//            return false;
//        }
//        else {
//            cout << "Invalid response. Please enter 'yes' or 'no'." << endl;
//        }
//    }
//}
//
//// Handle the game outcome when the player loses
//bool lose(Character& player, Character& enemy) {
//    string response;
//    while (true) {
//        cout << "You lose. Would you like to play again? (yes/no)" << endl;
//        cin >> response;
//        for (char& c : response) {
//            c = tolower(c);
//        }
//        if (response == "yes" || response == "y") {
//            // Reset the game for a new round
//            player.hp = 50;
//            enemy.hp = 50;
//            player.shots = 6;
//            enemy.shots = 6;
//            player.shieldUsed = false;
//            enemy.shieldUsed = false;
//            player.shielded = false;
//            enemy.shielded = false;
//            player.attackDmg = 1;
//            enemy.attackDmg = 1;
//            return true;
//        }
//        else if (response == "no" || response == "n") {
//            cout << "Good luck next time!" << endl;
//            return false;
//        }
//        else {
//            cout << "Invalid response. Please enter 'yes' or 'no'." << endl;
//        }
//    }
//}
//
//int main() {
//    // Seed the random number generator
//    srand(static_cast<unsigned>(time(0)));
//
//    Character player("Cowboy", 50, 6, false, false, 2, false);
//    Character enemy("Bandit", 50, 6, false, false, 2, true);
//
//    while (true) {
//        GameLoop(player, enemy);
//    }
//
//    return 0;
//}
