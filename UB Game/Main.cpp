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
        vector<vector<float>> neural_network(3);
        // AI logic here
        return 0;
    }
};

void handleMove(int move, Character& player, Character& enemy) {
    int AIMove;
    bool sh=0;
    if (player.isAI == 1) {
        //AI generates its moves
        AIMove=enemy.do_ai_stuff();
    }
    if (move == 3) {
        sh = 1;
    }


    if (AIMove == ATTACK) {
        cout << "A.I. slices you with a sword." << endl;
        player.hp -= 1 * (enemy.attackDmg * (sh ? 0 : 1));
        enemy.attackDmg = 1;
        enemy.shielded = false;
    }
    else if (AIMove == FAN) {
        if (enemy.shots < 1) {
            cout << "A.I. doesnt have enough shots to Fan the Gun." << endl;
            return;
        }
        cout << "A.I. Fans the Gun!" << endl;
        while (enemy.shots != 0) {
            player.hp -= 1.5 * (enemy.attackDmg * (sh ? 0 : 1));
            enemy.shots -= 1;
            cout << "A Shot hits you!" << endl;
        }
        enemy.attackDmg = 1;
        enemy.shielded = false;
    }
    else if (AIMove == REVOLVER) {
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

   
    if (move == ATTACK) {
        cout << "You sliced the enemy with your sword." << endl;
        enemy.hp -= 1 * (player.attackDmg * (enemy.shielded ? 0 : 1));
        player.attackDmg = 1;
        player.shielded = false;
    }
    else if (move == FAN) {
        if (player.shots < 15) {
            cout << "You don't have enough shots to summon Ultima." << endl;
            return;
        }
        cout << "You summoned Ultima!" << endl;
        enemy.hp -= 10 * (player.attackDmg * (enemy.shielded ? 0 : 1));
        player.shots -= 15;
        player.attackDmg = 1;
        player.shielded = false;
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

void GameLoop(int move, Character player, Character enemy) {
    // Initialize characters, environment, and other game elements

    // Main game loop
    while (player.hp > 0 && enemy.hp > 0) {
        //processing the AI turn first, but giving it the players chosen move to apply the shied and dodge correctly
        handleMove(move, enemy, player);

        handleMove(move, player, enemy);
        
        




    }

    // Display game outcome and handle game over logic
}

bool win(Character& player) {
    string response;
    while (true) {
        cout << "You win. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            player.hp = 50;
            player.shots = 6;
            player.shieldUsed = false;
            player.shielded = false;
            player.attackDmg = 1;
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

bool lose(Character& player) {
    string response;
    while (true) {
        cout << "You lose. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            player.hp = 20;
            player.shots = 6;
            player.shieldUsed = false;
            player.shielded = false;
            player.attackDmg = 1;
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
    Character player{ "Cowboy", 50, 6, false, false, 2, 0};
    Character enemy{ "Bandit", 50, 6, false, false, 2, 1 };
    int move;
    while (true) {
        while (true) {
            cout << "Enemy HP: " << enemy.hp << endl
                << "Player HP: " << player.hp << endl
                << "Player Shots: " << player.shots << endl
                << "Enemy Shots: " << enemy.shots << endl 
                << "Player Shielded: " << player.shielded << endl
                << "Enemy Shielded: " << enemy.shielded << endl
                << "Player Shield Used: " << player.shieldUsed << endl
                << "Enemy Shield Used: " << enemy.shieldUsed << endl
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

