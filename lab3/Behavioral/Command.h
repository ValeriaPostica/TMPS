#ifndef BEHAVIORAL_COMMAND_H
#define BEHAVIORAL_COMMAND_H
#include "../SOLID/StoryEntities.h"
#include "../Structural/EffectComposites.h"
#include "State.h"
#include <memory>
#include <vector>

class IGameCommand {
public:
    virtual ~IGameCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};

class ApplyEffectCommand final : public IGameCommand {
private:
    Player& player_;
    std::shared_ptr<IActionEffect> effect_;
    PlayerStateContext& stateContext_;
    int& currentNodeId_;
    int previousNodeId_;
    int nextNodeId_;
    bool executed_ = false;
    Player::StateSnapshot snapshot_{};

public:
    ApplyEffectCommand(Player& player,
                       std::shared_ptr<IActionEffect> effect,
                       PlayerStateContext& stateContext,
                       int& currentNodeId,
                       int nextNodeId)
        : player_(player), effect_(std::move(effect)),
          stateContext_(stateContext), currentNodeId_(currentNodeId),
          previousNodeId_(currentNodeId), nextNodeId_(nextNodeId) {}

    void execute() override {
        snapshot_ = player_.saveState();
        if (effect_) effect_->apply(player_);
        stateContext_.refreshFromPlayer();
        currentNodeId_ = nextNodeId_;
        executed_ = true;
    }

    void undo() override {
        if (!executed_) return;
        player_.restoreState(snapshot_);
        currentNodeId_ = previousNodeId_;
        stateContext_.refreshFromPlayer();
        executed_ = false;
    }
};

class CommandInvoker {
private:
    std::vector<std::unique_ptr<IGameCommand>> history_;

public:
    void execute(std::unique_ptr<IGameCommand> command) {
        command->execute();
        history_.push_back(std::move(command));
    }

    bool undo() {
        if (history_.empty()) return false;
        history_.back()->undo();
        history_.pop_back();
        return true;
    }

    int size() const { return static_cast<int>(history_.size()); }
};

#endif
