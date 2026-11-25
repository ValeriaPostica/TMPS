#include "Structural/EffectComposites.h"
#include "Structural/GameFacade.h"
#include "Structural/OutputDecorators.h"
#include "Creational/PlayerBuilder.h"
#include "Behavioral/Command.h"
#include "Behavioral/Observer.h"
#include "Behavioral/State.h"
#include "SOLID/GameEngine.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

class TestWriter final : public IOutputWriter {
public:
    vector<string> messages;

    void write(const string& message) override {
        messages.push_back(message);
    }
};

void require(bool condition, const string& message) {
    if (!condition) {
        throw runtime_error(message);
    }
}

Player testPlayer() {
    return PlayerBuilder()
        .setName("Tester")
        .setHealth(100)
        .setMoney(100)
        .setIntelligence(10)
        .setReputation(0)
        .build();
}

void decoratorAddsResponsibility() {
    TestWriter base;
    PlayerNameOutputDecorator decorated(base, "Tester");
    decorated.write("ready");
    require(base.messages.size() == 1, "Decorator did not delegate to the component.");
    require(base.messages[0] == "Tester, ready", "Decorator produced the wrong message.");
}

void decoratorsCanBeStacked() {
    TestWriter base;
    ColoredOutputDecorator colored(base, "\033[32m");
    MessagePrefixOutputDecorator prefixed(colored, "[GAME]");
    PlayerNameOutputDecorator named(prefixed, "Tester");
    prefixed.write("ready");
    require(base.messages.size() == 1, "Stacked decorators did not delegate.");
    require(base.messages[0] == "\033[32m[GAME] ready\033[0m",
            "Stacked decorators did not preserve both responsibilities.");
    named.write("ready");
    require(base.messages.size() == 2,
            "Three stacked decorators did not delegate.");
    require(base.messages[1] == "\033[32m[GAME] Tester, ready\033[0m",
            "Three stacked decorators did not preserve all responsibilities.");
}

void coloredDecoratorUsesMultipleGameColors() {
    TestWriter base;
    ColoredOutputDecorator colored(base, "\033[36m");

    colored.write("Player: Tester | HP: 100 | Money: $500 | INT: 10 | REP: 5 | Inventory: Empty");
    require(base.messages.size() == 1, "Colored output did not write one message.");
    require(base.messages[0].find("\033[33m") != string::npos &&
            base.messages[0].find("Money: $500") != string::npos,
            "Money was not highlighted with yellow.");
    require(base.messages[0].find("\033[31m") != string::npos,
            "Health was not highlighted with red.");

    colored.write("Invalid option. Choose one of the displayed choices.");
    require(base.messages[1].find("\033[31m") != string::npos &&
            base.messages[1].find("Invalid option") != string::npos,
            "Errors were not highlighted with red.");
}

void compositeAppliesLeafEffectsInOrder() {
    Player player = testPlayer();
    auto effects = make_shared<CompositeEffect>();
    effects->add(make_shared<StatEffect>(0, -30, 5, 0));
    effects->add(make_shared<ItemEffect>("Key"));
    effects->apply(player);
    require(player.money() == 70, "Composite did not apply the stat effect.");
    require(player.intelligence() == 15, "Composite did not apply all stat changes.");
    require(player.hasItem("Key"), "Composite did not apply the item effect.");
}

void compositeAndLeafShareTheSameInterface() {
    Player player = testPlayer();
    shared_ptr<IActionEffect> effect = make_shared<StatEffect>(-20, 0, 0, 0);
    effect->apply(player);
    require(player.health() == 80, "Leaf effect cannot be used through the component interface.");

    effect = make_shared<CompositeEffect>();
    auto composite = dynamic_pointer_cast<CompositeEffect>(effect);
    composite->add(make_shared<StatEffect>(-10, 0, 0, 0));
    effect->apply(player);
    require(player.health() == 70,
            "Composite cannot be used through the component interface.");
}

void facadeStartsTheConfiguredGame() {
    GameFacade facade;
    istringstream input("3\n1\n4\n3\n1\n3\n");
    ostringstream output;
    streambuf* previousInput = cin.rdbuf(input.rdbuf());
    streambuf* previousOutput = cout.rdbuf(output.rdbuf());
    facade.run(testPlayer());
    cout.rdbuf(previousOutput);
    cin.rdbuf(previousInput);
    require(output.str().find("[FAF CITY] FAF CITY: CHOOSE YOUR DESTINY") != string::npos,
            "Facade did not build the configured prefix and color decorators.");
    require(output.str().find("[FAF CITY] Tester, choose an option:") != string::npos,
            "Player name was not included in the choice prompt.");
    require(output.str().find("[FAF CITY] Tester, Player:") == string::npos,
            "Player name was unnecessarily prefixed to a non-prompt line.");
    require(output.str().find("STATE CHANGE: Exploring -> Cautious") != string::npos &&
            output.str().find("STATE CHANGE: Cautious -> Critical") != string::npos,
            "State transitions were not displayed in the game output.");
    require(output.str().find("GAME OVER") != string::npos,
            "Facade did not load and start the story.");
}

void facadeRunsAnotherSubsystemPath() {
    GameFacade facade;
    istringstream input("2\n2\n3\n1\n1\n");
    ostringstream output;
    streambuf* previous = cin.rdbuf(input.rdbuf());
    streambuf* previousOutput = cout.rdbuf(output.rdbuf());
    facade.run(testPlayer());
    cout.rdbuf(previousOutput);
    cin.rdbuf(previous);
    require(output.str().find("CORPO CEO ENDING") != string::npos,
            "Facade did not coordinate a second story path.");
}

void stateSupportsValidTransitions() {
    Player player = testPlayer();
    PlayerStateContext context(player);
    require(context.state().type() == PlayerStateType::Exploring,
            "Initial state must be Exploring.");
    require(context.setState(make_unique<CautiousState>()),
            "Exploring must allow transition to Cautious.");
    require(context.setState(make_unique<CriticalState>()),
            "Cautious must allow transition to Critical.");
    require(context.setState(make_unique<CautiousState>()),
            "Critical must allow recovery to Cautious.");
}

void stateRejectsForbiddenTransitions() {
    Player player = testPlayer();
    PlayerStateContext context(player);
    require(!context.setState(make_unique<CriticalState>()),
            "Exploring must not transition directly to Critical.");
    require(context.state().type() == PlayerStateType::Exploring,
            "Forbidden transition changed the state.");
}

void commandExecuteAndUndoRestoreSnapshot() {
    Player player = testPlayer();
    PlayerStateContext context(player);
    CommandInvoker invoker;
    int currentNodeId = 1;
    invoker.execute(make_unique<ApplyEffectCommand>(
        player, make_shared<StatEffect>(-20, -30, 5, 4),
        context, currentNodeId, 2));
    require(player.health() == 80 && player.money() == 70,
            "Command execute did not apply its effect.");
    require(currentNodeId == 2, "Command did not advance to the next story node.");
    require(invoker.undo(), "Command history did not undo a command.");
    require(player.health() == 100 && player.money() == 100 &&
            player.intelligence() == 10 && player.reputation() == 0,
            "Command undo did not restore the exact prior state.");
    require(currentNodeId == 1, "Command undo did not restore the previous story node.");
}

void gameUndoReturnsToThePreviousQuestion() {
    Player player = testPlayer();
    TestWriter writer;
    GameEngine engine(player, writer);

    auto firstNode = make_unique<StoryNode>(1, "First question");
    firstNode->addChoice("First path", 2, make_shared<StatEffect>(0, -10, 0, 0));
    firstNode->addChoice("Second path", 2, make_shared<StatEffect>(0, -20, 0, 0));
    firstNode->addChoice("Third path", 2);
    engine.addNode(move(firstNode));

    auto secondNode = make_unique<StoryNode>(2, "Second question");
    secondNode->addChoice("Finish", 3, make_shared<EndingEffect>("Finished"));
    secondNode->addChoice("Finish another way", 3, make_shared<EndingEffect>("Finished"));
    secondNode->addChoice("Finish once more", 3, make_shared<EndingEffect>("Finished"));
    engine.addNode(move(secondNode));

    istringstream input("1\n4\n2\n4\n3\n1\n");
    streambuf* previousInput = cin.rdbuf(input.rdbuf());
    engine.start(1);
    cin.rdbuf(previousInput);

    require(count(writer.messages.begin(), writer.messages.end(), "First question") == 3,
            "Undo did not display the previous question again after each choice.");
    require(count(writer.messages.begin(), writer.messages.end(), "Second question") == 3,
            "Undo did not return to the previous story node after each choice.");
    require(player.money() == 100,
            "Repeated undo did not restore each choice's player effects.");
}

void terminalChoiceCanBeUndoneBeforeFinishing() {
    Player player = testPlayer();
    TestWriter writer;
    GameEngine engine(player, writer);

    auto firstNode = make_unique<StoryNode>(1, "First question");
    firstNode->addChoice("Go to ending", 2);
    firstNode->addChoice("Alternative path", 2);
    firstNode->addChoice("Another path", 2);
    engine.addNode(move(firstNode));

    auto endingNode = make_unique<StoryNode>(2, "Ending question");
    endingNode->addChoice("End one", 2, make_shared<EndingEffect>("Ending reached"));
    endingNode->addChoice("End two", 2, make_shared<EndingEffect>("Ending reached"));
    endingNode->addChoice("End three", 2, make_shared<EndingEffect>("Ending reached"));
    engine.addNode(move(endingNode));

    istringstream input("1\n1\n1\n3\n2\n");
    streambuf* previousInput = cin.rdbuf(input.rdbuf());
    engine.start(1);
    cin.rdbuf(previousInput);

    require(count(writer.messages.begin(), writer.messages.end(), "Ending question") == 2,
            "Undo after a terminal choice did not return to its question.");
    require(count(writer.messages.begin(), writer.messages.end(), "Ending reached") == 2,
            "Undo did not clear the terminal result before the next ending.");
    require(player.isGameOver(),
            "Finishing after undoing a terminal choice did not end the game.");
}

void mementoUndoPreservesInventoryAndEnding() {
    Player player = testPlayer();
    player.addItem("Key");
    Player::StateSnapshot snapshot = player.saveState();
    player.modifyHealth(-100);
    player.addItem("Forbidden");
    player.restoreState(snapshot);
    require(player.health() == 100, "Memento did not restore health.");
    require(player.hasItem("Key"), "Memento did not restore inventory.");
    require(!player.hasItem("Forbidden"), "Memento retained a later inventory item.");
    require(!player.isGameOver(), "Memento did not restore game-over state.");
}

void observerNotifiesSubscribersAndSupportsUnsubscribe() {
    Player player = testPlayer();
    PlayerStateContext context(player);
    AchievementObserver first;
    AchievementObserver second;
    context.attach(first);
    context.attach(second);
    context.setState(make_unique<CautiousState>());
    context.detach(second);
    context.setState(make_unique<CriticalState>());
    require(first.stateChanges() == 2,
            "Attached observer was not notified on both state changes.");
    require(second.stateChanges() == 1,
            "Unsubscribed observer still received notifications.");
}

void observerReceivesCriticalStateChange() {
    Player player = testPlayer();
    PlayerStateContext context(player);
    AchievementObserver observer;
    context.attach(observer);
    context.setState(make_unique<CautiousState>());
    context.setState(make_unique<CriticalState>());
    require(observer.criticalEntries() == 1,
            "Observer did not report entry into the Critical state.");
}

int main() {
    try {
        decoratorAddsResponsibility();
        decoratorsCanBeStacked();
        coloredDecoratorUsesMultipleGameColors();
        compositeAppliesLeafEffectsInOrder();
        compositeAndLeafShareTheSameInterface();
        facadeStartsTheConfiguredGame();
        facadeRunsAnotherSubsystemPath();
        stateSupportsValidTransitions();
        stateRejectsForbiddenTransitions();
        commandExecuteAndUndoRestoreSnapshot();
        gameUndoReturnsToThePreviousQuestion();
        terminalChoiceCanBeUndoneBeforeFinishing();
        mementoUndoPreservesInventoryAndEnding();
        observerNotifiesSubscribersAndSupportsUnsubscribe();
        observerReceivesCriticalStateChange();
        cout << "All Lab 3 tests passed.\n";
        return 0;
    } catch (const exception& error) {
        cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
}
