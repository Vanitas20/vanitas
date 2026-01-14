#include "vanitas/args_parser.hpp"
#include <stdexcept>

namespace vanitas {

ArgsParser::ArgsParser(int argc, char *const argv[]) : argc_(argc), argv_(argv) {}

Args ArgsParser::parse()
{
    if (argc_ < 2) {
        return parse_help();
    }

    std::string cmd = argv_[1];

    if (cmd == "file")
        return parse_file();
    if (cmd == "run")
        return parse_run();
    if (cmd == "pipe")
        return parse_pipe();
    if (cmd == "help" || cmd == "-h" || cmd == "--help")
        return parse_help();

    throw std::runtime_error("Unknown command: " + cmd);
}

Args ArgsParser::parse_file()
{
    Args out;
    out.mode = Mode::File;

    if (argc_ < 3) {
        throw std::runtime_error("Usage: vanitas file <path>");
    }

    out.file = argv_[2];

    return out;
}

Args ArgsParser::parse_pipe()
{
    Args out;
    out.mode = Mode::Pipe;

    // TODO: parse options like: --profile "name"

    return out;
}

Args ArgsParser::parse_run()
{
    Args out;
    out.mode = Mode::Run;

    int i = 2;
    for (; i < argc_; ++i) {
        std::string a = argv_[i];
        if (a == "--") {
            ++i;
            break;
        }
        if (a == "--profile") {
            out.profile = argv_[i];
            ++i;
            break;
        }
    }

    for (; i < argc_; ++i)
        out.cmd.push_back(argv_[i]);

    if (out.cmd.empty()) {
        throw std::runtime_error("Usage: vanitas run [--profile <name>] -- <cmd> [args...]");
    }

    return out;
}

Args ArgsParser::parse_help()
{
    Args out;
    out.mode = Mode::Help;
    return out;
}

} // namespace vanitas
