#include "commands/include/help.hpp"
#include <iostream>

namespace vanitas::cli {

int HelpCommand::execute()
{
    std::cout << "vanitas - log analyzer\n"
              << "\n"
              << "Usage:\n"
              << "  vanitas help\n"
              << "  vanitas file <path>\n"
              << "  vanitas pipe\n"
              << "  vanitas run -- <cmd> [args...]\n"
              << "\n"
              << "Commands:\n"
              << "  help   Show this help.\n"
              << "  file   Analyze a file.\n"
              << "  pipe   Analyze stdin.\n"
              << "  run    Run a command and analyze its output (stdout+stderr).\n"
              << "\n";
    return 0;
}

} // namespace vanitas::cli
