#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H
#include "StoryEntities.h"
#include <iostream>
#include <limits>
#include <map>
#include <memory>
using namespace std;

class GameEngine {
private:
    Player& player_;
    IOutputWriter& writer_;
    map<int, unique_ptr<StoryNode>> storyGraph_;

public:
    GameEngine(Player& player, IOutputWriter& writer)
        : player_(player), writer_(writer) {}

    void addNode(unique_ptr<StoryNode> node) {
        storyGraph_[node->id()] = move(node);
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
            const auto& choices = node.choices();
            for (size_t i = 0; i < choices.size(); ++i) {
                const Choice& choice = choices[i];
                cout << i + 1 << ". " << choice.description;
                if (!choice.requiredItem.empty()) {
                    cout << " (Requires: " << choice.requiredItem << ")";
                }
                if (choice.requiredIntelligence > 0) {
                    cout << " (Requires INT: " << choice.requiredIntelligence << ")";
                }
                cout << '\n';
            }

            cout << "\nChoose option: ";
            int userChoice = 0;
            if (!(cin >> userChoice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                writer_.write("Please enter a number from 1 to 3.");
                continue;
            }
            if (userChoice < 1 || userChoice > static_cast<int>(choices.size())) {
                writer_.write("Invalid option. Choose one of the displayed choices.");
                continue;
            }

            const Choice& selected = choices[static_cast<size_t>(userChoice - 1)];
            if (!selected.requiredItem.empty() && !player_.hasItem(selected.requiredItem)) {
                writer_.write("You do not have the required item: " + selected.requiredItem);
                continue;
            }
            if (player_.intelligence() < selected.requiredIntelligence) {
                writer_.write("Your Intelligence is too low for this choice.");
                continue;
            }

            for (const auto& effect : selected.effects) {
                effect->apply(player_);
            }
            currentNodeId = selected.nextNodeId;
        }

        writer_.write(player_.endingMessage().empty()
                          ? "THE END - Thanks for playing!"
                          : player_.endingMessage());
    }
};

#endif
