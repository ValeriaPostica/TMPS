#include "StoryLoader.h"
#include "PlayerBuilder.h"
#include "GameConfig.h"
#include <iostream>
#include <limits>
#include <string>
using namespace std;

int readInteger(const string& prompt) {
    int value = 0;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            return value;
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Please enter a valid integer.\n";
    }
}

int main() {
    string name;
    cout << "Enter your hero name: ";
    getline(cin >> ws, name);

    const int health = readInteger("Enter starting health (0-100): ");
    const int money = readInteger("Enter starting money (0 or greater): ");
    const int intelligence = readInteger(
        "Enter starting intelligence (0-100): ");
    const int reputation = readInteger(
        "Enter starting reputation (-100 to 100): ");

    GameConfig& config = GameConfig::instance();
    config.setStoryFile("story.json");
    config.setStartNodeId(1);
    config.setChoicesPerNode(3);

    Player player = PlayerBuilder()
        .setName(name)
        .setHealth(health)
        .setMoney(money)
        .setIntelligence(intelligence)
        .setReputation(reputation)
        .build();
    ConsoleWriter writer;
    GameEngine engine(player, writer);
    StoryLoader loader;
    loader.load(config.storyFile(), engine);
    engine.start(config.startNodeId());
}
