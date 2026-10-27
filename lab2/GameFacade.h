#ifndef GAME_FACADE_H
#define GAME_FACADE_H
#include "GameConfig.h"
#include "GameEngine.h"
#include "OutputDecorators.h"
#include "PlayerBuilder.h"
#include "StoryLoader.h"
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
        PlayerNameOutputDecorator writer(consoleWriter, player.name());
        GameEngine engine(player, writer);
        StoryLoader loader;
        loader.load(config.storyFile(), engine);
        engine.start(config.startNodeId());
    }
};

#endif
