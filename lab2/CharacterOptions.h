#ifndef CHARACTER_OPTIONS_H
#define CHARACTER_OPTIONS_H
#include "PlayerBuilder.h"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

struct CharacterPreset {
    string name;
    string description;
    int health;
    int money;
    int intelligence;
    int reputation;
    vector<string> inventory;
};

inline const vector<CharacterPreset>& characterPresets() {
    static const vector<CharacterPreset> presets = {
        {"Richie", "Rich but not very intelligent.", 100, 1000, 5, 10, {}},
        {"Hacker", "Very intelligent but physically fragile.", 20, 150, 100, 5, {"Neural Chip"}},
        {"Survivor", "Poor, but healthy and street-smart.", 100, 20, 15, 20, {"Medkit"}}
    };
    return presets;
}

inline int readCharacterInteger(const string& prompt, int minimum, int maximum) {
    int value = 0;
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minimum && value <= maximum) {
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Value must be an integer between " << minimum
             << " and " << maximum << ". Please try again.\n";
    }
}

inline Player buildCustomCharacter() {
    string name;
    do {
        cout << "Enter your hero name: ";
        getline(cin >> ws, name);
        if (name.empty()) {
            cout << "The hero name cannot be empty. Please try again.\n";
        }
    } while (name.empty());

    return PlayerBuilder()
        .setName(name)
        .setHealth(readCharacterInteger("Enter starting health (0-100): ", 0, 100))
        .setMoney(readCharacterInteger("Enter starting money (0 or greater): ",
                                       0, 2147483647))
        .setIntelligence(readCharacterInteger(
            "Enter starting intelligence (0-100): ", 0, 100))
        .setReputation(readCharacterInteger(
            "Enter starting reputation (-100 to 100): ", -100, 100))
        .build();
}

inline Player buildSelectedCharacter() {
    const auto& presets = characterPresets();
    cout << "\nChoose a character:\n";
    for (size_t i = 0; i < presets.size(); ++i) {
        cout << i + 1 << ". " << presets[i].name
             << " - " << presets[i].description << '\n';
    }
    cout << presets.size() + 1 << ". Create your own character\n";

    int selection = 0;
    while (selection < 1 ||
           selection > static_cast<int>(presets.size()) + 1) {
        selection = readCharacterInteger(
            "Your choice: ", 1, static_cast<int>(presets.size()) + 1);
        if (selection < 1 ||
            selection > static_cast<int>(presets.size()) + 1) {
            cout << "Invalid character selection. Choose a displayed option.\n";
        }
    }
    if (selection == static_cast<int>(presets.size()) + 1) {
        return buildCustomCharacter();
    }

    const CharacterPreset& preset = presets[static_cast<size_t>(selection - 1)];
    PlayerBuilder builder;
    builder.setName(preset.name)
        .setHealth(preset.health)
        .setMoney(preset.money)
        .setIntelligence(preset.intelligence)
        .setReputation(preset.reputation);
    for (const auto& item : preset.inventory) {
        builder.addItem(item);
    }
    return builder.build();
}

#endif
