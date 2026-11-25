#ifndef BEHAVIORAL_STATE_H
#define BEHAVIORAL_STATE_H
#include "../SOLID/StoryEntities.h"
#include "Observer.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

enum class PlayerStateType { Exploring, Cautious, Critical };

class IPlayerState {
public:
    virtual ~IPlayerState() = default;
    virtual PlayerStateType type() const = 0;
    virtual const char* name() const = 0;
    virtual bool canTransitionTo(PlayerStateType next) const = 0;
};

class ExploringState final : public IPlayerState {
public:
    PlayerStateType type() const override { return PlayerStateType::Exploring; }
    const char* name() const override { return "Exploring"; }
    bool canTransitionTo(PlayerStateType next) const override {
        return next == PlayerStateType::Cautious;
    }
};

class CautiousState final : public IPlayerState {
public:
    PlayerStateType type() const override { return PlayerStateType::Cautious; }
    const char* name() const override { return "Cautious"; }
    bool canTransitionTo(PlayerStateType next) const override {
        return next == PlayerStateType::Exploring ||
               next == PlayerStateType::Critical;
    }
};

class CriticalState final : public IPlayerState {
public:
    PlayerStateType type() const override { return PlayerStateType::Critical; }
    const char* name() const override { return "Critical"; }
    bool canTransitionTo(PlayerStateType next) const override {
        return next == PlayerStateType::Cautious;
    }
};

class PlayerStateContext : public IGameEventSubject {
private:
    Player& player_;
    std::unique_ptr<IPlayerState> state_;
    std::vector<IGameEventObserver*> observers_;

public:
    explicit PlayerStateContext(Player& player)
        : player_(player), state_(std::make_unique<ExploringState>()) {}

    IPlayerState& state() { return *state_; }
    const IPlayerState& state() const { return *state_; }

    bool setState(std::unique_ptr<IPlayerState> next) {
        if (!next || !state_->canTransitionTo(next->type())) return false;
        const char* previous = state_->name();
        state_ = std::move(next);
        notify({GameEventType::StateChanged, previous, state_->name()});
        return true;
    }

    void refreshFromPlayer() {
        const int health = player_.health();
        PlayerStateType target = health <= 20 ? PlayerStateType::Critical
                                 : health <= 50 ? PlayerStateType::Cautious
                                                : PlayerStateType::Exploring;
        if (target == state_->type()) return;

        if (target == PlayerStateType::Critical) {
            if (state_->type() == PlayerStateType::Exploring)
                setState(std::make_unique<CautiousState>());
            if (state_->type() == PlayerStateType::Cautious)
                setState(std::make_unique<CriticalState>());
        } else if (target == PlayerStateType::Cautious) {
            if (state_->type() == PlayerStateType::Critical)
                setState(std::make_unique<CautiousState>());
            else if (state_->type() == PlayerStateType::Exploring)
                setState(std::make_unique<CautiousState>());
        } else {
            if (state_->type() == PlayerStateType::Critical)
                setState(std::make_unique<CautiousState>());
            if (state_->type() == PlayerStateType::Cautious)
                setState(std::make_unique<ExploringState>());
        }
    }

    void attach(IGameEventObserver& observer) override {
        detach(observer);
        observers_.push_back(&observer);
    }

    void detach(IGameEventObserver& observer) override {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(),
                           [&observer](IGameEventObserver* item) {
                               return item == &observer;
                           }),
            observers_.end());
    }

    void notify(const GameEvent& event) override {
        for (auto* observer : observers_) observer->onGameEvent(event);
    }
};

#endif
