#ifndef GAME_FACADE_H
#define GAME_FACADE_H
#include "../SOLID/GameEngine.h"
#include "../StoryLoader.h"
#include "../Creational/GameConfig.h"
#include "../Creational/PlayerBuilder.h"
#include "OutputDecorators.h"
#include <string>
#include <utility>
using namespace std;

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

#endif
