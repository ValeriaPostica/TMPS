#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H
#include <stdexcept>
#include <string>
#include <utility>
using namespace std;

class GameConfig {
private:
    string storyFile_ = "story.json";
    int startNodeId_ = 1;
    int choicesPerNode_ = 3;

    GameConfig() = default;

public:
    GameConfig(const GameConfig&) = delete;
    GameConfig& operator=(const GameConfig&) = delete;

    static GameConfig& instance() {
        static GameConfig config;
        return config;
    }

    const string& storyFile() const { return storyFile_; }
    int startNodeId() const { return startNodeId_; }
    int choicesPerNode() const { return choicesPerNode_; }

    void setStoryFile(string storyFile) {
        if (storyFile.empty()) {
            throw invalid_argument("Story file name cannot be empty.");
        }
        storyFile_ = move(storyFile);
    }

    void setStartNodeId(int startNodeId) {
        startNodeId_ = startNodeId;
    }

    void setChoicesPerNode(int choicesPerNode) {
        if (choicesPerNode <= 0) {
            throw invalid_argument("The number of choices must be positive.");
        }
        choicesPerNode_ = choicesPerNode;
    }
};

#endif
