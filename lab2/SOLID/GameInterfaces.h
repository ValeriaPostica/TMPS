#ifndef GAME_INTERFACES_H
#define GAME_INTERFACES_H
#include <iostream>
#include <string>
using namespace std;

class Player;

class IActionEffect {
public:
    virtual ~IActionEffect() = default;
    virtual void apply(Player& player) = 0;
};

class IOutputWriter {
public:
    virtual ~IOutputWriter() = default;
    virtual void write(const string& message) = 0;
};

class ConsoleWriter final : public IOutputWriter {
public:
    void write(const string& message) override {
        cout << message << '\n';
    }
};

#endif
