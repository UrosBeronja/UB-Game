#include <iostream>
#include <string.h>
#include <boost/math_fwd.hpp>
#include "openai.hpp"

using namespace std;

// Constants for the different moves
const int ATTACK = 1;
const int MAGIC_MISSILE = 2;
const int SUMMON = 4;
const int SHIELD = 3;

// Struct to hold player and enemy data
struct Character {
    int hp;
    int mp;
    bool shieldUsed;
    bool shielded;
    int attackMultiplier;
};

// Function to handle losing
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
            player.mp = 20;
            player.shieldUsed = false;
            player.shielded = false;
            player.attackMultiplier = 1;
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

// Function to handle winning
bool win(Character& player) {
    string response;
    while (true) {
        cout << "You win. Would you like to play again? (yes/no)" << endl;
        cin >> response;
        for (char& c : response) {
            c = tolower(c);
        }
        if (response == "yes") {
            player.hp = 20;
            player.mp = 20;
            player.shieldUsed = false;
            player.shielded = false;
            player.attackMultiplier = 1;
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

// Function to handle a move
void handleMove(int move, Character& player, Character& enemy) {
    // Check if enemy used shield
    if (enemy.shielded) {
        enemy.shielded = true;
    }
    if (move == ATTACK) {
        cout << "You sliced the enemy with your sword." << endl;
        enemy.hp -= 1 * (player.attackMultiplier * (enemy.shielded ? 0 : 1));
        player.attackMultiplier = 1;
        player.shielded = false;
    }
    else if (move == SUMMON) {
        if (player.mp < 15) {
            cout << "You don't have enough MP to summon Ultima." << endl;
            return;
        }
        cout << "You summoned Ultima!" << endl;
        enemy.hp -= 10 * (player.attackMultiplier * (enemy.shielded ? 0 : 1));
        player.mp -= 15;
        player.attackMultiplier = 1;
        player.shielded = false;
    }
    else if (move == MAGIC_MISSILE) {
        if (player.mp < 1) {
            cout << "You don't have enough MP to cast Magic Missile." << endl;
            return;
        }
        cout << "You cast a devastating Magic Missile." << endl;
        enemy.hp -= 2 * (player.attackMultiplier * (enemy.shielded ? 0 : 1));
        player.mp -= 1;
        player.attackMultiplier = 1;
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
        player.attackMultiplier = 2;
    }

    //Enemy's turn
    int enemyMove = rand() % 3 + 1; // Random number between 1 and 3
    cout << "The enemy generates move: " << enemyMove << endl;
    if (enemyMove == ATTACK || (enemyMove == MAGIC_MISSILE && enemy.mp <= 0) || (enemyMove == SHIELD && enemy.shieldUsed == true)) {
        cout << "The enemy attacks you." << endl;
        player.hp -= 1 * (enemy.attackMultiplier * (player.shielded ? 0 : 1));
        enemy.attackMultiplier = 1;
        enemy.shielded = false;
    }
    else if (enemyMove == MAGIC_MISSILE && enemy.mp > 0) {
        cout << "The enemy casts Magic Missile." << endl;
        player.hp -= 2 * (enemy.attackMultiplier * (player.shielded ? 0 : 1));
        enemy.mp -= 1;
        enemy.attackMultiplier = 1;
        enemy.shielded = false;
    }
    else if (enemyMove == SHIELD) {
        cout << "The enemy raises its shield." << endl;
        enemy.shieldUsed = true;
        enemy.shielded = true;
        enemy.attackMultiplier = 2;
    }


    // Check win/lose conditions
    if (enemy.hp <= 0) {
        if (!win(player)) {
            exit(0);
        }
    }
    else if (player.hp <= 0) {
        if (!lose(player)) {
            exit(0);
        }
    }
}

int main() {
    Character player{ 20, 20, false, false, 1 };
    Character enemy{ 30, 10, false, false, 1 };
    int move;

    while (true) {
        while (true) {
            cout << "Enemy HP: " << enemy.hp << endl
                << "Player HP: " << player.hp << endl
                << "Player MP: " << player.mp << endl
                << "Enemy MP: " << enemy.mp << endl
                << "(1) Attack" << endl
                << "(2) Magic Missile" << endl
                << "(3) Shield" << endl
                << "(4) Summon" << endl;

            cin >> move;

            if (move < ATTACK || move > SUMMON) {
                cout << "Invalid move. Please try again." << endl;
                continue;
            }

            handleMove(move, player, enemy);

            if (player.hp <= 0 || enemy.hp <= 0) {
                break;
            }
        }

        enemy.hp = 30; // Reset enemy HP for the next game
        enemy.mp = 10; // Reset enemy MP for the next game
        enemy.shieldUsed = false; // Reset enemy shield for the next game
    }

    return 0;