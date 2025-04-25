#include <iostream>
#include <string>
#include "gui/ChessGUI.hpp"

int main(int argc, char **argv)
{
    try
    {
        // Default FEN for standard chess starting position
        std::string startingFen = chess::STARTPOS;

        // Override with custom FEN if provided as a command line argument
        if (argc > 1)
        {
            startingFen = argv[1];
        }

        // Create and run the GUI
        ChessGUI gui(600, 710);

        // Set the position if a custom FEN was provided
        if (startingFen != chess::STARTPOS)
        {
            gui.setPosition(startingFen);
        }

        // Run the GUI
        gui.run();

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}