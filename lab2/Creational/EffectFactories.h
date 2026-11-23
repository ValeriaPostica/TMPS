#ifndef EFFECT_FACTORIES_H
#define EFFECT_FACTORIES_H
#include "../SOLID/StoryEntities.h"
#include "../nlohmann/json.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
using namespace std;

class EffectCreator {
public:
    using Json = nlohmann::json;

    virtual ~EffectCreator() = default;
    virtual shared_ptr<IActionEffect> create(const Json& json) const = 0;
};

class StatEffectCreator final : public EffectCreator {
public:
    shared_ptr<IActionEffect> create(const Json& json) const override {
        return make_shared<StatEffect>(
            json.value("health", 0),
            json.value("money", 0),
            json.value("intelligence", 0),
            json.value("reputation", 0));
    }
};

class ItemEffectCreator final : public EffectCreator {
public:
    shared_ptr<IActionEffect> create(const Json& json) const override {
        return make_shared<ItemEffect>(json.at("item").get<string>());
    }
};

class EndingEffectCreator final : public EffectCreator {
public:
    shared_ptr<IActionEffect> create(const Json& json) const override {
        return make_shared<EndingEffect>(json.at("message").get<string>());
    }
};

class EffectFactory {
public:
    using Creator = shared_ptr<IActionEffect> (*)(const nlohmann::json&);

    static void registerCreator(string type, Creator creator) {
        creators()[move(type)] = creator;
    }

    static shared_ptr<IActionEffect> create(const nlohmann::json& json) {
        const string type = json.at("type").get<string>();
        const auto creator = creators().find(type);
        if (creator == creators().end()) {
            throw runtime_error("Unsupported story effect type: " + type);
        }
        return creator->second(json);
    }

private:
    static unordered_map<string, Creator>& creators() {
        static unordered_map<string, Creator> registered = {
            {"stat", [](const nlohmann::json& json) {
                return StatEffectCreator().create(json);
            }},
            {"item", [](const nlohmann::json& json) {
                return ItemEffectCreator().create(json);
            }},
            {"ending", [](const nlohmann::json& json) {
                return EndingEffectCreator().create(json);
            }}
        };
        return registered;
    }
};

inline shared_ptr<IActionEffect> createEffect(const nlohmann::json& json) {
    return EffectFactory::create(json);
}

#endif
