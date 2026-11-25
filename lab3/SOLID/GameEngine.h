#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H
#include "StoryEntities.h"
#include "../Behavioral/Command.h"
#include "../Behavioral/Observer.h"
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
using namespace std;

class GameEngine {
private:
    Player& player_;
    IOutputWriter& writer_;
    map<int, unique_ptr<StoryNode>> storyGraph_;
    PlayerStateContext stateContext_;
    CommandInvoker commandInvoker_;
    int currentNodeId_ = 0;

public:
    GameEngine(Player& player, IOutputWriter& writer)
        : player_(player), writer_(writer), stateContext_(player) {}

    void addNode(unique_ptr<StoryNode> node) {
        storyGraph_[node->id()] = move(node);
    }

    void attachObserver(IGameEventObserver& observer) {
        stateContext_.attach(observer);
    }

    void detachObserver(IGameEventObserver& observer) {
        stateContext_.detach(observer);
    }

    const IPlayerState& currentState() const { return stateContext_.state(); }

    void start(int startNodeId) {
        writer_.write("FAF CITY: CHOOSE YOUR DESTINY");
        currentNodeId_ = startNodeId;
        bool endingDisplayed = false;

        while (true) {
            if (player_.isGameOver()) {
                writer_.write(player_.endingMessage());
                endingDisplayed = true;
                writer_.write("1. Undo last choice");
                writer_.write("2. Finish game");
                writer_.write(player_.name() + ", choose an option:");

                int endChoice = 0;
                if (!(cin >> endChoice)) {
                    if (cin.eof()) break;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    writer_.write("Please enter 1 to undo or 2 to finish.");
                    continue;
                }
                if (endChoice == 1) {
                    if (commandInvoker_.undo()) {
                        endingDisplayed = false;
                        writer_.write("The last choice was undone.");
                    } else {
                        writer_.write("There is no choice to undo.");
                    }
                    continue;
                }
                if (endChoice == 2) break;
                writer_.write("Invalid option. Choose 1 to undo or 2 to finish.");
                continue;
            }

            const auto nodeIt = storyGraph_.find(currentNodeId_);
            if (nodeIt == storyGraph_.end()) {
                writer_.write("Error: story node is missing.");
                break;
            }

            const StoryNode& node = *nodeIt->second;
            writer_.write(player_.statsMessage());
            writer_.write(node.text());
            const auto& choices = node.choices();
            for (size_t i = 0; i < choices.size(); ++i) {
                const Choice& choice = choices[i];
                ostringstream choiceMessage;
                choiceMessage << i + 1 << ". " << choice.description;
                if (!choice.requiredItem.empty()) {
                    choiceMessage << " (Requires: " << choice.requiredItem << ")";
                }
                if (choice.requiredIntelligence > 0) {
                    choiceMessage << " (Requires INT: " << choice.requiredIntelligence << ")";
                }
                writer_.write(choiceMessage.str());
            }
            writer_.write(to_string(choices.size() + 1) + ". Undo last choice");

            writer_.write(player_.name() + ", choose an option:");
            int userChoice = 0;
            if (!(cin >> userChoice)) {
                if (cin.eof()) break;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                writer_.write("Please enter a number from 1 to " +
                              to_string(choices.size() + 1) + ".");
                continue;
            }
            const int undoOption = static_cast<int>(choices.size()) + 1;
            if (userChoice == undoOption) {
                if (commandInvoker_.undo())
                    writer_.write("The last choice was undone.");
                else
                    writer_.write("There is no choice to undo.");
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

            commandInvoker_.execute(make_unique<ApplyEffectCommand>(
                player_, selected.effect, stateContext_,
                currentNodeId_, selected.nextNodeId));
        }

        if (!endingDisplayed) {
            writer_.write(player_.endingMessage().empty()
                              ? "THE END - Thanks for playing!"
                              : player_.endingMessage());
        }
    }
};

#endif
