#ifndef STORY_LOADER_H
#define STORY_LOADER_H
#include "GameEngine.h"
#include "EffectFactories.h"
#include "GameConfig.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
using namespace std;

class StoryLoader {
private:
    using Json = nlohmann::json;

public:
    void load(const string& fileName, GameEngine& engine) const {
        ifstream file(fileName);
        if (!file) {
            throw runtime_error("Unable to open story file: " + fileName);
        }

        const Json document = Json::parse(file);
        const Json& nodes = document.at("nodes");
        for (const Json& nodeJson : nodes) {
            auto node = make_unique<StoryNode>(
                nodeJson.at("id").get<int>(),
                nodeJson.at("text").get<string>());

            const Json& choices = nodeJson.at("choices");
            if (choices.size() != static_cast<size_t>(GameConfig::instance().choicesPerNode())) {
                throw runtime_error("Every story node must contain exactly " +
                                    to_string(GameConfig::instance().choicesPerNode()) +
                                    " choices.");
            }

            for (const Json& choiceJson : choices) {
                vector<shared_ptr<IActionEffect>> effects;
                for (const Json& effectJson : choiceJson.value("effects", Json::array())) {
                    effects.push_back(createEffect(effectJson));
                }
                node->addChoice(
                    choiceJson.at("description").get<string>(),
                    choiceJson.at("nextNodeId").get<int>(),
                    move(effects),
                    choiceJson.value("requiredItem", ""),
                    choiceJson.value("requiredIntelligence", 0));
            }
            engine.addNode(move(node));
        }
    }
};

#endif
