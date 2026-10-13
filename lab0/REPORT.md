# Laboratory Work No. 0

**Course:** TMPS

## 1. Introduction

**FAF City: Choose Your Destiny** is a terminal-based *Choose Your Own Adventure* (CYOA) game written in C++. The player creates a hero and explores a futuristic version of FAF City through a directed graph of 20 story nodes. At every step, the current node presents exactly three choices. A choice can move the player to another node, change one or more character statistics, add an item to the inventory, or finish the story with an ending.

The player state contains four gameplay statistics:

- **Health** - physical condition; reaching zero ends the game.
- **Money** - available funds used by some story decisions.
- **Intelligence** - determines whether technically difficult choices are available.
- **Reputation** - represents the player's standing in FAF City.

The state also contains an inventory of items such as a Neural Chip, Medkit, Stun Gun, and Cyber-Deck. Choices may require a particular item or a minimum Intelligence value. The game engine validates those requirements before applying effects and following the selected connection in the story graph. Because every node has three choices, the player can repeatedly make meaningful decisions while different paths lead to different endings.

## 2. SOLID Principles Applied

### 2.1 Single Responsibility Principle (SRP)

Each major class has one focused responsibility:

- **`Player`** owns the mutable state of the hero. It stores the name, statistics, inventory, and final status, and exposes operations for changing that state. It does not decide which story node should be visited.
- **`StoryNode`** represents one piece of narrative and its three `Choice` objects. It knows how to expose choices and their requirements, but it does not run the game loop.
- **`GameEngine`** coordinates execution. It owns the story graph, displays nodes through an `IOutputWriter`, validates input and requirements, applies effects, and moves to the next node. Story data and player state remain separate from this orchestration logic.

This separation means that changing the player model, adding a different node representation, or changing navigation rules does not require modifying all other classes.

### 2.2 Open/Closed Principle (OCP)

Choices store polymorphic objects through the `IActionEffect` interface. The current implementation provides:

- **`StatEffect`**, which changes Health, Money, Intelligence, and Reputation.
- **`ItemEffect`**, which adds an item to the player's inventory.
- **`EndingEffect`**, which marks the game as finished and stores the ending text.

`GameEngine` only invokes `effect->apply(player)`. It does not contain a conditional branch for every effect type. A future `LocationEffect`, `TimeEffect`, or `AchievementEffect` can be added by implementing `IActionEffect`; the engine and existing story-node code remain closed for modification.

### 2.3 Dependency Inversion Principle (DIP)

The high-level `GameEngine` depends on the `IOutputWriter` abstraction rather than directly depending on terminal output. `ConsoleWriter` is one low-level implementation of that interface and is injected into the engine by reference. A test writer, file writer, or graphical adapter can replace `ConsoleWriter` without changing the engine's navigation logic.

The same principle is used for choice effects: the engine depends on the abstract `IActionEffect`, not on concrete effect classes. These dependencies are passed into the game instead of being constructed inside the orchestration loop.

## 3. Source Code Structure

The source is split into three files:

```text
lab0/
├── GameInterfaces.h   # IActionEffect and IOutputWriter abstractions
├── StoryEntities.h    # Player, effects, Choice, and StoryNode
└── main.cpp           # GameEngine, 20 nodes, and main()
```

The following listings form a complete C++17 implementation.

### 3.1 `GameInterfaces.h`

```cpp
#ifndef GAME_INTERFACES_H
#define GAME_INTERFACES_H

#include <iostream>
#include <string>

class Player;

class IActionEffect {
public:
    virtual ~IActionEffect() = default;
    virtual void apply(Player& player) = 0;
};

class IOutputWriter {
public:
    virtual ~IOutputWriter() = default;
    virtual void write(const std::string& message) = 0;
};

class ConsoleWriter final : public IOutputWriter {
public:
    void write(const std::string& message) override {
        std::cout << message << '\n';
    }
};

#endif
```

### 3.2 `StoryEntities.h`

```cpp
#ifndef STORY_ENTITIES_H
#define STORY_ENTITIES_H

#include "GameInterfaces.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Player {
private:
    std::string name_;
    int health_ = 100;
    int money_ = 100;
    int intelligence_ = 10;
    int reputation_ = 0;
    std::vector<std::string> inventory_;
    bool gameOver_ = false;
    std::string endingMessage_;

public:
    explicit Player(std::string name) : name_(std::move(name)) {}

    const std::string& name() const { return name_; }
    int health() const { return health_; }
    int money() const { return money_; }
    int intelligence() const { return intelligence_; }
    int reputation() const { return reputation_; }
    bool isGameOver() const { return gameOver_; }
    const std::string& endingMessage() const { return endingMessage_; }

    void modifyHealth(int amount) {
        health_ = std::clamp(health_ + amount, 0, 100);
        if (health_ == 0) {
            finish("GAME OVER: Your health reached zero in FAF City.");
        }
    }

    void modifyMoney(int amount) { money_ = std::max(0, money_ + amount); }
    void modifyIntelligence(int amount) { intelligence_ += amount; }
    void modifyReputation(int amount) { reputation_ += amount; }

    void addItem(const std::string& item) {
        inventory_.push_back(item);
        std::cout << "Item obtained: " << item << '\n';
    }

    bool hasItem(const std::string& item) const {
        return std::find(inventory_.begin(), inventory_.end(), item) != inventory_.end();
    }

    void finish(const std::string& message) {
        gameOver_ = true;
        endingMessage_ = message;
    }

    void displayStats() const {
        std::cout << "\nPlayer: " << name_
                  << " | Health: " << health_
                  << " | Money: $" << money_
                  << " | Intelligence: " << intelligence_
                  << " | Reputation: " << reputation_ << '\n';
        std::cout << "Inventory: ";
        if (inventory_.empty()) {
            std::cout << "Empty";
        } else {
            for (std::size_t i = 0; i < inventory_.size(); ++i) {
                if (i > 0) std::cout << ", ";
                std::cout << inventory_[i];
            }
        }
        std::cout << "\n\n";
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
    std::string item_;

public:
    explicit ItemEffect(std::string item) : item_(std::move(item)) {}

    void apply(Player& player) override { player.addItem(item_); }
};

class EndingEffect final : public IActionEffect {
private:
    std::string message_;

public:
    explicit EndingEffect(std::string message) : message_(std::move(message)) {}

    void apply(Player& player) override { player.finish(message_); }
};

struct Choice {
    std::string description;
    int nextNodeId;
    std::string requiredItem;
    int requiredIntelligence;
    std::vector<std::shared_ptr<IActionEffect>> effects;
};

class StoryNode {
private:
    int id_;
    std::string text_;
    std::vector<Choice> choices_;

public:
    StoryNode(int id, std::string text) : id_(id), text_(std::move(text)) {}

    int id() const { return id_; }
    const std::string& text() const { return text_; }
    const std::vector<Choice>& choices() const { return choices_; }

    void addChoice(std::string description, int nextNodeId,
                   std::vector<std::shared_ptr<IActionEffect>> effects = {},
                   std::string requiredItem = "", int requiredIntelligence = 0) {
        choices_.push_back({std::move(description), nextNodeId, std::move(requiredItem),
                            requiredIntelligence, std::move(effects)});
    }
};

#endif
```

### 3.3 `main.cpp`

```cpp
#include "StoryEntities.h"

#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>

class GameEngine {
private:
    Player& player_;
    IOutputWriter& writer_;
    std::map<int, std::unique_ptr<StoryNode>> storyGraph_;

    void showChoice(const Choice& choice, std::size_t index) const {
        std::cout << index + 1 << ". " << choice.description;
        if (!choice.requiredItem.empty()) {
            std::cout << " [requires " << choice.requiredItem << "]";
        }
        if (choice.requiredIntelligence > 0) {
            std::cout << " [requires INT " << choice.requiredIntelligence << "]";
        }
        std::cout << '\n';
    }

public:
    GameEngine(Player& player, IOutputWriter& writer)
        : player_(player), writer_(writer) {}

    void addNode(std::unique_ptr<StoryNode> node) {
        storyGraph_[node->id()] = std::move(node);
    }

    void start(int startNodeId) {
        writer_.write("FAF CITY: CHOOSE YOUR DESTINY");
        int currentNodeId = startNodeId;

        while (!player_.isGameOver()) {
            const auto nodeIt = storyGraph_.find(currentNodeId);
            if (nodeIt == storyGraph_.end()) {
                writer_.write("Error: story node is missing.");
                break;
            }

            const StoryNode& node = *nodeIt->second;
            player_.displayStats();
            writer_.write(node.text());
            for (std::size_t i = 0; i < node.choices().size(); ++i) {
                showChoice(node.choices()[i], i);
            }

            int selectedNumber = 0;
            std::cout << "Choose option: ";
            if (!(std::cin >> selectedNumber)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                writer_.write("Please enter a number from 1 to 3.");
                continue;
            }
            if (selectedNumber < 1 ||
                selectedNumber > static_cast<int>(node.choices().size())) {
                writer_.write("Invalid option. Choose 1, 2, or 3.");
                continue;
            }

            const Choice& choice = node.choices()[static_cast<std::size_t>(selectedNumber - 1)];
            if (!choice.requiredItem.empty() && !player_.hasItem(choice.requiredItem)) {
                writer_.write("You do not have the required item: " + choice.requiredItem);
                continue;
            }
            if (player_.intelligence() < choice.requiredIntelligence) {
                writer_.write("Your Intelligence is too low for this choice.");
                continue;
            }

            for (const auto& effect : choice.effects) {
                effect->apply(player_);
            }
            currentNodeId = choice.nextNodeId;
        }

        writer_.write(player_.endingMessage().empty()
                          ? "THE END - Thanks for playing!"
                          : player_.endingMessage());
    }
};

std::unique_ptr<StoryNode> node(int id, const std::string& text) {
    return std::make_unique<StoryNode>(id, text);
}

std::shared_ptr<IActionEffect> stats(int health, int money, int intelligence, int reputation) {
    return std::make_shared<StatEffect>(health, money, intelligence, reputation);
}

std::shared_ptr<IActionEffect> item(const std::string& name) {
    return std::make_shared<ItemEffect>(name);
}

std::shared_ptr<IActionEffect> ending(const std::string& message) {
    return std::make_shared<EndingEffect>(message);
}

int main() {
    std::string name;
    std::cout << "Enter your hero name: ";
    std::cin >> name;

    Player player(name);
    ConsoleWriter writer;
    GameEngine engine(player, writer);

    auto n1 = node(1, "You wake up in a rainy FAF City alley.");
    n1->addChoice("Buy a Neural Chip for $80", 2,
                  {stats(0, -80, 15, 0), item("Neural Chip")});
    n1->addChoice("Buy a Medkit and Stun Gun for $90", 3,
                  {stats(0, -90, 0, 5), item("Medkit"), item("Stun Gun")});
    n1->addChoice("Follow a street hacker to a bar", 4, {stats(0, 0, 5, 10)});
    engine.addNode(std::move(n1));

    auto n2 = node(2, "Your Neural Chip reveals a weakness in the city bank.");
    n2->addChoice("Hack the bank directly", 5, {stats(0, 0, 10, 0)}, "", 20);
    n2->addChoice("Sell the flaw to a Netrunner clan", 6, {stats(0, 500, 5, 10)});
    n2->addChoice("Apply for a C++ job at Megacorp", 7, {stats(0, 200, 10, 5)});
    engine.addNode(std::move(n2));

    auto n3 = node(3, "Street bandits surround you and demand your money.");
    n3->addChoice("Use the Stun Gun", 8, {stats(-10, 300, 0, 25)}, "Stun Gun");
    n3->addChoice("Give them $50 and run", 9, {stats(-10, -50, 0, -5)});
    n3->addChoice("Trade your Medkit for passage", 10, {stats(0, 0, 0, 10)}, "Medkit");
    engine.addNode(std::move(n3));

    auto n4 = node(4, "A fixer in the bar offers a risky corporate job.");
    n4->addChoice("Accept the corporate heist", 11, {stats(0, 0, 0, 15)});
    n4->addChoice("Work as bar security", 12, {stats(-10, 200, 0, 5)});
    n4->addChoice("Steal the fixer's Cyber-Deck", 13,
                  {stats(0, 0, 10, -10), item("Cyber-Deck")});
    engine.addNode(std::move(n4));

    auto n5 = node(5, "Your hack succeeds and millions in digital currency are exposed.");
    n5->addChoice("Buy an orbital palace", 14, {stats(0, 1000, 0, 10)});
    n5->addChoice("Erase Sector 7 citizens' debts", 15, {stats(0, 0, 0, 50)});
    n5->addChoice("Buy Megacorp stock", 16, {stats(0, 800, 0, 20)});
    engine.addNode(std::move(n5));

    auto n6 = node(6, "The Netrunners pay you and offer full membership.");
    n6->addChoice("Lead the clan into a city war", 15, {stats(-10, 0, 0, 40)});
    n6->addChoice("Invest in a technology company", 16, {stats(0, 300, 5, 15)});
    n6->addChoice("Gamble your earnings", 17, {stats(0, -500, 0, -10)});
    engine.addNode(std::move(n6));

    auto n7 = node(7, "You are hired as a coder and work every day.");
    n7->addChoice("Outsmart your boss", 16, {stats(0, 300, 5, 20)});
    n7->addChoice("Work one hundred hour weeks", 18, {stats(-40, 500, 10, 0)});
    n7->addChoice("Build an indie game", 19, {stats(10, 1000, 20, 20)});
    engine.addNode(std::move(n7));

    auto n8 = node(8, "The street clans respect your victory over the bandits.");
    n8->addChoice("Unite the street clans", 15, {stats(0, 0, 0, 40)});
    n8->addChoice("Demand protection taxes", 16, {stats(0, 400, 0, 10)});
    n8->addChoice("Guard the neighborhood", 19, {stats(10, 100, 0, 20)});
    engine.addNode(std::move(n8));

    auto n9 = node(9, "You escaped, but lost money and street respect.");
    n9->addChoice("Beg in the Sector 7 alleys", 17, {stats(-10, -20, 0, -10)});
    n9->addChoice("Steal from a market stall", 18, {stats(-50, 200, 0, -15)});
    n9->addChoice("Take a cleaning job at Megacorp", 7, {stats(0, 50, 0, 0)});
    engine.addNode(std::move(n9));

    auto n10 = node(10, "The bandits accept your Medkit and leave.");
    n10->addChoice("Look for work at the bar", 4, {});
    n10->addChoice("Search trash for a Cyber-Deck", 13, {item("Cyber-Deck")});
    n10->addChoice("Sleep on a park bench", 19, {stats(20, 0, 0, 0)});
    engine.addNode(std::move(n10));

    auto n11 = node(11, "You enter the corporate vault, but security turrets activate.");
    n11->addChoice("Disable the turrets with a Cyber-Deck", 14,
                   {stats(0, 500, 5, 15)}, "Cyber-Deck");
    n11->addChoice("Patch your wounds with a Medkit", 19, {stats(10, 0, 0, 0)}, "Medkit");
    n11->addChoice("Charge through the turret fire", 20, {stats(-100, 0, 0, 0)});
    engine.addNode(std::move(n11));

    auto n12 = node(12, "You work security shifts and save your money.");
    n12->addChoice("Buy the bar", 16, {stats(0, 300, 0, 20)});
    n12->addChoice("Live a peaceful street life", 19, {stats(10, 100, 0, 10)});
    n12->addChoice("Buy a Neural Chip", 2, {stats(0, -80, 15, 0), item("Neural Chip")});
    engine.addNode(std::move(n12));

    auto n13 = node(13, "You have a Cyber-Deck and access to high-tier networks.");
    n13->addChoice("Join the corporate heist", 11, {stats(0, 0, 0, 10)});
    n13->addChoice("Sell the deck to the Netrunners", 6, {stats(0, 300, 0, 5)});
    n13->addChoice("Study the deck's code", 5, {stats(0, 0, 15, 0)});
    engine.addNode(std::move(n13));

    auto n14 = node(14, "TECH BILLIONAIRE ENDING: luxury awaits above FAF City.");
    n14->addChoice("Accept your new life", 14, {ending("TECH BILLIONAIRE ENDING: You mastered the city.")});
    n14->addChoice("Celebrate with the citizens", 14, {ending("TECH BILLIONAIRE ENDING: You mastered the city.")});
    n14->addChoice("Close the story", 14, {ending("TECH BILLIONAIRE ENDING: You mastered the city.")});
    engine.addNode(std::move(n14));

    auto n15 = node(15, "CYBERPUNK LEGEND ENDING: Sector 7 follows your example.");
    n15->addChoice("Lead with honor", 15, {ending("CYBERPUNK LEGEND ENDING: You became a city legend.")});
    n15->addChoice("Protect the weak", 15, {ending("CYBERPUNK LEGEND ENDING: You became a city legend.")});
    n15->addChoice("Close the story", 15, {ending("CYBERPUNK LEGEND ENDING: You became a city legend.")});
    engine.addNode(std::move(n15));

    auto n16 = node(16, "CORPO CEO ENDING: you control Megacorp.");
    n16->addChoice("Sign the executive contract", 16, {ending("CORPO CEO ENDING: You control Megacorp.")});
    n16->addChoice("Reinvest in FAF City", 16, {ending("CORPO CEO ENDING: You control Megacorp.")});
    n16->addChoice("Close the story", 16, {ending("CORPO CEO ENDING: You control Megacorp.")});
    engine.addNode(std::move(n16));

    auto n17 = node(17, "BROKE ENDING: your money and opportunities are gone.");
    n17->addChoice("Accept the consequences", 17, {ending("BROKE ENDING: You survived, but lost everything.")});
    n17->addChoice("Start over in the alleys", 17, {ending("BROKE ENDING: You survived, but lost everything.")});
    n17->addChoice("Close the story", 17, {ending("BROKE ENDING: You survived, but lost everything.")});
    engine.addNode(std::move(n17));

    auto n18 = node(18, "BURNT-OUT ENDING: money arrived, but your health collapsed.");
    n18->addChoice("Rest at last", 18, {ending("BURNT-OUT ENDING: You traded your health for success.")});
    n18->addChoice("Leave the corporation", 18, {ending("BURNT-OUT ENDING: You traded your health for success.")});
    n18->addChoice("Close the story", 18, {ending("BURNT-OUT ENDING: You traded your health for success.")});
    engine.addNode(std::move(n18));

    auto n19 = node(19, "PERFECT ENDING: you balanced work, health, and wealth.");
    n19->addChoice("Enjoy a quiet evening", 19, {ending("PERFECT ENDING: You found balance in FAF City.")});
    n19->addChoice("Help another traveler", 19, {ending("PERFECT ENDING: You found balance in FAF City.")});
    n19->addChoice("Close the story", 19, {ending("PERFECT ENDING: You found balance in FAF City.")});
    engine.addNode(std::move(n19));

    auto n20 = node(20, "GAME OVER: automated turrets stop your escape.");
    n20->addChoice("Face the consequence", 20, {ending("GAME OVER: The turrets ended your journey.")});
    n20->addChoice("Remember the warning", 20, {ending("GAME OVER: The turrets ended your journey.")});
    n20->addChoice("Close the story", 20, {ending("GAME OVER: The turrets ended your journey.")});
    engine.addNode(std::move(n20));

    engine.start(1);
    return 0;
}
```

## 4. Conclusions

The project demonstrates how SOLID principles can be applied to a small but complete C++ game. SRP separates player state, story data, and execution flow. OCP makes choice behavior extensible through polymorphic action effects, so new effects can be introduced without changing the engine. DIP keeps the high-level game logic independent from terminal output and concrete effect implementations.

The resulting architecture is decoupled, testable, and maintainable. Ownership is explicit through `std::unique_ptr`, the story graph is easy to extend, and all nodes consistently expose three choices. This provides a stable foundation for future TMPS laboratories involving Design Patterns such as Factory Method, Command, State, Observer, and Strategy.
