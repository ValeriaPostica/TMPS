#ifndef STORY_LOADER_H
#define STORY_LOADER_H
#include "GameEngine.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
using namespace std;

class StoryLoader {
private:
    using Json = nlohmann::json;

    static shared_ptr<IActionEffect> parseEffect(const Json& json) {
        const string type = json.at("type").get<string>();
        if (type == "stat") {
            return make_shared<StatEffect>(
                json.value("health", 0),
                json.value("money", 0),
                json.value("intelligence", 0),
                json.value("reputation", 0));
        }
        if (type == "item") {
            return make_shared<ItemEffect>(json.at("item").get<string>());
        }
        if (type == "ending") {
            return make_shared<EndingEffect>(json.at("message").get<string>());
        }
        throw runtime_error("Unsupported story effect type: " + type);
    }

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
            if (choices.size() != 3) {
                throw runtime_error("Every story node must contain exactly 3 choices.");
            }

            for (const Json& choiceJson : choices) {
                vector<shared_ptr<IActionEffect>> effects;
                for (const Json& effectJson : choiceJson.value("effects", Json::array())) {
                    effects.push_back(parseEffect(effectJson));
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
