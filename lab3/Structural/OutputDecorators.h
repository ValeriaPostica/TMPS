#ifndef OUTPUT_DECORATORS_H
#define OUTPUT_DECORATORS_H
#include "../SOLID/GameInterfaces.h"
#include <string>
#include <utility>
using namespace std;

class OutputDecorator : public IOutputWriter {
protected:
    IOutputWriter& writer_;

    explicit OutputDecorator(IOutputWriter& writer) : writer_(writer) {}
};

class PlayerNameOutputDecorator final : public OutputDecorator {
private:
    string playerName_;

public:
    PlayerNameOutputDecorator(IOutputWriter& writer, string playerName)
        : OutputDecorator(writer), playerName_(move(playerName)) {}

    void write(const string& message) override {
        writer_.write(playerName_ + ", " + message);
    }
};

class MessagePrefixOutputDecorator final : public OutputDecorator {
private:
    string prefix_;

public:
    MessagePrefixOutputDecorator(IOutputWriter& writer, string prefix)
        : OutputDecorator(writer), prefix_(move(prefix)) {}

    void write(const string& message) override {
        writer_.write(prefix_ + " " + message);
    }
};

class ColoredOutputDecorator final : public OutputDecorator {
private:
    string colorCode_;

    static string highlightStats(const string& message) {
        constexpr const char* reset = "\033[0m";
        constexpr const char* cyan = "\033[36m";
        constexpr const char* yellow = "\033[33m";
        constexpr const char* green = "\033[32m";
        constexpr const char* red = "\033[31m";
        constexpr const char* blue = "\033[34m";
        constexpr const char* magenta = "\033[35m";

        string result;
        size_t start = 0;
        while (start < message.size()) {
            size_t separator = message.find('|', start);
            size_t end = separator == string::npos ? message.size() : separator;
            string segment = message.substr(start, end - start);

            const char* segmentColor = cyan;
            if (segment.find("HP:") != string::npos) {
                segmentColor = red;
            } else if (segment.find("Money:") != string::npos) {
                segmentColor = yellow;
            } else if (segment.find("INT:") != string::npos) {
                segmentColor = blue;
            } else if (segment.find("REP:") != string::npos) {
                segmentColor = green;
            } else if (segment.find("Inventory:") != string::npos) {
                segmentColor = magenta;
            }

            result += segmentColor;
            result += segment;
            result += reset;
            if (separator != string::npos) {
                result += " |";
                start = separator + 1;
            } else {
                break;
            }
        }
        return result;
    }

    string categoryColor(const string& message) const {
        if (message.find("Player:") != string::npos) {
            return highlightStats(message);
        }
        if (message.find("Error:") != string::npos ||
            message.find("Invalid") != string::npos ||
            message.find("too low") != string::npos ||
            message.find("do not have") != string::npos ||
            message.find("Please enter") != string::npos ||
            message.find("Input ended") != string::npos) {
            return "\033[31m" + message + "\033[0m";
        }
        if (message.find("GAME OVER") != string::npos ||
            message.find("ENDING") != string::npos ||
            message.find("THE END") != string::npos ||
            message.find("YOU DIED") != string::npos) {
            return "\033[35m" + message + "\033[0m";
        }
        if (message.find("choose an option") != string::npos ||
            message.find("Choose") != string::npos ||
            message.find("Accept") != string::npos) {
            return "\033[32m" + message + "\033[0m";
        }
        if (message.find("FAF CITY") != string::npos ||
            message.find("wake up") != string::npos ||
            message.find("At the") != string::npos ||
            message.find("You ") == 0) {
            return "\033[36m" + message + "\033[0m";
        }
        for (char digit = '1'; digit <= '9'; ++digit) {
            if (message.find(string(1, digit) + ". ") != string::npos) {
                return "\033[34m" + message + "\033[0m";
            }
        }
        if (!message.empty() && message[0] >= '1' && message[0] <= '9') {
            return "\033[34m" + message + "\033[0m";
        }
        return colorCode_ + message + "\033[0m";
    }

public:
    ColoredOutputDecorator(IOutputWriter& writer, const string& colorCode)
        : OutputDecorator(writer), colorCode_(colorCode) {}

    void write(const string& message) override {
        writer_.write(categoryColor(message));
    }
};

#endif
