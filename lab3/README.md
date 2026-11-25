# FAF City: Choose Your Destiny

Laboratory Work No. 3 for TMPS. This C++17 console application continues the
Lab 2 structural-pattern implementation and adds behavioral design patterns:
Command, State, and Observer.

## Build

Run commands from the repository root:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic lab3\main.cpp -o lab3\main.exe
g++ -std=c++17 -Wall -Wextra -pedantic lab3\tests.cpp -o lab3\tests.exe
```

## Run

```powershell
Set-Location lab3
.\main.exe
```

The game loads `story.json`, so it must be started from the `lab3` folder.
During play, the menu also provides:

- `Undo last choice`: restore the player's previous state and return to the
  previous story question.

## Test

From `lab3`:

```powershell
.\run-tests.bat
```

Or from the repository root:

```powershell
Set-Location lab3
.\tests.exe
```

The executable tests cover Lab 2 and Lab 3 behavior. Successful output is:

```text
All Lab 3 tests passed.
```

## Structure

- `Behavioral/` contains Command, State, and Observer;
- `SOLID/` contains domain entities and the game engine;
- `Creational/` contains Builder, Factory Method, and Singleton;
- `Structural/` contains Decorator, Composite, and Facade;
- `CharacterOptions.h` and `StoryLoader.h` provide application support;
- `story.json` contains the story graph;
- `tests.cpp` contains executable unit tests;
- `REPORT.md` contains diagrams, code explanations, and test evidence.
