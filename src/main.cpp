#include "Game.h"
#include <iostream>
#include <string>

#ifdef HAS_GUI
#include "GUI.h"
#endif

int main(int argc, char *argv[])
{
    bool guiMode = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--gui")
            guiMode = true;
    }

    Game game;

    if (guiMode)
    {
#ifdef HAS_GUI
        ChessGUI gui(game);
        if (!gui.init(640))
        {
            std::cerr << "Failed to initialize GUI.\n";
            return 1;
        }
        gui.run();
        gui.shutdown();
#else
        std::cerr << "GUI not available (SDL2 not found at build time).\n";
        return 1;
#endif
    }
    else
    {
        GameMode selectedMode;
        Difficulty selectedDiff;
        Color selectedCPUColor;
        Game::showMenu(selectedMode, selectedDiff, selectedCPUColor);
        game.setMode(selectedMode);
        game.setDifficulty(selectedDiff);
        game.setCPUColor(selectedCPUColor);
        game.run();
    }

    return 0;
}