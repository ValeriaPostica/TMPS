#ifndef STORY_ENTITIES_H
#define STORY_ENTITIES_H

#include <iostream>
#include <string>
#include <vector>
#include "GameInterfaces.h"

using namespace std;

// 2. SRP: Player class handles only stats, items, and game state
class Player {
private:
    string name;
    int health;
    int money;
    int intelligence;
    int reputation;
    vector<string> inventory;
    bool isGameOver;
    string endingMessage;

public:
    Player(string n) {
        name = n;
        health = 100;
        money = 100;
        intelligence = 10;
        reputation = 0;
        isGameOver = false;
        endingMessage = "";
    }

    string getName() { return name; }
    int getHealth() { return health; }
    int getMoney() { return money; }
    int getIntelligence() { return intelligence; }
    int getReputation() { return reputation; }
    bool getIsGameOver() { return isGameOver; }
    string getEndingMessage() { return endingMessage; }

    void modifyHealth(int amount) {
        health += amount;
        if (health > 100) health = 100;
        if (health <= 0) {
            health = 0;
            triggerEnd("YOU DIED: You could not survive the harsh streets of FAF-City.");
        }
    }

    void modifyMoney(int amount) {
        money += amount;
        if (money < 0) money = 0;
    }

    void modifyIntelligence(int amount) { intelligence += amount; }
    void modifyReputation(int amount) { reputation += amount; }

    void addItem(string item) {
        inventory.push_back(item);
        cout << " Item obtained: " << item << "\n";
    }

    bool hasItem(string item) {
        for (string i : inventory) {
            if (i == item) return true;
        }
        return false;
    }

    void triggerEnd(string message) {
        isGameOver = true;
        endingMessage = message;
    }

    void displayStats() {
        cout << "\nPlayer: " << name 
             << " | HP: " << health 
             << " | Money: $" << money 
             << " | INT: " << intelligence 
             << " | REP: " << reputation << "\n";
        cout << "Inventory: ";
        if (inventory.empty()) {
            cout << "Empty";
        } else {
            for (size_t i = 0; i < inventory.size(); i++) {
                cout << inventory[i] << (i == inventory.size() - 1 ? "" : ", ");
            }
        }
        cout << "\n\n";
    }
};

// Polymorphic Effects (OCP)
class StatEffect : public IActionEffect {
private:
    int health, money, intel, rep;
public:
    StatEffect(int h, int m, int i, int r) {
        health = h;
        money = m;
        intel = i;
        rep = r;
    }
    void apply(Player& player) {
        player.modifyHealth(health);
        player.modifyMoney(money);
        player.modifyIntelligence(intel);
        player.modifyReputation(rep);
    }
};

class ItemEffect : public IActionEffect {
private:
    string item;
public:
    ItemEffect(string itemName) {
        item = itemName;
    }
    void apply(Player& player) {
        player.addItem(item);
    }
};

class EndingEffect : public IActionEffect {
private:
    string finalMessage;
public:
    EndingEffect(string msg) {
        finalMessage = msg;
    }
    void apply(Player& player) {
        player.triggerEnd(finalMessage);
    }
};

// Choice structure
struct Choice {
    string description;
    int nextNodeId;
    string requiredItem;
    int requiredIntel;
    vector<IActionEffect*> effects;
};

// Scene representation
class StoryNode {
private:
    int id;
    string text;
    vector<Choice> choices;

public:
    StoryNode(int nodeId, string storyText) {
        id = nodeId;
        text = storyText;
    }

    ~StoryNode() {
        for (Choice c : choices) {
            for (IActionEffect* e : c.effects) {
                delete e;
            }
        }
    }

    int getId() { return id; }
    string getText() { return text; }

    void addChoice(string desc, int nextNode, vector<IActionEffect*> fx = {}, string reqItem = "", int reqIntel = 0) {
        choices.push_back({desc, nextNode, reqItem, reqIntel, fx});
    }

    void displayNode() {
        cout << text << "\n\n";
        for (size_t i = 0; i < choices.size(); i++) {
            cout << i + 1 << ". " << choices[i].description;
            if (!choices[i].requiredItem.empty()) {
                cout << " (Requires: " << choices[i].requiredItem << ")";
            }
            if (choices[i].requiredIntel > 0) {
                cout << " (Requires INT: " << choices[i].requiredIntel << ")";
            }
            cout << "\n";
        }
    }

    int getChoiceCount() { return choices.size(); }
    Choice getChoice(int index) { return choices[index]; }
};

#endif