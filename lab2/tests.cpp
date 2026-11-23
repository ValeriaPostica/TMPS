#include "Structural/EffectComposites.h"
#include "Structural/GameFacade.h"
#include "Structural/OutputDecorators.h"
#include "Creational/PlayerBuilder.h"
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
    istringstream input("3\n1\n3\n");
    ostringstream output;
    streambuf* previousInput = cin.rdbuf(input.rdbuf());
    streambuf* previousOutput = cout.rdbuf(output.rdbuf());
    facade.run(testPlayer());
    cout.rdbuf(previousOutput);
    cin.rdbuf(previousInput);
    require(output.str().find("[FAF CITY] Tester,") != string::npos,
            "Facade did not build the configured decorator chain.");
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

int main() {
    try {
        decoratorAddsResponsibility();
        decoratorsCanBeStacked();
        coloredDecoratorUsesMultipleGameColors();
        compositeAppliesLeafEffectsInOrder();
        compositeAndLeafShareTheSameInterface();
        facadeStartsTheConfiguredGame();
        facadeRunsAnotherSubsystemPath();
        cout << "All Lab 2 tests passed.\n";
        return 0;
    } catch (const exception& error) {
        cerr << "Test failure: " << error.what() << '\n';
        return 1;
    }
}
