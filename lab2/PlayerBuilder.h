#ifndef PLAYER_BUILDER_H
#define PLAYER_BUILDER_H
#include "StoryEntities.h"
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
using namespace std;

class PlayerBuilder {
private:
    string name_;
    int health_ = 100;
    int money_ = 100;
    int intelligence_ = 10;
    int reputation_ = 0;
    vector<string> inventory_;

public:
    PlayerBuilder& setName(string name) {
        name_ = move(name);
        return *this;
    }

    PlayerBuilder& setHealth(int health) {
        health_ = health;
        return *this;
    }

    PlayerBuilder& setMoney(int money) {
        money_ = money;
        return *this;
    }

    PlayerBuilder& setIntelligence(int intelligence) {
        intelligence_ = intelligence;
        return *this;
    }

    PlayerBuilder& setReputation(int reputation) {
        reputation_ = reputation;
        return *this;
    }

    PlayerBuilder& addItem(string item) {
        inventory_.push_back(move(item));
        return *this;
    }

    Player build() const {
        if (name_.empty()) {
            throw invalid_argument("Player name cannot be empty.");
        }
        if (health_ < 0 || health_ > 100) {
            throw invalid_argument("Player health must be between 0 and 100.");
        }
        if (money_ < 0) {
            throw invalid_argument("Player money cannot be negative.");
        }
        if (intelligence_ < 0 || intelligence_ > 100) {
            throw invalid_argument("Player intelligence must be between 0 and 100.");
        }
        if (reputation_ < -100 || reputation_ > 100) {
            throw invalid_argument("Player reputation must be between -100 and 100.");
        }
        return Player(name_, health_, money_, intelligence_, reputation_, inventory_);
    }
};

#endif
