# FAF City: Choose Your Destiny

Laboratory Work No. 2 for TMPS. This C++17 console application is a
choose-your-own-adventure game that demonstrates three SOLID principles,
three creational patterns, and three structural patterns.

## Build

From the repository root:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic lab2\main.cpp -o lab2\main.exe
g++ -std=c++17 -Wall -Wextra -pedantic lab2\tests.cpp -o lab2\tests.exe
```

## Run

Run the application from the `lab2` directory:

```powershell
Set-Location lab2
.\main.exe
```

The story is loaded from `story.json`, so the working directory is important.

## Test

From the repository root:

```powershell
Set-Location lab2
.\tests.exe
```

The tests cover Decorator delegation and stacking, including the
`ColoredOutputDecorator` and its yellow money/red health highlighting,
Composite behavior through the shared interface, and Facade startup paths.

To compile and run the tests with one command from the `lab2` directory:

```powershell
.\run-tests.bat
```

## Structure

- `SOLID/` contains the three SOLID-principle implementations.
- `Creational/` contains Builder, Factory Method, and Singleton.
- `Structural/` contains the Decorator, Composite, and Facade implementations.
- `CharacterOptions.h`, `StoryLoader.h`, and the application entry points are
  application support files that use the grouped implementations.
- `tests.cpp` contains the executable unit tests.
- `REPORT.md` contains the detailed design report and diagrams.
