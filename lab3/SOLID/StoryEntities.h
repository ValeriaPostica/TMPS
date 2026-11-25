#ifndef STORY_ENTITIES_H
#define STORY_ENTITIES_H
#include "GameInterfaces.h"
#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;

class Player {
private:
    string name_;
    int health_ = 100;
    int money_ = 100;
    int intelligence_ = 10;
    int reputation_ = 0;
    vector<string> inventory_;
    bool gameOver_ = false;
    string endingMessage_;

public:
    explicit Player(string name) : name_(move(name)) {}
    Player(string name, int health, int money, int intelligence, int reputation,
           vector<string> inventory = {})
        : name_(move(name)),
          health_(health),
          money_(money),
          intelligence_(intelligence),
          reputation_(reputation),
          inventory_(move(inventory)) {}

    const string& name() const { return name_; }
    int health() const { return health_; }
    int money() const { return money_; }
    int intelligence() const { return intelligence_; }
    int reputation() const { return reputation_; }
    bool isGameOver() const { return gameOver_; }
    const string& endingMessage() const { return endingMessage_; }

    struct StateSnapshot {
        int health;
        int money;
        int intelligence;
        int reputation;
        vector<string> inventory;
        bool gameOver;
        string endingMessage;
    };

    StateSnapshot saveState() const {
        return {health_, money_, intelligence_, reputation_, inventory_,
                gameOver_, endingMessage_};
    }

    void restoreState(const StateSnapshot& snapshot) {
        health_ = snapshot.health;
        money_ = snapshot.money;
        intelligence_ = snapshot.intelligence;
        reputation_ = snapshot.reputation;
        inventory_ = snapshot.inventory;
        gameOver_ = snapshot.gameOver;
        endingMessage_ = snapshot.endingMessage;
    }

    void modifyHealth(int amount) {
        health_ = clamp(health_ + amount, 0, 100);
        if (health_ == 0) {
            finish("GAME OVER: Your health reached zero in FAF City.");
        }
    }

    void modifyMoney(int amount) { money_ = max(0, money_ + amount); }
    void modifyIntelligence(int amount) { intelligence_ += amount; }
    void modifyReputation(int amount) { reputation_ += amount; }

    void addItem(const string& item) {
        inventory_.push_back(item);
    }

    bool hasItem(const string& item) const {
        return find(inventory_.begin(), inventory_.end(), item) != inventory_.end();
    }

    void finish(const string& message) {
        gameOver_ = true;
        endingMessage_ = message;
    }

    string statsMessage() const {
        string message = "Player: " + name_ +
                         " | HP: " + to_string(health_) +
                         " | Money: $" + to_string(money_) +
                         " | INT: " + to_string(intelligence_) +
                         " | REP: " + to_string(reputation_) +
                         " | Inventory: ";
        if (inventory_.empty()) {
            message += "Empty";
        } else {
            for (size_t i = 0; i < inventory_.size(); ++i) {
                if (i > 0) message += ", ";
                message += inventory_[i];
            }
        }
        return message;
    }
};

class StatEffect final : public IActionEffect {
private:
    int health_;
    int money_;
    int intelligence_;
    int reputation_;

public:
    StatEffect(int health, int money, int intelligence, int reputation)
        : health_(health), money_(money), intelligence_(intelligence), reputation_(reputation) {}

    void apply(Player& player) override {
        player.modifyHealth(health_);
        player.modifyMoney(money_);
        player.modifyIntelligence(intelligence_);
        player.modifyReputation(reputation_);
    }
};

class ItemEffect final : public IActionEffect {
private:
    string item_;

public:
    explicit ItemEffect(string item) : item_(move(item)) {}

    void apply(Player& player) override { player.addItem(item_); }
};

class EndingEffect final : public IActionEffect {
private:
    string message_;

public:
    explicit EndingEffect(string message) : message_(move(message)) {}

    void apply(Player& player) override { player.finish(message_); }
};

struct Choice {
    string description;
    int nextNodeId;
    string requiredItem;
    int requiredIntelligence;
    shared_ptr<IActionEffect> effect;
};

class StoryNode {
private:
    int id_;
    string text_;
    vector<Choice> choices_;

public:
    StoryNode(int id, string text) : id_(id), text_(move(text)) {}

    int id() const { return id_; }
    const string& text() const { return text_; }
    const vector<Choice>& choices() const { return choices_; }

    void addChoice(string description, int nextNodeId,
                   shared_ptr<IActionEffect> effect = nullptr,
                   string requiredItem = "", int requiredIntelligence = 0) {
        choices_.push_back({move(description), nextNodeId, move(requiredItem),
                            requiredIntelligence, move(effect)});
    }
};

#endif
