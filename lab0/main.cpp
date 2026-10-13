#include "StoryLoader.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
    string name;
    cout << "Enter your hero name: ";
    cin >> name;
    Player player(name); ConsoleWriter writer; GameEngine engine(player, writer);
    StoryLoader loader;
    loader.load("story.json", engine);
    engine.start(1);
}
