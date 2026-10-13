# Laboratory Work No. 0

**Course:** TMPS (Software Modeling and Design Techniques)  
**Topic:** FAF City: Choose Your Destiny  
**Language:** C++

## 1. Introduction

**FAF City: Choose Your Destiny** is a terminal-based *Choose Your Own Adventure* game written in C++. The player explores a futuristic FAF City through 20 interconnected story nodes. Each node provides exactly three choices. Every decision can change the player's state, add an item to the inventory, require a statistic or item, or lead to another part of the story.

The player has four main statistics:

- **Health** - physical condition; reaching zero ends the game.
- **Money** - funds used by financial decisions.
- **Intelligence** - enables technically difficult choices.
- **Reputation** - reflects the player's status in the city.

The inventory contains items such as the Neural Chip, Medkit, Stun Gun, and Cyber-Deck. The `GameEngine` validates requirements, applies the effects of a selected choice, and follows the corresponding connection in the story graph.

## 2. SOLID Principles Applied

### 2.1 Single Responsibility Principle (SRP)

Each class has one primary reason to change:

- `Player` manages only hero data: statistics, inventory, and ending state.
- `StoryNode` manages only narrative text and its three `Choice` objects.
- `StoryLoader` manages only external-data loading and conversion from JSON into domain objects.
- `GameEngine` manages only execution: input, navigation, requirement checks, and effect application.

For example, changing how Health is stored affects `Player`, while changing the JSON story content affects `story.json` and `StoryLoader`; neither change requires rewriting the game loop.

### 2.2 Open/Closed Principle (OCP)

Choice actions are represented by the `IActionEffect` abstraction. Existing effects include `StatEffect`, `ItemEffect`, and `EndingEffect`. The engine only calls `apply()` and does not need to know the concrete type.

Therefore, a new effect such as `ReputationEffect` or `UnlockAchievementEffect` can be added as another `IActionEffect` implementation without modifying `GameEngine`, `Choice`, or `StoryNode`.

### 2.3 Dependency Inversion Principle (DIP)

`GameEngine` depends on the `IOutputWriter` abstraction rather than directly depending on terminal output. `ConsoleWriter` is injected into the engine in `main()`. The same approach is used for effects: the engine stores and executes `IActionEffect` objects rather than depending on concrete effect classes.

This makes it possible to replace console output with a test writer, file writer, or graphical adapter without changing the game-flow code.

## 3. Source Code Structure and Representative Code

The implementation is divided into three files:

```text
lab0/
├── story.json         # narrative data, choices, requirements, and effects
├── GameInterfaces.h   # abstractions for effects and output
├── StoryEntities.h    # player, effects, choices, and story nodes
├── GameEngine.h       # game execution and story navigation
├── StoryLoader.h      # JSON parsing and graph construction
└── main.cpp           # minimal program entry point
```

The story content is no longer embedded in C++ source code. `story.json` is read at startup, and `StoryLoader` constructs and registers the graph automatically. The report includes only representative code sections that demonstrate the design.

### 3.1 `GameInterfaces.h` - abstractions for OCP and DIP

```cpp
class Player;

class IActionEffect {
public:
    virtual ~IActionEffect() = default;
    virtual void apply(Player& player) = 0;
};

class IOutputWriter {
public:
    virtual ~IOutputWriter() = default;
    virtual void write(const std::string& message) = 0;
};

class ConsoleWriter final : public IOutputWriter {
public:
    void write(const std::string& message) override {
        std::cout << message << '\n';
    }
};
```

**How the principles are used:** `IActionEffect` is the extension point required by OCP. The engine can execute any future effect through `apply()` without adding type-specific conditions. `IOutputWriter` is the DIP abstraction; `ConsoleWriter` is only one replaceable implementation of output.

### 3.2 `StoryEntities.h` - SRP and polymorphic effects

```cpp
class Player {
private:
    int health_ = 100;
    int money_ = 100;
    int intelligence_ = 10;
    int reputation_ = 0;
    std::vector<std::string> inventory_;
    bool gameOver_ = false;

public:
    void modifyHealth(int amount);
    void modifyMoney(int amount);
    void modifyIntelligence(int amount);
    void modifyReputation(int amount);
    void addItem(const std::string& item);
    bool hasItem(const std::string& item) const;
};
```

**How the principle is used:** `Player` has the single responsibility of maintaining hero state. It does not display story nodes, read input, or select the next node. Its methods provide controlled operations for effects and the engine.

```cpp
class StatEffect final : public IActionEffect {
public:
    StatEffect(int health, int money, int intelligence, int reputation);
    void apply(Player& player) override {
        player.modifyHealth(health_);
        player.modifyMoney(money_);
        player.modifyIntelligence(intelligence_);
        player.modifyReputation(reputation_);
    }
};

class ItemEffect final : public IActionEffect {
public:
    explicit ItemEffect(std::string item);
    void apply(Player& player) override {
        player.addItem(item_);
    }
};

class EndingEffect final : public IActionEffect {
public:
    explicit EndingEffect(std::string message);
    void apply(Player& player) override {
        player.finish(message_);
    }
};
```

**How the principle is used:** these classes extend `IActionEffect` with separate behaviors. `StatEffect` changes numbers, `ItemEffect` changes inventory, and `EndingEffect` finishes the story. Each effect has one focused responsibility, while the shared interface keeps the design open for additional effect types.

```cpp
struct Choice {
    std::string description;
    int nextNodeId;
    std::string requiredItem;
    int requiredIntelligence;
    std::vector<std::shared_ptr<IActionEffect>> effects;
};

class StoryNode {
private:
    int id_;
    std::string text_;
    std::vector<Choice> choices_;

public:
    void addChoice(std::string description, int nextNodeId,
                   std::vector<std::shared_ptr<IActionEffect>> effects = {},
                   std::string requiredItem = "",
                   int requiredIntelligence = 0);
};
```

**How the principle is used:** `Choice` stores destination and requirements, while `StoryNode` stores narrative data and choices. Neither class runs the game loop or directly applies effects. `StoryLoader` fills these objects from JSON, so story changes do not require C++ source changes.

### 3.3 `GameEngine.h` - execution and DIP injection

```cpp
class GameEngine {
private:
    Player& player_;
    IOutputWriter& writer_;
    std::map<int, std::unique_ptr<StoryNode>> storyGraph_;

public:
    GameEngine(Player& player, IOutputWriter& writer)
        : player_(player), writer_(writer) {}
};
```

**How the principles are used:** `GameEngine` is responsible for coordinating the game, but it receives `IOutputWriter` through its constructor. This is dependency injection and applies DIP: the high-level engine depends on an abstraction, not on `ConsoleWriter`. The engine does not know whether nodes came from JSON, a database, or another source.

```cpp
for (const auto& effect : choice.effects) {
    effect->apply(player_);
}
currentNodeId = choice.nextNodeId;
```

**How the principles are used:** the engine applies effects polymorphically. It does not contain `if` or `switch` statements for `StatEffect`, `ItemEffect`, or `EndingEffect`, which keeps the game-flow logic closed for modification and supports OCP.

### 3.4 `StoryLoader.h` - SRP and OCP at the data boundary

```cpp
class StoryLoader {
public:
    void load(const std::string& fileName, GameEngine& engine) const {
        std::ifstream file(fileName);
        const nlohmann::json document = nlohmann::json::parse(file);

        for (const auto& nodeJson : document.at("nodes")) {
            auto node = std::make_unique<StoryNode>(
                nodeJson.at("id").get<int>(),
                nodeJson.at("text").get<std::string>());
            // Parse three choices, requirements, and polymorphic effects.
            engine.addNode(std::move(node));
        }
    }
};
```

**How the principle is used:** `StoryLoader` has one responsibility: translate external JSON into `StoryNode`, `Choice`, and `IActionEffect` objects. The loader selects `StatEffect`, `ItemEffect`, or `EndingEffect` based on the JSON `"type"` field. Adding a new effect type extends the loader's factory logic while the game engine remains unchanged.

### 3.5 `story.json` - external story data

```json
{
  "startNodeId": 1,
  "nodes": [
    {
      "id": 1,
      "text": "You wake up in a rainy alley in Sector 7.",
      "choices": [
        {
          "description": "Buy a Neural Chip ($80)",
          "nextNodeId": 2,
          "effects": [
            { "type": "stat", "money": -80, "intelligence": 15 },
            { "type": "item", "item": "Neural Chip" }
          ]
        }
      ]
    }
  ]
}
```

The actual file contains all 20 nodes and all three choices for every node. Narrative text, links, requirements, and effect values are now data rather than compiled logic. A content author can rebalance the game or add story branches without editing `main.cpp`.

### 3.6 `main.cpp` - minimal composition root

```cpp
int main() {
    std::string name;
    std::cin >> name;
    Player player(name);
    ConsoleWriter writer;
    GameEngine engine(player, writer);
    StoryLoader loader;
    loader.load("story.json", engine);
    engine.start(1);
}
```

**How the principles are used:** `main()` only composes the application. It creates the concrete dependencies, asks `StoryLoader` to populate the engine, and starts execution. It contains no story-node definitions or effect-specific logic.

## 4. Conclusions

The project uses SRP to separate player state, story data, JSON loading, effects, and game execution. OCP is achieved through the `IActionEffect` hierarchy, allowing new choice behaviors without modifying the game engine. DIP is achieved by injecting `IOutputWriter` and storing dependencies through abstractions.

Using these principles creates a decoupled and maintainable architecture. The complete story can grow independently from the engine because it is stored in `story.json`, output can be replaced for testing or another interface, and future TMPS laboratories can introduce patterns such as Factory Method, Command, State, Observer, or Strategy without restructuring the core design.
