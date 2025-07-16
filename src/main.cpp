#include "cli_interface.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        CLIInterface cli;
        
        // If arguments provided, process as single command
        if (argc > 1) {
            std::string command;
            for (int i = 1; i < argc; ++i) {
                command += argv[i];
                if (i < argc - 1) command += " ";
            }
            cli.processCommand(command);
        } else {
            // Interactive mode
            cli.run();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
