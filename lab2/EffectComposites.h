#ifndef EFFECT_COMPOSITES_H
#define EFFECT_COMPOSITES_H
#include "StoryEntities.h"
#include <memory>
#include <utility>
#include <vector>
using namespace std;

class CompositeEffect final : public IActionEffect {
private:
    vector<shared_ptr<IActionEffect>> effects_;

public:
    void add(shared_ptr<IActionEffect> effect) {
        if (effect) {
            effects_.push_back(move(effect));
        }
    }

    void apply(Player& player) override {
        for (const auto& effect : effects_) {
            effect->apply(player);
        }
    }

    bool empty() const {
        return effects_.empty();
    }
};

#endif
