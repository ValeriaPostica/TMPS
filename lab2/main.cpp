#include "CharacterOptions.h"
#include "GameFacade.h"
#include <exception>
#include <iostream>
using namespace std;

int main() {
    try {
        Player player = buildSelectedCharacter();
        GameFacade game;
        game.run(move(player));
    } catch (const exception& error) {
        cerr << "Unable to start the game: " << error.what() << '\n';
        return 1;
    }
}
