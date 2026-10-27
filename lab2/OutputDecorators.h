#ifndef OUTPUT_DECORATORS_H
#define OUTPUT_DECORATORS_H
#include "GameInterfaces.h"
#include <iostream>
#include <string>
#include <utility>
using namespace std;

class PlayerNameOutputDecorator final : public IOutputWriter {
private:
    IOutputWriter& writer_;
    string playerName_;

public:
    PlayerNameOutputDecorator(IOutputWriter& writer, string playerName)
        : writer_(writer), playerName_(move(playerName)) {}

    void write(const string& message) override {
        writer_.write(playerName_ + ", " + message);
    }
};

#endif
