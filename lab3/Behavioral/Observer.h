#ifndef BEHAVIORAL_OBSERVER_H
#define BEHAVIORAL_OBSERVER_H
#include "../SOLID/GameInterfaces.h"
#include <string>
#include <vector>

enum class GameEventType { StateChanged };

struct GameEvent {
    GameEventType type;
    std::string from;
    std::string to;
};

class IGameEventObserver {
public:
    virtual ~IGameEventObserver() = default;
    virtual void onGameEvent(const GameEvent& event) = 0;
};

class IGameEventSubject {
public:
    virtual ~IGameEventSubject() = default;
    virtual void attach(IGameEventObserver& observer) = 0;
    virtual void detach(IGameEventObserver& observer) = 0;
    virtual void notify(const GameEvent& event) = 0;
};

class OutputGameObserver final : public IGameEventObserver {
private:
    IOutputWriter& writer_;

public:
    explicit OutputGameObserver(IOutputWriter& writer) : writer_(writer) {}

    void onGameEvent(const GameEvent& event) override {
        if (event.type == GameEventType::StateChanged) {
            writer_.write("STATE CHANGE: " + event.from + " -> " + event.to);
        }
    }
};

class AchievementObserver final : public IGameEventObserver {
private:
    int criticalEntries_ = 0;
    int stateChanges_ = 0;

public:
    int criticalEntries() const { return criticalEntries_; }
    int stateChanges() const { return stateChanges_; }
    void onGameEvent(const GameEvent& event) override {
        if (event.type != GameEventType::StateChanged) return;
        ++stateChanges_;
        if (event.to == "Critical")
            ++criticalEntries_;
    }
};

#endif
