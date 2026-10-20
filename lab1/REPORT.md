# Laboratory Work No. 1

**Course:** TMPS (Software Modeling and Design Techniques)  
**Topic:** Creational Design Patterns in FAF City: Choose Your Destiny  
**Language:** C++17

## 1. Objective

This laboratory continues the terminal-based adventure game developed in Laboratory
Work No. 0. The game reads a story graph from JSON, creates the corresponding
domain objects, and lets the player navigate through FAF City.

The objective of this laboratory is to implement three creational design patterns:

1. Builder;
2. Factory Method;
3. Singleton.

The existing gameplay, JSON format, effect behavior, validation rules, and
dependency-injection design from Lab 0 are preserved.

## 2. Project structure

```text
lab1/
├── story.json              # story nodes, choices, requirements, and effects
├── GameInterfaces.h        # IActionEffect and IOutputWriter abstractions
├── StoryEntities.h         # Player, effects, Choice, and StoryNode
├── PlayerBuilder.h         # Builder implementation for Player
├── EffectFactories.h       # Factory Method creator hierarchy
├── GameConfig.h            # Singleton application configuration
├── GameEngine.h            # game loop and story navigation
├── StoryLoader.h           # JSON loading and effect creation
├── main.cpp                # composition root
└── nlohmann/json.hpp       # JSON library copied from Lab 0
```

## 3. Existing Lab 0 architecture

The original architecture already separates the main responsibilities:

- `Player` stores statistics, inventory, and ending state.
- `StoryNode` stores narrative text and choices.
- `StoryLoader` translates JSON into domain objects.
- `GameEngine` validates choices, applies effects, and follows the story graph.
- `IOutputWriter` allows output to be replaced without changing the engine.
- `IActionEffect` allows effects to be executed polymorphically.

Lab 1 adds object-creation abstractions without moving game-flow logic into the
creation classes.

## 4. Builder pattern

### 4.1 Motivation

Before Lab 1, a player could be created directly with only a name:

```cpp
Player player(name);
```

The remaining values were fixed in member initializers. This is insufficient when
the application needs different starting configurations, such as a rich player,
a highly intelligent player, or a player with starting items.

### 4.2 Implementation

`PlayerBuilder` stores construction parameters and exposes chainable setters:

```cpp
Player player = PlayerBuilder()
    .setName(name)
    .setHealth(100)
    .setMoney(100)
    .setIntelligence(10)
    .setReputation(0)
    .build();
```

The builder still performs the final validation, so invalid values are rejected
when `build()` is called rather than being accepted silently.

The accepted input ranges are:

| Value | Minimum | Maximum | Explanation |
|---|---:|---:|---|
| Health | 0 | 100 | The builder validates this range explicitly |
| Money | 0 | No gameplay maximum | Negative money is rejected |
| Intelligence | 0 | 100 | The builder validates this range explicitly |
| Reputation | -100 | 100 | The builder validates this range explicitly |

These are the intended gameplay ranges. Money has no application-level upper
limit, although it is stored in a C++ `int`.

The final `build()` method validates the configuration and constructs `Player`.
The validation rules are:

- the name must not be empty;
- health must be between 0 and 100;
- money must not be negative;
- intelligence must be between 0 and 100;
- reputation must be between -100 and 100.

`Player` has a constructor that accepts all initial values, while the builder
controls the readable construction process.

### 4.3 Participants

| Participant | Class | Responsibility |
|---|---|---|
| Product | `Player` | Represents the object being built |
| Builder | `PlayerBuilder` | Stores options and validates construction |
| Client | `main()` | Configures and requests the product |

### 4.4 Advantages

- avoids a long constructor with many parameters;
- makes optional configuration explicit;
- places construction validation in one location;
- supports multiple player configurations;
- keeps `main()` readable.

### 4.5 Trade-offs

The builder introduces an additional class and duplicates some constructor
parameters. It is justified when an object has several configuration options. For
a class with only one or two required values, direct construction would be simpler.

## 5. Factory Method pattern

### 5.1 Motivation

In Lab 0, `StoryLoader::parseEffect()` directly created every concrete effect:

```cpp
make_shared<StatEffect>(...);
make_shared<ItemEffect>(...);
make_shared<EndingEffect>(...);
```

As more effect types are added, the loader would become increasingly coupled to
concrete classes.

### 5.2 Implementation

`EffectCreator` is the creator abstraction. It declares the virtual factory
method:

```cpp
class EffectCreator {
public:
    virtual ~EffectCreator() = default;
    virtual shared_ptr<IActionEffect>
    create(const Json& json) const = 0;
};
```

Concrete creators implement the creation operation:

- `StatEffectCreator` creates `StatEffect`;
- `ItemEffectCreator` creates `ItemEffect`;
- `EndingEffectCreator` creates `EndingEffect`.

The loader delegates concrete construction through `createEffect()`:

```cpp
effects.push_back(createEffect(effectJson));
```

The loader still reads the JSON type because it is responsible for interpreting
the external data format. The concrete object construction is isolated in the
creator classes.

### 5.3 Participants

| Participant | Class | Responsibility |
|---|---|---|
| Product | `IActionEffect` | Common interface for all effects |
| Concrete products | `StatEffect`, `ItemEffect`, `EndingEffect` | Implement effect behavior |
| Creator | `EffectCreator` | Defines the creation contract |
| Concrete creators | `StatEffectCreator`, `ItemEffectCreator`, `EndingEffectCreator` | Construct concrete products |
| Client | `StoryLoader` | Requests the appropriate product |

### 5.4 Extension example

To add a new `TeleportEffect`, the implementation would need a
`TeleportEffectCreator` and registration in the type-selection function. The
`GameEngine` would not need to change because it already works with
`IActionEffect`.

### 5.5 Advantages and trade-offs

Factory Method:

- hides concrete construction from the main loading flow;
- preserves polymorphism;
- keeps the engine independent of concrete effect classes;
- makes new product types easier to add.

The trade-off is that each product type requires a creator class. For a very
small application, a simple factory function would be shorter, but the creator
hierarchy demonstrates the requested Factory Method pattern more clearly.

## 6. Singleton pattern

### 6.1 Motivation

The application has configuration values that should be shared consistently:

- the story file name;
- the starting node identifier;
- the required number of choices per story node.

Previously, these values were hardcoded in different parts of the application.
`GameConfig` centralizes them.

### 6.2 Implementation

The singleton is accessed through:

```cpp
GameConfig& config = GameConfig::instance();
```

The implementation uses a function-local static object:

```cpp
static GameConfig config;
return config;
```

This guarantees one instance per process and initializes it on first use.
Copying and assignment are disabled:

```cpp
GameConfig(const GameConfig&) = delete;
GameConfig& operator=(const GameConfig&) = delete;
```

The application configures and uses the singleton in `main()`:

```cpp
config.setStoryFile("story.json");
config.setStartNodeId(1);
config.setChoicesPerNode(3);

loader.load(config.storyFile(), engine);
engine.start(config.startNodeId());
```

`StoryLoader` also reads `choicesPerNode()` when validating the story.

### 6.3 Participants

| Participant | Class or operation | Responsibility |
|---|---|---|
| Singleton | `GameConfig` | Stores one shared configuration |
| Access point | `GameConfig::instance()` | Returns the unique instance |
| Clients | `main()`, `StoryLoader` | Read or update configuration |

### 6.4 Advantages and trade-offs

The singleton provides:

- one authoritative configuration;
- easy access from the application boundary;
- centralized validation of configuration values.

Singleton also introduces global shared state, which can make tests less isolated.
For that reason, it is used only for application configuration. `Player`,
`GameEngine`, `StoryLoader`, and `ConsoleWriter` remain ordinary objects and are
created explicitly.

## 7. Patterns considered but not implemented

### 7.1 Prototype

Prototype would be suitable for cloning preconfigured story nodes or players.
For a polymorphic `IActionEffect`, correct deep copying would require a virtual
`clone()` operation on the interface and every concrete effect. The current
application does not need to duplicate story nodes at runtime, so Prototype
would add complexity without solving a current problem.

### 7.2 Abstract Factory

Abstract Factory creates related families of objects. It would become useful if
the game supported multiple themes, for example:

```text
CyberpunkFactory -> cyberpunk player, nodes, and effects
FantasyFactory  -> fantasy player, nodes, and effects
```

The current project has one FAF City product family, so a complete Abstract
Factory would be artificial. Factory Method is the more appropriate abstraction
for the current effect hierarchy.

### 7.3 Object Pooling

Object Pooling reuses frequently created and destroyed objects. Story nodes and
effects are created while loading the story and remain in use during gameplay.
They are not allocated frequently enough to justify a pool. Smart pointers already
provide the required ownership and lifetime management.

## 8. Interaction between the patterns

The three patterns have separate responsibilities:

```text
main()
 ├── GameConfig::instance()  -> shared application configuration
 ├── PlayerBuilder           -> configured Player
 ├── StoryLoader
 │    └── Factory Method      -> concrete IActionEffect objects
 └── GameEngine              -> gameplay execution
```

The patterns do not replace the existing abstractions. `GameEngine` continues to
depend on `IOutputWriter` and `IActionEffect`, preserving the dependency
inversion and open/closed principles demonstrated in Lab 0.

## 9. Verification

The project was compiled from the repository root with:

```text
g++ -std=c++17 -Wall -Wextra -pedantic lab1\main.cpp -o lab1\lab1.exe
```

The compilation completed successfully without warnings.

The following behavior should be checked when running the executable:

1. enter a non-empty player name;
2. confirm that the first story node is loaded from `story.json`;
3. select choices that create stat and item effects;
4. verify that required items and intelligence are still checked;
5. reach an ending and confirm the ending message is displayed;
6. change configuration values and confirm that the loader and engine use them;
7. create alternative players through `PlayerBuilder` and verify validation.

## 10. Conclusion

Lab 1 extends the Lab 0 adventure game with three creational design patterns.
Builder makes `Player` construction configurable and validated. Factory Method
isolates creation of polymorphic action effects from JSON loading. Singleton
centralizes application configuration while keeping the rest of the application
explicitly constructed and testable.

The patterns improve separation of concerns without changing the story format or
gameplay rules. The result is easier to extend: new player configurations,
effects, and application settings can be introduced with localized changes.
