#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

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
const int FRENZY = 9;

const int WIN_REWARD = 10;
const int SURVIVAL_REWARD = 1;
const int WASTE_PENALTY = 3;


struct GameState;
class Character;
GameState get_current_state(Character player, Character enemy);
GameState execute_action(int action, Character& player, Character& enemy);
float calculate_reward(GameState state);
void update_internal_state(int action, Character& player);
void handleMove(int move, Character& player, Character& enemy);
bool win(Character& player, Character& enemy);
bool lose(Character& player, Character& enemy);
void reset_player(Character& player);


// Define the state and action spaces
const int PLAYER_HP_STATES = 101;
const int PLAYER_SHIELD_STATES = 2;
const int PLAYER_REVOLVER_SHOTS_STATES = 7;
const int PLAYER_SHIELDED_STATES = 2;
const int PLAYER_HEALING_STATUS = 4;
const int PLAYER_POISON_STATUS = 2;
const int PLAYER_POISONED_STATUS = 4;
const int PLAYER_COVER_STATUS = 2;
const int PLAYER_COVERTIME_STATUS = 4;
const int PLAYER_FRENZY_STATUS = 2;
const int PLAYER_FRENZYTIME_STATUS = 4;

const int AI_HP_STATES = 101;
const int AI_SHOTS_STATES = 7;
const int AI_SHIELD_STATES = 2;
const int AI_SHIELDED_STATES = 2;
const int AI_HEALING_STATUS = 4;
const int AI_POISON_STATUS = 2;
const int AI_POISONED_STATUS = 4;
const int AI_COVER_STATUS = 2;
const int AI_COVERTIME_STATUS = 4;
const int AI_FRENZY_STATUS = 2;
const int AI_FRENZYTIME_STATUS = 4;

const int AI_ATTACK_DMG_STATES = 3;

const int NUM_STATES = PLAYER_HP_STATES *PLAYER_SHIELD_STATES *PLAYER_REVOLVER_SHOTS_STATES *PLAYER_SHIELDED_STATES *PLAYER_HEALING_STATUS 
*PLAYER_POISON_STATUS *PLAYER_POISONED_STATUS *PLAYER_COVER_STATUS *PLAYER_COVERTIME_STATUS *PLAYER_FRENZY_STATUS *PLAYER_FRENZYTIME_STATUS 
*AI_HP_STATES *AI_SHOTS_STATES *AI_SHIELD_STATES *AI_SHIELDED_STATES *AI_ATTACK_DMG_STATES *AI_HEALING_STATUS *AI_POISON_STATUS 
*AI_POISONED_STATUS *AI_COVER_STATUS *AI_COVERTIME_STATUS *AI_FRENZY_STATUS *AI_FRENZYTIME_STATUS;// 1.6436706e+15, 1,6 quadrilion, 14 quadrilion options
const int NUM_ACTIONS = 9;


// Initialize the Q-table with zeros
vector<vector<float>> Q_table;

void initialize_Q_table() {
    Q_table.resize(NUM_STATES, vector<float>(NUM_ACTIONS, 0.0));
}

struct GameState {
    int player_hp;
    int player_shots;
    bool player_shieldUsed;
    bool player_shielded;
    int player_healing;
    int player_poison;
    int player_poisoned;
    int player_Cover;
    int player_CoverT;
    int player_Frenzy;
    int player_FrenzyT;

    int ai_hp;
    int ai_shots;
    bool ai_shieldUsed;
    bool ai_shielded;
    int ai_attackDmg;
    int ai_healing;
    int ai_poison;
    int ai_poisoned;
    int ai_Cover;
    int ai_CoverT;
    int ai_Frenzy;
    int ai_FrenzyT;

    int damage_dealt_to_player;
};



// Convert a game state to an index for the Q-table
int state_to_index(GameState state) {
    int index = 0;

    index *= PLAYER_HP_STATES;
    index += state.player_hp;
    index *= PLAYER_SHIELD_STATES;
    index += state.player_shieldUsed ? 1 : 0;
    index *= PLAYER_REVOLVER_SHOTS_STATES;
    index += state.player_shots;
    index *= PLAYER_SHIELDED_STATES;
    index += state.player_shielded ? 1 : 0;
    index *= PLAYER_HEALING_STATUS;
    index *= state.player_healing;
    index *= PLAYER_POISON_STATUS;
    index *= state.player_poison;
    index *= PLAYER_POISONED_STATUS;
    index *= state.player_poisoned;
    index *= PLAYER_COVER_STATUS;
    index *= state.player_Cover;
    index *= PLAYER_COVERTIME_STATUS;
    index *= state.player_CoverT;
    index *= PLAYER_FRENZY_STATUS;
    index *= state.player_Frenzy;
    index *= PLAYER_FRENZYTIME_STATUS;
    index *= state.player_FrenzyT;

    index *= AI_HP_STATES;
    index += state.ai_hp;
    index *= AI_SHOTS_STATES;
    index += state.ai_shots;
    index *= AI_SHIELD_STATES;
    index += state.ai_shieldUsed ? 1 : 0;
    index *= AI_SHIELDED_STATES;
    index += state.ai_shielded ? 1 : 0;
    index *= AI_HEALING_STATUS;
    index *= state.ai_healing;
    index *= AI_POISON_STATUS;
    index *= state.ai_poison;
    index *= AI_POISONED_STATUS;
    index *= state.ai_poisoned;
    index *= AI_COVER_STATUS;
    index *= state.ai_Cover;
    index *= AI_COVERTIME_STATUS;
    index *= state.ai_CoverT;
    index *= AI_FRENZY_STATUS;
    index *= state.ai_Frenzy;
    index *= AI_FRENZYTIME_STATUS;
    index *= state.ai_FrenzyT;

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

    Q_table[state_index][action] = Q_table[state_index][action] + learning_rate * (reward + discount_factor * max_next_Q 
        - Q_table[state_index][action]);
}

class Character {
public:
    string name;
    int hp;
    int shots;
    bool shieldUsed;
    bool shielded;
    int attackDmg;
    int healing;
    int poison;
    int poisoned;
    int Frenzied;
    int FrenzT;
    int Covered;
    int CoverT;
    bool ActiveCover;
    bool ActiveFrenzy;
    bool isAI;

    Character(string name, int hp, int shots, bool shieldUsed, bool shielded, int attackDmg, int healing, int poison, int poisoned, int Frenzied, 
        int FrenzT, int Covered, int CoverT, bool ActiveCover, bool ActiveFrenzy, bool isAI) : name(name), hp(hp), shots(shots), shieldUsed(shieldUsed), shielded(shielded), 
        attackDmg(attackDmg), isAI(isAI) {
    }

    int do_ai_stuff(Character& player, Character& enemy, int move, int AIMove) {
        GameState current_state = get_current_state(player, enemy);
        float epsilon = 0.2; // Set your exploration rate (epsilon) as needed

        if (rand() / static_cast<float>(RAND_MAX) < epsilon) {
            AIMove = rand() % NUM_ACTIONS; // Explore (choose a random action)
        }
        else {
            AIMove = choose_action(current_state); // Exploit (choose the action with the highest Q-value)
        }

        GameState next_state = execute_action(AIMove, player, enemy);
        float reward = calculate_reward(next_state);
        update_Q_table(current_state, AIMove, reward, next_state);
        update_internal_state(move, player);

        return AIMove;
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
        action = enemy.do_ai_stuff(player, enemy, 0, 0);
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

void handleMove(int move, Character& player, Character& enemy, int AIMove) {
    bool sh = 0;
    bool cv = 0;

    if (player.isAI == 1) {
        
        if (move == SHIELD) {
            sh = 1;
        }if (move == COVER) {
            cv = 1;
        }
        
        ////AI MOVES
        if (AIMove == ATTACK) {
            if ((player.Covered < 1) && (player.CoverT > 0)) {
                cout << "You cant use your Saber while in Cover." << endl;
                return;
            }
            cout << "You sliced the Cyborg with your sword. You deal " << 1 * (player.attackDmg * (sh ? 0 : 1) *
                (enemy.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1) * (enemy.ActiveFrenzy ? 0.5 : 1)) << " damage." << endl;
            enemy.hp -= 1 * (player.attackDmg * (sh ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1)
                * (enemy.ActiveFrenzy ? 0.5 : 1));
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == FAN) {
            int i = 4;
            if (player.shots < i) {
                cout << "You don't have enough shots to Fan the Gun." << endl;
                return;
            }

            cout << "You Fan the Gun!" << endl;
            while (i > 0) {
                player.hp -= 1.5 * (enemy.attackDmg * (sh ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) * (cv ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1)
                    * (enemy.ActiveFrenzy ? 0.5 : 1));
                enemy.shots -= 1;
                cout << "A Shot hits the Cyborg for " << 1.5 * (enemy.attackDmg * (sh ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) * (cv ? 0.5 : 1) *
                    (player.ActiveFrenzy ? 2 : 1) * (enemy.ActiveFrenzy ? 0.5 : 1)) << " damage!" << endl;
            }
            enemy.attackDmg = 1;
            enemy.shielded = false;
        }
        else if (AIMove == RELOAD) {
            cout << "You Reload your Revolver." << endl;
            player.shots = 6;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == REVOLVER) {
            if (player.shots < 1) {
                cout << "You don't have enough shots to use the Revolver." << endl;
                return;
            }
            if ((player.Frenzied < 1) && (player.FrenzT > 0)) {
                cout << "You cant Shoot while you are Frenzied." << endl;
                return;
            }
            cout << "You execute a devastating Shot! You deal " << 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) 
                * (cv ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1) * (enemy.ActiveFrenzy ? 0.5 : 1))
                << "damage to the Cyborg." << endl;
            enemy.hp -= 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) * (cv ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1)
                * (enemy.ActiveFrenzy ? 0.5 : 1));
            player.shots -= 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == HEAL) {
            if (player.healing < 1) {
                cout << "You don't have any Healing Packs left." << endl;
                return;
            }
            cout << "You Heal your wounds! You heal " << 2 * player.attackDmg << " wounds." << endl;
            player.hp += 2 * player.attackDmg;
            if (player.hp > 100) {
                player.hp = 100;
            }
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == POISON) {
            if (player.poison < 1) {
                cout << "You don't have any more Poisons." << endl;
                return;
            }
            cout << "You manage to Poison your Enemy! He will be poisoned for 3 Turns. This damage bypasses Cover and Frenzy." << endl;
            enemy.poisoned = 3 * (enemy.shielded ? 0 : 1);
            player.poison -= 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == SHIELD) {
            if (player.shieldUsed) {
                cout << "You have already used your Lightning Reflexes." << endl;
                return;
            }
            cout << "You use the burst of speed to evade any attack and set up for a devestating counter attack or heal!" << endl;
            player.shieldUsed = true;
            player.shielded = true;
            player.attackDmg = 2;
        }
        else if (AIMove == COVER) {
            if (player.Covered < 1) {
                cout << "You have already used your Cover." << endl;
                return;
            }
            if ((player.Frenzied < 1) && (player.FrenzT > 0)) {
                cout << "You cant jump into Cover while you are Frenzied." << endl;
                return;
            }
            cout << "You expertly jump into Cover! You will take half damage for 3 turns, but you wont be able to use your Saber or Frenzy." << endl;
            player.Covered -= 1;
            player.ActiveCover = 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (AIMove == FRENZY) {
            if (player.Frenzied < 1) {
                cout << "You have already used up your Frenzy." << endl;
                return;
            }
            if ((player.Covered < 1) && (player.CoverT > 0)) {
                cout << "You cant use your Frenzy while in Cover." << endl;
                return;
            }
            cout << "You enter a Battle Frenzy! You will take less damage, heal more and do double damage, but you wont be able to use your "
                << "Revolver or Fan the Gun." << endl;
            player.Frenzied -= 1;
            player.ActiveFrenzy = 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
    } ////PLAYER MOVES
    else {
        if (move == ATTACK) {
            if ((player.Covered < 1) && (player.CoverT>0)) {
                cout << "You cant use your Saber while in Cover." << endl;
                return;
            }
            cout << "You sliced the Cyborg with your sword. You deal "<< 1 * (player.attackDmg * (enemy.shielded ? 0 : 1) * 
                (enemy.ActiveCover ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1) * (enemy.ActiveFrenzy ? 0.5 : 1)) << " damage." << endl;
            enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1) 
                * (enemy.ActiveFrenzy ? 0.5 : 1));
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == FAN) {
            int i=4;
            if (player.shots < i) {
                cout << "You don't have enough shots to Fan the Gun." << endl;
                return;
            }

            cout << "You Fan the Gun!" << endl;
            while (i>0) {
                player.hp -= 1.5 * (enemy.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1)
                    * (enemy.ActiveFrenzy ? 0.5 : 1));
                enemy.shots -= 1;
                cout << "A Shot hits the Cyborg for "<< 1.5 * (enemy.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) * 
                    (player.ActiveFrenzy ? 2 : 1) * (enemy.ActiveFrenzy ? 0.5 : 1)) << " damage!" << endl;
            }
            enemy.attackDmg = 1;
            enemy.shielded = false;
        }
        else if (move == RELOAD) {
            cout << "You Reload your Revolver." << endl;
            player.shots = 6;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == REVOLVER) {
            if (player.shots < 1) {
                cout << "You don't have enough shots to use the Revolver." << endl;
                return;
            }
            if ((player.Frenzied < 1) && (player.FrenzT > 0)) {
                cout << "You cant Shoot while you are Frenzied." << endl;
                return;
            }
            cout << "You execute a devastating Shot! You deal " << 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) 
                * (player.ActiveFrenzy ? 2 : 1) * (enemy.ActiveFrenzy ? 0.5 : 1))
                << "damage to the Cyborg." << endl;
            enemy.hp -= 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.CoverT ? 0.5 : 1) * (player.ActiveFrenzy ? 2 : 1)
                * (enemy.ActiveFrenzy ? 0.5 : 1));
            player.shots -= 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == HEAL) {
            if (player.healing < 1) {
                cout << "You don't have any Healing Packs left." << endl;
                return;
            }
            cout << "You Heal your wounds! You heal "<< 2 * player.attackDmg << " wounds." << endl;
            player.hp += 2*player.attackDmg;
            if (player.hp > 100) {
                player.hp = 100;
            }
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == POISON) {
            if (player.poison < 1) {
                cout << "You don't have any more Poisons." << endl;
                return;
            }
            cout << "You manage to Poison your Enemy! He will be poisoned for 3 Turns. This damage bypasses Cover and Frenzy." << endl;
            enemy.poisoned = 3 * (enemy.shielded ? 0 : 1);
            player.poison -= 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == SHIELD) {
            if (player.shieldUsed) {
                cout << "You have already used your Lightning Reflexes." << endl;
                return;
            }
            cout << "You use the burst of speed to evade any attack and set up for a devestating counter attack or heal!" << endl;
            player.shieldUsed = true;
            player.shielded = true;
            player.attackDmg = 2;
        }
        else if (move == COVER) {
            if (player.Covered < 1) {
                cout << "You have already used your Cover." << endl;
                return;
            }
            if ((player.Frenzied < 1) && (player.FrenzT > 0)) {
                cout << "You cant jump into Cover while you are Frenzied." << endl;
                return;
            }
            cout << "You expertly jump into Cover! You will take half damage for 3 turns, but you wont be able to use your Saber or Frenzy." << endl;
            player.Covered -= 1;
            player.ActiveCover = 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
        else if (move == FRENZY) {
            if (player.Frenzied < 1) {
                cout << "You have already used up your Frenzy." << endl;
                return;
            }
            if ((player.Covered < 1) && (player.CoverT > 0)) {
                cout << "You cant use your Frenzy while in Cover." << endl;
                return;
            }
            cout << "You enter a Battle Frenzy! You will take less damage, heal more and do double damage, but you wont be able to use your "
                << "Revolver or Fan the Gun." << endl;
            player.Frenzied -= 1;
            player.ActiveFrenzy = 1;
            player.attackDmg = 1;
            player.shielded = false;
        }
    }
}

void GameLoop(int move, Character player, Character enemy) {
    bool exit;
    initialize_Q_table();
    while (player.hp > 0 && enemy.hp > 0) {
        if (player.poisoned != 0) {
            player.hp -= 4;
            player.poisoned -= 1;
            cout << "You take 4 damage. You are poisoned for " << player.poisoned << " more turn(s)." << endl;
        }
        if (enemy.poisoned != 0) {
            enemy.hp -= 4;
            enemy.poisoned -= 1;
            cout << "The Enemy Cyborg takes 4 damage. They are poisoned for " << enemy.poisoned << " more turn(s)." << endl;
        }
        if ((player.Covered == 0) && (player.CoverT>0)) {
            player.CoverT -= 1;
        }
        else if ((player.Covered == 0) && (player.CoverT == 0)) {
            player.ActiveCover = 0;
        }
        if ((enemy.Covered != 1) && (enemy.CoverT = !0)) {
            enemy.CoverT -= 1;
        }
        else if ((enemy.Covered == 0) && (enemy.CoverT == 0)) {
            enemy.ActiveCover = 0;
        }
        if ((player.Frenzied == 0) && (player.FrenzT > 0)) {
            player.FrenzT -= 1;
        }
        else if ((player.Frenzied == 0) && (player.FrenzT == 0)) {
            player.ActiveFrenzy = 0;
        }
        if ((enemy.Frenzied == 0) && (enemy.FrenzT > 0)) {
            enemy.FrenzT -= 1;
        }
        else if ((enemy.Frenzied == 0) && (enemy.FrenzT == 0)) {
            enemy.ActiveFrenzy = 0;
        }

        if (enemy.hp <= 0 && player.hp > 0) {
            exit = win(player, enemy);
        }
        else if (player.hp <= 0 && enemy.hp > 0) {
            exit = lose(player, enemy);
        }
        else if (player.hp == 0 && enemy.hp == 0) {
            exit = lose(player, enemy);
        }
        int AIMove = 0;
        AIMove = enemy.do_ai_stuff(player, enemy, move, AIMove);

        handleMove(move, enemy, player, AIMove);
        handleMove(move, player, enemy);

        GameState current_state = get_current_state(player, enemy);
        int action = enemy.do_ai_stuff(player, enemy, move, action);
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
            reset_player(player);
            reset_player(enemy);

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
            reset_player(player);
            reset_player(enemy);
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

void reset_player(Character& player) {
    player.hp = 100;
    player.shots = 6;
    player.shieldUsed = false;
    player.shielded = false;
    player.attackDmg = 2;
    player.healing = 3;
    player.poison = 1;
    player.poisoned = 0;
    player.Covered = 1;
    player.CoverT = 3;
    player.Frenzied = 1;
    player.FrenzT = 3;
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    Character player((string)"Cowboy", 100, 6, false, false, 4, 3, 1, 0, 1, 3, 1, 3, false, false, false);
    Character enemy((string)"Bandit", 100, 6, false, false, 4, 3, 1, 0, 1, 3, 1, 3, false, false, true);
    int move;

    initialize_Q_table();

    while (true) {
        while (true) {
            cout
                << "Player HP: " << player.hp << endl
                << "Player Shots: " << player.shots << endl
                << "Player Shielded: " << player.shielded << endl
                << "Player Shield Used: " << player.shieldUsed << endl
                << "Enemy A.I. HP: " << enemy.hp << endl
                << "Enemy A.I. Shots: " << enemy.shots << endl
                << "Enemy A.I. Shielded: " << enemy.shielded << endl
                << "Enemy A.I. Shield Used: " << enemy.shieldUsed << endl
                << "(1) Saber" << endl
                << "(2) Revolver ("<< player.shots << " Shots left)" << endl
                << "(3) Reload Revolver" << endl
                << "(4) Fan the Gun!" << endl
                << "(5) Healing Pack ("<< player.healing << "Packs left)" << endl
                << "(6) Poison Dart ("<< player.poison << "Darts left)" << endl
                << "(7) Lightning Reflexes (" << (int)(player.shieldUsed ? 0 : 1) << " Use left)" << endl
                << "(8) Take Cover! ("<< player.Covered << "Cover left)" << endl
                << "(9) Frenzy! ("<< player.Frenzied << "Use left)" << endl;

            cin >> move;
            if (move < ATTACK || move > FRENZY) {
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


