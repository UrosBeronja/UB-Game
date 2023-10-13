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
                Q_table[i][j] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
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

    index *= PLAYER_HP_STATES;
    index += state.player_hp;
    index *= PLAYER_SHIELDED_STATES;
    index += state.player_shielded ? 1 : 0;
    index *= PLAYER_HEALING_STATUS;
    index *= state.player_healing;
    index *= state.player_Cover;

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
    index *= AI_COVER_STATUS;
    index *= state.ai_Cover;
    index *= AI_COVERED_STATUS;
    index *= state.ai_Covered;


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
        bool e=1;
        GameState current_state = get_current_state(player, enemy);
        float epsilon = 0.2; // Set your exploration rate (epsilon) as needed
        while (e!=0){
            if (rand() / static_cast<float>(RAND_MAX) < epsilon) {
                AIMove = rand() % NUM_ACTIONS; // Explore (choose a random action)
            }
            else {
                AIMove = choose_action(current_state); // Exploit (choose the action with the highest Q-value)
            }

            GameState next_state = execute_action(AIMove, move, player, enemy);
            if (next_state.ai_hp==-100) {
                minus += 5;
                e = 1;
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


int handleAIMove(int move, Character& player, Character& enemy, int enMove) {
    int dmg = enemy.attackDmg;
    bool sh = 0;
    bool cv = 0;
    int reward = player.hp;
    int hp = enemy.hp;

    if (enemy.isAI == 1) {

        if (enMove == SHIELD) {
            sh = 1;
        }if (enMove == COVER) {
            cv = 1;
        }

        ////AI MOVES
        if (move == ATTACK) {
            if ((enemy.Covered < 1) && (enemy.CoverT > 0)) {
                cout << "You cant use your Saber while in Cover." << endl;
                return 100;
            }
            cout << "You sliced the Cyborg with your sword. You deal " << 1 * (enemy.attackDmg * (sh ? 0 : 1) *
                (player.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1)) << " damage." << endl;
            player.hp -= 1 * (enemy.attackDmg * (sh ? 0 : 1) * (player.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1));
            enemy.attackDmg = dmg;
            enemy.shielded = false;
        }
        else if (move == FAN) {
            int i = 4;
            if (enemy.shots < i) {
                cout << "You don't have enough shots to Fan the Gun." << endl;
                return 100;
            }

            cout << "You Fan the Gun!" << endl;
            while (i > 0) {
                enemy.hp -= 1.5 * enemy.attackDmg *(player.attackDmg * (sh ? 0 : 1)*(player.ActiveCover ? 0.5 : 1)
                    * (cv ? 0.5 : 1));
                player.shots -= 1;
                i--;
                cout << "A Shot hits the Cyborg for " << 1.5 * (enemy.attackDmg * (sh ? 0 : 1) *(player.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1))
                    << " damage!" << endl;
            }
            player.attackDmg = dmg;
            player.shielded = false;
        }
        else if (move == RELOAD) {
            cout << "You Reload your Revolver." << endl;
            enemy.shots = 6;
            enemy.attackDmg = dmg;
            enemy.shielded = false;
        }
        else if (move == REVOLVER) {
            if (enemy.shots < 1) {
                cout << "You don't have enough shots to use the Revolver." << endl;
                return 100;
            }
            cout << "You execute a devastating Shot! You deal " << 1.5 * enemy.attackDmg * (player.shielded ? 0 : 1) 
                *(player.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1) << "damage to the Cyborg." << endl;
            enemy.hp -= 1.5 * enemy.attackDmg * (player.shielded ? 0 : 1) * (player.ActiveCover ? 0.5 : 1) * (cv ? 0.5 : 1);
            enemy.shots -= 1;
            enemy.attackDmg = dmg;
            enemy.shielded = false;
        }
        else if (move == HEAL) {
            if (enemy.healing < 1) {
                cout << "You don't have any Healing Packs left." << endl;
                return 100;
            }
            cout << "You Heal your wounds! You heal " << 2 * enemy.attackDmg << " wounds." << endl;
            enemy.hp += 2 * enemy.attackDmg;
            if (enemy.hp > 100) {
                enemy.hp = 100;
            }
            enemy.attackDmg = dmg;
            enemy.shielded = false;
        }
        else if (move == POISON) {
            if (enemy.poison < 1) {
                cout << "You don't have any more Poisons." << endl;
                return 100;
            }
            cout << "You manage to Poison your Enemy! He will be poisoned for 3 Turns. This damage bypasses Cover and Frenzy." << endl;
            player.poisoned = 3 * (player.shielded ? 0 : 1);
            enemy.poison -= 1;
            enemy.attackDmg = dmg;
            enemy.shielded = false;
        }
        else if (move == SHIELD) {
            if (enemy.shieldUsed) {
                cout << "You have already used your Lightning Reflexes." << endl;
                return 100;
            }
            cout << "You use the burst of speed to evade any attack and set up for a devestating counter attack or heal!" << endl;
            enemy.shieldUsed = true;
            enemy.shielded = true;
            enemy.attackDmg = 2* dmg;
        }
        else if (move == COVER) {
            if (enemy.Covered < 1) {
                cout << "You have already used your Cover." << endl;
                return 100;
            }
            cout << "You expertly jump into Cover! You will take half damage for 3 turns, but you wont be able to use your Saber or Frenzy." << endl;
            enemy.Covered -= 1;
            enemy.ActiveCover = 1;
            enemy.attackDmg = dmg;
            enemy.shielded = false;
        }
    }
    reward -= player.hp;
    int newhp = enemy.hp;
    newhp -= hp;
    reward += newhp;
    return reward;
}



void handleMove(int move, Character& player, Character& enemy) {
    ////PLAYER MOVES
    int dmg = player.attackDmg;
    if (move == ATTACK) {
        if ((player.Covered < 1) && (player.CoverT > 0)) {
            cout << "You cant use your Saber while in Cover." << endl;
            return;
        }
        cout << "You sliced the Cyborg with your sword. You deal " << 1 * player.attackDmg * (enemy.shielded ? 0 : 1) *
            (enemy.ActiveCover ? 0.5 : 1) << " damage." << endl;
        enemy.hp -= 1 * player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1);
        player.attackDmg = dmg;
        player.shielded = false;
    }
    else if (move == FAN) {
        int i = 4;
        if (player.shots < i) {
            cout << "You don't have enough shots to Fan the Gun." << endl;
            return;
        }

        cout << "You Fan the Gun!" << endl;
        while (i > 0) {
            player.hp -= 1.5 *player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1);
            enemy.shots -= 1;
            i -= 1;
            cout << "A Shot hits the Cyborg for " << 1.5 * (enemy.shielded ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1) << " damage!" << endl;
        }
        enemy.attackDmg = dmg;
        enemy.shielded = false;
    }
    else if (move == RELOAD) {
        cout << "You Reload your Revolver." << endl;
        player.shots = 6;
        player.attackDmg = dmg;
        player.shielded = false;
    }
    else if (move == REVOLVER) {
        if (player.shots < 1) {
            cout << "You don't have enough shots to use the Revolver." << endl;
            return;
        }
       
        cout << "You execute a devastating Shot! You deal " << 1.5 * (player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1))
            << "damage to the Cyborg." << endl;
        enemy.hp -= 1.5 *player.attackDmg *(player.attackDmg * (enemy.shielded ? 0 : 1) * (enemy.ActiveCover ? 0.5 : 1));
        player.shots -= 1;
        player.attackDmg = dmg;
        player.shielded = false;
    }
    else if (move == HEAL) {
        if (player.healing < 1) {
            cout << "You don't have any Healing Packs left." << endl;
            return;
        }
        cout << "You Heal your wounds! You heal " << 2 * player.attackDmg << " wounds." << endl;
        player.hp += 2 * player.attackDmg;
        if (player.hp > 100) {
            player.hp = 100;
        }
        player.attackDmg = dmg;
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
        player.attackDmg = dmg;
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
        player.attackDmg = 2* dmg;
    }
    else if (move == COVER) {
        if (player.Covered < 1) {
            cout << "You have already used your Cover." << endl;
            return;
        }
        cout << "You expertly jump into Cover! You will take half damage for 3 turns, but you wont be able to use your Saber or Frenzy." << endl;
        player.Covered -= 1;
        player.ActiveCover = 1;
        player.attackDmg = dmg;
        player.shielded = false;
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
}

int main() {
    srand(static_cast<unsigned>(time(0)));
    Character player( 100, 6, false, false, 4, 3, 1, 0, 1, 3, false, false);
    Character enemy( 100, 6, false, false, 4, 3, 1, 0, 1, 3, false, true);
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
                << "(2) Revolver (" << player.shots << " Shots left)" << endl
                << "(3) Reload Revolver" << endl
                << "(4) Fan the Gun!" << endl
                << "(5) Healing Pack (" << player.healing << "Packs left)" << endl
                << "(6) Poison Dart (" << player.poison << "Darts left)" << endl
                << "(7) Lightning Reflexes (" << (int)(player.shieldUsed ? 0 : 1) << " Use left)" << endl
                << "(8) Take Cover! (" << player.Covered << "Cover left)" << endl;

            cin >> move;
            if (move < ATTACK || move > COVER) {
                cout << "Invalid move. Please try again." << endl;
                continue;
            }
            bool exit;
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

            if (enemy.hp <= 0 && player.hp > 0) {
                exit = win(player, enemy);
            }
            else if (player.hp <= 0 && enemy.hp > 0) {
                exit = lose(player, enemy);
            }
            else if (player.hp == 0 && enemy.hp == 0) {
                exit = lose(player, enemy);
            }

            handleMove(move, player, enemy);
            enemy.do_ai_stuff(player, enemy, move);

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
        if (player.hp <= 0 || enemy.hp <= 0) {
            break;
            
        }
        return 0;
    }
}


