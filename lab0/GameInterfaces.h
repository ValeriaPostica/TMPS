#ifndef GAME_INTERFACES_H
#define GAME_INTERFACES_H

#include <iostream>
#include <string>

using namespace std;

class Player;

// 1. OCP: Interface for effects triggered by choices
class IActionEffect {
public:
    virtual ~IActionEffect() {}
    virtual void apply(Player& player) = 0;
};

// 3. DIP: Interface for console output
class IOutputWriter {
public:
    virtual ~IOutputWriter() {}
    virtual void write(string message) = 0;
};

class ConsoleWriter : public IOutputWriter {
public:
    void write(string message) {
        cout << message << "\n";
    }
};

#endif