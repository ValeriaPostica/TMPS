# Laboratory Work No. 2

**Course:** TMPS (Software Modeling and Design Techniques)  
**Topic:** SOLID Principles and Design Patterns in FAF City: Choose Your Destiny  
**Language:** C++17

## 1. Objective

This laboratory continues the adventure game from Laboratory Works No. 0 and
No. 1. It organizes the implementation around three SOLID principles and
keeps three creational patterns:

- Builder creates and validates `Player` objects;
- Factory Method creates action effects from JSON;
- Singleton stores application configuration.

The laboratory also adds three structural design patterns:

1. Decorator;
2. Composite;
3. Facade.

The game also supports selecting a predefined character or creating a custom
character.

Custom numeric input is validated before it is accepted: health and
intelligence must be between 0 and 100, money must be non-negative, and
reputation must be between -100 and 100. Invalid values are rejected and the
same prompt is displayed again. Game messages and choice prompts address the
player by the selected or custom name.

## 2. Project structure

```text
lab2/
├── story.json              # story nodes, choices, requirements, and effects
├── nlohmann/json.hpp       # JSON library
├── SOLID/
│   ├── GameInterfaces.h    # abstractions for OCP and DIP
│   ├── StoryEntities.h     # domain objects and effect implementations
│   └── GameEngine.h        # game loop using abstractions
├── CharacterOptions.h      # preset and custom character creation
├── StoryLoader.h           # JSON loading and effect composition
├── Creational/
│   ├── PlayerBuilder.h     # Lab 1 Builder
│   ├── EffectFactories.h   # Lab 1 Factory Method
│   └── GameConfig.h        # Lab 1 Singleton
├── Structural/
│   ├── OutputDecorators.h  # Decorator implementation
│   ├── EffectComposites.h  # Composite implementation
│   └── GameFacade.h        # Facade implementation
├── main.cpp                # input and application entry point
├── tests.cpp               # unit tests
├── run-tests.bat           # one-command test build and run
├── README.md               # build, run, and test instructions
└── REPORT.md               # laboratory report
```

## 3. Domain description and existing architecture

The game is a terminal-based choose-your-own-adventure application. The
`Player` stores health, money, intelligence, reputation, inventory, and ending
state. `StoryNode` stores narrative text and choices. `StoryLoader` reads the
story graph from JSON, while `GameEngine` validates choices, applies effects,
and navigates to the next node.

The player explores FAF City, makes choices, gains or loses resources, and
reaches one of several endings. The application keeps the story data separate
from the game engine so new story nodes and effects can be added independently.

The important extension points from the previous laboratories are:

```cpp
class IActionEffect {
public:
    virtual ~IActionEffect() = default;
    virtual void apply(Player& player) = 0;
};

class IOutputWriter {
public:
    virtual ~IOutputWriter() = default;
    virtual void write(const string& message) = 0;
};
```

The Lab 2 structural patterns reuse these abstractions instead of introducing
a separate game-flow architecture.

## 4. Character selection and creation

### 4.1 Motivation

Previously, `main()` always requested all player statistics manually. Lab 2
adds three predefined character configurations and a custom option:

| Character | Health | Money | Intelligence | Reputation | Description |
|---|---:|---:|---:|---:|---|
| The Tycoon | 100 | 1000 | 5 | 10 | Rich but not very intelligent |
| The Hacker | 20 | 150 | 100 | 5 | Very intelligent but physically fragile |
| The Survivor | 100 | 20 | 15 | 20 | Poor, but healthy and street-smart |
| Custom | User input | User input | User input | User input | Created by the player |

### 4.2 Implementation

`CharacterOptions.h` stores immutable preset data in `CharacterPreset`.
`buildSelectedCharacter()` displays the menu. A selected preset is translated
into the existing `PlayerBuilder`, so validation and construction remain in one
place. The custom branch also uses the same builder.

```cpp
const CharacterPreset& preset = presets[selection - 1];
PlayerBuilder builder;
builder.setName(preset.name)
    .setHealth(preset.health)
    .setMoney(preset.money)
    .setIntelligence(preset.intelligence)
    .setReputation(preset.reputation);
```

This avoids duplicating the `Player` construction logic and preserves the Lab 1
Builder pattern.

## 5. Decorator pattern

### 5.1 Intent

Decorator adds responsibilities to an object dynamically while preserving its
interface.

### 5.2 Problem

The game engine writes messages through `IOutputWriter`. The application needs
to add the selected player's name to every game message, but this formatting
should not be placed inside `GameEngine` or duplicated in every output
implementation.

### 5.3 Implementation

`OutputDecorator` stores the wrapped `IOutputWriter`. The concrete decorators
preserve the same interface, add their responsibility, and delegate to the
wrapped writer:

```cpp
class OutputDecorator : public IOutputWriter {
protected:
    IOutputWriter& writer_;

    explicit OutputDecorator(IOutputWriter& writer) : writer_(writer) {}
};

class PlayerNameOutputDecorator final : public OutputDecorator {
private:
    string playerName_;

public:
    PlayerNameOutputDecorator(IOutputWriter& writer, string playerName)
        : OutputDecorator(writer), playerName_(move(playerName)) {}

    void write(const string& message) override {
        writer_.write(playerName_ + ", " + message);
    }
};
```

Source: [OutputDecorators.h](Structural/OutputDecorators.h)

### 5.4 Participants

| Participant | Class |
|---|---|
| Component | `IOutputWriter` |
| Concrete component | `ConsoleWriter` |
| Decorator | `OutputDecorator` |
| Concrete decorators | `PlayerNameOutputDecorator`, `MessagePrefixOutputDecorator`, `ColoredOutputDecorator` |
| Client | `GameEngine` through `GameFacade` |

### 5.5 Usage

```cpp
ConsoleWriter consoleWriter;
ColoredOutputDecorator coloredWriter(consoleWriter, "\033[36m");
MessagePrefixOutputDecorator prefixedWriter(coloredWriter, "[FAF CITY]");
PlayerNameOutputDecorator writer(prefixedWriter, player.name());
GameEngine engine(player, writer);
```

The engine only knows `IOutputWriter`. `ColoredOutputDecorator` recognizes
game message categories and adds appropriate ANSI colors: player statistics
use separate colors for health, money, intelligence, reputation and inventory;
errors are red, endings are magenta, choices are blue, prompts are green, and
narrative messages are cyan. Additional decorators can be added later for
timestamps or file logging without changing the engine.

The concrete implementation is:

```cpp
class ColoredOutputDecorator final : public OutputDecorator {
private:
    string colorCode_;

public:
    ColoredOutputDecorator(IOutputWriter& writer, const string& colorCode)
        : OutputDecorator(writer), colorCode_(colorCode) {}

    void write(const string& message) override {
        writer_.write(categoryColor(message));
    }
};
```

Passing `"\033[36m"` as `colorCode_` creates the default cyan output. The
decorator is message-aware: the `Money: $500` segment is wrapped with
`"\033[33m"` (yellow), while `HP`, `INT`, `REP`, and `Inventory` receive their
own colors. `"\033[0m"` restores the terminal's default style.

| Message category | ANSI color |
|---|---|
| Title and narrative | Cyan |
| Choices and menu options | Blue |
| Prompts and successful actions | Green |
| Health | Red |
| Money | Yellow |
| Intelligence | Blue |
| Reputation | Green |
| Inventory | Magenta |
| Errors and invalid choices | Red |
| Endings and game-over messages | Magenta |

### 5.6 Benefits and trade-offs

The decorator keeps formatting separate from game logic, preserves the output
interface, and allows responsibilities to be composed. The trade-off is an
additional wrapper object and an extra delegation call.

## 6. Composite pattern

### 6.1 Intent

Composite composes objects into tree structures and lets clients treat an
individual object and a group of objects uniformly.

### 6.2 Problem

A story choice may contain several effects. For example, buying a Neural Chip
both changes statistics and adds an item:

```json
"effects": [
  {"type": "stat", "money": -80, "intelligence": 15},
  {"type": "item", "item": "Neural Chip"}
]
```

The effects should be applied as one choice action.

### 6.3 Implementation

`CompositeEffect` implements the same `IActionEffect` interface as every leaf
effect:

```cpp
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
};
```

Source: [EffectComposites.h](Structural/EffectComposites.h)

During JSON loading, all effects belonging to a choice are placed in one
composite. The `Choice` stores one `shared_ptr<IActionEffect>`, which can be a
leaf or a composite. The engine only performs:

```cpp
if (selected.effect) {
    selected.effect->apply(player_);
}
```

### 6.4 Participants

| Participant | Class |
|---|---|
| Component | `IActionEffect` |
| Leaf components | `StatEffect`, `ItemEffect`, `EndingEffect` |
| Composite | `CompositeEffect` |
| Client | `StoryLoader` and `GameEngine` |

### 6.5 Object structure

```text
CompositeEffect
├── StatEffect
└── ItemEffect
```

This makes the loader and engine independent of the number and concrete types
of effects in a choice.

### 6.6 Benefits and trade-offs

Composite makes grouped actions easy to extend and keeps the engine simple.
The trade-off is that an object representing one effect can also contain many
effects, so the object structure must be documented clearly.

## 7. Facade pattern

### 7.1 Intent

Facade provides a simple interface to a complex subsystem.

### 7.2 Problem

Without a facade, `main()` must know the order of multiple operations:

1. configure `GameConfig`;
2. create the player;
3. create the output implementation;
4. construct `GameEngine`;
5. load the story;
6. start the engine.

### 7.3 Implementation

``GameFacade::run()` coordinates the subsystem classes:

```cpp
class GameFacade {
public:
    void run(Player player) const {
        GameConfig& config = GameConfig::instance();
        config.setStoryFile("story.json");
        config.setStartNodeId(1);
        config.setChoicesPerNode(3);

        ConsoleWriter consoleWriter;
        ColoredOutputDecorator coloredWriter(consoleWriter, "\033[36m");
        MessagePrefixOutputDecorator prefixedWriter(coloredWriter, "[FAF CITY]");
        PlayerNameOutputDecorator writer(prefixedWriter, player.name());
        GameEngine engine(player, writer);
        StoryLoader loader;
        loader.load(config.storyFile(), engine);
        engine.start(config.startNodeId());
    }
};
```

Source: [GameFacade.h](Structural/GameFacade.h)

The entry point only selects or creates a player and starts the facade:

```cpp
Player player = buildSelectedCharacter();
GameFacade game;
game.run(move(player));
```

### 7.4 Participants

| Participant | Class |
|---|---|
| Facade | `GameFacade` |
| Subsystems | `GameConfig`, `StoryLoader`, `GameEngine`, `MessagePrefixOutputDecorator`, `PlayerNameOutputDecorator`, `ColoredOutputDecorator` |
| Client | `main()` |

### 7.5 Benefits and trade-offs

Facade reduces coupling between the entry point and the subsystem classes. It
also gives the application one clear startup operation. The trade-off is that
the facade can become too large if unrelated responsibilities are continuously
added to it.

## 8. Combined architecture

## 8.1. Design pattern diagrams

The following diagrams show how the three structural design patterns are
implemented and used in the application.

### 8.1.1. Decorator pattern

The `GameEngine` uses `IOutputWriter`. `PlayerNameOutputDecorator`,
`MessagePrefixOutputDecorator`, and `ColoredOutputDecorator` add formatting
and delegate the final result to `ConsoleWriter`.

```mermaid
classDiagram
    class IOutputWriter {
        <<interface>>
        +write(message: string) void
    }

    class ConsoleWriter {
        +write(message: string) void
    }

    class OutputDecorator {
        <<abstract>>
        #writer_: IOutputWriter
    }

    class PlayerNameOutputDecorator {
        -writer_: IOutputWriter
        -playerName_: string
        +PlayerNameOutputDecorator(writer, playerName)
        +write(message: string) void
    }

    class MessagePrefixOutputDecorator {
        -prefix_: string
        +MessagePrefixOutputDecorator(writer, prefix)
        +write(message: string) void
    }

    class ColoredOutputDecorator {
        -colorCode_: string
        +ColoredOutputDecorator(writer, colorCode)
        +write(message: string) void
    }

    class GameEngine {
        -writer_: IOutputWriter
        +start(startNodeId: int) void
    }

    IOutputWriter <|.. ConsoleWriter
    OutputDecorator <|-- PlayerNameOutputDecorator
    OutputDecorator <|-- MessagePrefixOutputDecorator
    OutputDecorator <|-- ColoredOutputDecorator
    IOutputWriter <|.. OutputDecorator
    OutputDecorator --> IOutputWriter : wraps
    GameEngine --> IOutputWriter : uses
```

The output flow is:

```mermaid
sequenceDiagram
    participant Engine as GameEngine
    participant Prefix as MessagePrefixOutputDecorator
    participant Decorator as PlayerNameOutputDecorator
    participant Color as ColoredOutputDecorator
    participant Writer as ConsoleWriter
    participant Console as Console

    Engine->>Decorator: write(message)
    Decorator->>Decorator: add player name
    Decorator->>Prefix: write(formatted message)
    Prefix->>Prefix: add [FAF CITY] prefix
    Prefix->>Color: delegate
    Color->>Color: add ANSI color and reset codes
    Color->>Writer: delegate
    Writer->>Console: print message
```

### 8.1.2. Composite pattern

Every action effect implements `IActionEffect`. A `CompositeEffect` implements
the same interface, but stores a collection of effects and applies them in
sequence. This allows the game engine to treat one effect and a group of
effects uniformly.

```mermaid
classDiagram
    class IActionEffect {
        <<interface>>
        +apply(player: Player) void
    }

    class StatEffect {
        +apply(player: Player) void
    }

    class ItemEffect {
        +apply(player: Player) void
    }

    class EndingEffect {
        +apply(player: Player) void
    }

    class CompositeEffect {
        -effects_: vector~IActionEffect~
        +add(effect: IActionEffect) void
        +apply(player: Player) void
        +empty() bool
    }

    IActionEffect <|.. StatEffect
    IActionEffect <|.. ItemEffect
    IActionEffect <|.. EndingEffect
    IActionEffect <|.. CompositeEffect
    CompositeEffect o-- "0..*" IActionEffect : contains
```

For example, buying a Neural Chip is represented by one composite containing
two effects:

```mermaid
sequenceDiagram
    participant Engine as GameEngine
    participant Composite as CompositeEffect
    participant Stats as StatEffect
    participant Item as ItemEffect
    participant Player

    Engine->>Composite: apply(Player)
    Composite->>Stats: apply(Player)
    Stats->>Player: money -80, intelligence +15
    Composite->>Item: apply(Player)
    Item->>Player: add "Neural Chip"
```

### 8.1.3. Facade pattern

`GameFacade` provides one simple entry point for starting the game. It
coordinates configuration, output adaptation, story loading, and game
execution, so `main()` does not depend on the internal startup sequence.

```mermaid
classDiagram
    class Main {
        +main() int
    }

    class GameFacade {
        +run(player: Player) void
    }

    class GameConfig {
        <<Singleton>>
        +setStoryFile(file: string) void
        +setStartNodeId(id: int) void
        +setChoicesPerNode(count: int) void
    }

    class StoryLoader {
        +load(fileName: string, engine: GameEngine) void
    }

    class GameEngine {
        +start(startNodeId: int) void
    }

    class PlayerNameOutputDecorator {
        +write(message: string) void
    }

    class MessagePrefixOutputDecorator {
        +write(message: string) void
    }

    class ColoredOutputDecorator {
        +write(message: string) void
    }

    Main --> GameFacade : calls run()
    GameFacade --> GameConfig : configures
    GameFacade --> StoryLoader : loads story
    GameFacade --> GameEngine : creates and starts
    GameFacade --> PlayerNameOutputDecorator : creates
    GameFacade --> MessagePrefixOutputDecorator : creates
    GameFacade --> ColoredOutputDecorator : creates
    GameEngine --> PlayerNameOutputDecorator : writes output
```

The startup sequence hidden by the facade is:

```mermaid
sequenceDiagram
    participant Main as main()
    participant Facade as GameFacade
    participant Config as GameConfig
    participant Decorator as PlayerNameOutputDecorator
    participant Loader as StoryLoader
    participant Engine as GameEngine
    participant Color as ColoredOutputDecorator

    Main->>Facade: run(player)
    Facade->>Config: set story file, start node, choice count
    Facade->>Decorator: create output decorator
    Facade->>Color: create cyan output decorator
    Facade->>Engine: create engine(player, decorator)
    Facade->>Loader: load(story file, engine)
    Loader-->>Engine: add story nodes and choices
    Facade->>Engine: start(start node)
```

### 8.1.4. Overall Lab 2 architecture

The three patterns work together as shown below. Character selection creates a
valid player, the facade starts the application, the loader builds composite effects from
JSON, and the decorator chain adds player-specific formatting, a prefix, and
cyan terminal color to console output.

```mermaid
flowchart TD
    Main[main.cpp] --> Options[CharacterOptions]
    Options --> Builder[PlayerBuilder]
    Builder --> Player[Player]
    Player --> Facade[GameFacade]

    Facade --> Config[GameConfig Singleton]
    Facade --> Loader[StoryLoader]
    Facade --> Engine[GameEngine]
    Facade --> Decorator[PlayerNameOutputDecorator]
    Decorator --> Prefix[MessagePrefixOutputDecorator]
    Prefix --> Color[ColoredOutputDecorator]
    Color --> Console[ConsoleWriter]

    Loader --> JSON[story.json]
    Loader --> Factories[Effect Factories]
    Factories --> Composite[CompositeEffect]
    Composite --> Effects[StatEffect / ItemEffect / EndingEffect]
    Engine --> Composite
    Engine --> Decorator
```

## 9. Verification scenarios

The following scenarios should be checked:

1. Select **Richie** and confirm the player starts with 1000 money and low
   intelligence.
2. Select **The Hacker** and confirm the player starts with low health, high
   intelligence, and a Neural Chip.
3. Select **The Survivor** and confirm the player starts poor, healthy, and
   with a Medkit.
4. Choose custom character creation and verify valid values are accepted.
5. Enter invalid custom values and verify the existing Builder validation
   reports an explicit error.
6. Select a choice with multiple JSON effects and verify all effects are
   applied by `CompositeEffect`.
7. Verify game messages pass through `PlayerNameOutputDecorator`,
   `MessagePrefixOutputDecorator`, and `ColoredOutputDecorator`.
8. Verify the complete game starts through `GameFacade`.
9. Verify invalid choices, missing items, insufficient intelligence, missing
   story files, and ending effects retain their previous behavior.

## 10. Unit tests

The test executable contains two focused tests for each structural pattern:

| Pattern | Test | What it verifies |
|---|---|---|
| Decorator | `decoratorAddsResponsibility` | A decorator delegates through the shared interface and adds the player name. |
| Decorator | `decoratorsCanBeStacked` | Two or three concrete decorators can be combined without changing the component. |
| Decorator | `coloredDecoratorUsesMultipleGameColors` | Money is yellow, health is red, and error messages are red. |
| Composite | `compositeAppliesLeafEffectsInOrder` | A group applies multiple leaf effects to one player. |
| Composite | `compositeAndLeafShareTheSameInterface` | A leaf and a composite are both usable as `IActionEffect`. |
| Facade | `facadeStartsTheConfiguredGame` | The facade configures, loads, decorates, and starts the game. |
| Facade | `facadeRunsAnotherSubsystemPath` | The same facade coordinates a different story path. |

Run them with:

```powershell
Set-Location lab2
.\tests.exe
```

Successful output:

```text
All Lab 2 tests passed.
```

Source: [tests.cpp](tests.cpp)

## 11. Compilation and execution

From the repository root, compile with a C++17 compiler:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic lab2\main.cpp -o lab2\main.exe
g++ -std=c++17 -Wall -Wextra -pedantic lab2\tests.cpp -o lab2\tests.exe
```

Run the program from the `lab2` directory so that `story.json` is found:

```powershell
Set-Location lab2
.\main.exe
```

## 12. Conclusion

The Lab 2 implementation extends the existing game without changing the
previous laboratory directories. Decorator adds player-specific output
formatting, a game prefix, and terminal color, Composite models a choice
containing multiple effects, and Facade simplifies application startup.
Character presets and custom creation provide
multiple ways to construct a valid `Player` while reusing the Lab 1 Builder
and its validation rules.
