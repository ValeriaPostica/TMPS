#ifndef EFFECT_FACTORIES_H
#define EFFECT_FACTORIES_H
#include "StoryEntities.h"
#include "nlohmann/json.hpp"
#include <memory>
#include <stdexcept>
#include <string>
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

inline shared_ptr<IActionEffect> createEffect(const nlohmann::json& json) {
    const string type = json.at("type").get<string>();
    if (type == "stat") {
        return StatEffectCreator().create(json);
    }
    if (type == "item") {
        return ItemEffectCreator().create(json);
    }
    if (type == "ending") {
        return EndingEffectCreator().create(json);
    }
    throw runtime_error("Unsupported story effect type: " + type);
}

#endif
