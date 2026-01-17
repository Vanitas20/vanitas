#include "vanitas/args_parser.hpp"
#include <stdexcept>

namespace vanitas {

static void parse_profile_opt(int &i, int argc, char *const *argv, Args &out)
{
    // argv[i] == "--profile"
    if (i + 1 >= argc) {
        throw std::runtime_error("Usage: --profile <name>");
    }
    out.profile = std::string(argv[i + 1]);
    i += 1;
}

static bool is_global_flag(const std::string &a)
{
    return a == "--profile" || a == "--dump-config" || a == "--dump-profile" || a == "-h" || a == "--help";
}

static void parse_global_flag(int &i, int argc, char *const *argv, Args &out)
{
    std::string a = argv[i];

    if (a == "--dump-config") {
        out.dump_config = true;
        return;
    }
    if (a == "--dump-profile") {
        out.dump_profile = true;
        return;
    }
    if (a == "--profile") {
        parse_profile_opt(i, argc, argv, out);
        return;
    }

    throw std::logic_error("parse_global_flag: unreachable for arg: " + a);
}

ArgsParser::ArgsParser(int argc, char *const argv[]) : argc_(argc), argv_(argv) {}

Args ArgsParser::parse()
{
    Args out;

    if (argc_ < 2) {
        out.mode = Mode::Help;
        return out;
    }

    int i = 1;

    // глобальні прапорці ДО команди
    for (; i < argc_; ++i) {
        std::string a = argv_[i];

        if (a == "-h" || a == "--help") {
            out.mode = Mode::Help;
            return out;
        }

        if (!is_global_flag(a))
            break;

        parse_global_flag(i, argc_, argv_, out);
    }

    if (i >= argc_) {
        out.mode = Mode::Help;
        return out;
    }

    std::string cmd = argv_[i];

    if (cmd == "file")
        return parse_file(i + 1, std::move(out));
    if (cmd == "run")
        return parse_run(i + 1, std::move(out));
    if (cmd == "pipe")
        return parse_pipe(i + 1, std::move(out));
    if (cmd == "profile")
        return parse_profile(i + 1, std::move(out));
    if (cmd == "help") {
        out.mode = Mode::Help;
        return out;
    }

    throw std::runtime_error("Unknown command: " + cmd);
}

Args ArgsParser::parse_file(int start, Args out)
{
    out.mode = Mode::File;

    for (int i = start; i < argc_; ++i) {
        std::string a = argv_[i];

        if (a == "--profile") {
            parse_profile_opt(i, argc_, argv_, out);
            continue;
        }
        if (a == "--dump-config") {
            out.dump_config = true;
            continue;
        }
        if (a == "--dump-profile") {
            out.dump_profile = true;
            continue;
        }

        out.file = argv_[i];
        break;
    }

    if (out.file.empty()) {
        throw std::runtime_error("Usage: vanitas [global opts] file [opts] <path>");
    }
    return out;
}

Args ArgsParser::parse_pipe(int start, Args out)
{
    out.mode = Mode::Pipe;

    for (int i = start; i < argc_; ++i) {
        std::string a = argv_[i];
        if (a == "--profile") {
            parse_profile_opt(i, argc_, argv_, out);
            continue;
        }
        if (a == "--dump-config") {
            out.dump_config = true;
            continue;
        }
        if (a == "--dump-profile") {
            out.dump_profile = true;
            continue;
        }
        throw std::runtime_error("Usage: vanitas pipe [--profile <name>]");
    }
    return out;
}

Args ArgsParser::parse_run(int start, Args out)
{
    out.mode = Mode::Run;

    int i = start;

    for (; i < argc_; ++i) {
        std::string a = argv_[i];

        if (a == "--profile") {
            parse_profile_opt(i, argc_, argv_, out);
            continue;
        }

        if (a == "--dump-config") {
            out.dump_config = true;
            continue;
        }
        if (a == "--dump-profile") {
            out.dump_profile = true;
            continue;
        }

        if (a == "--") {
            ++i;
            break;
        }

        throw std::runtime_error("Usage: vanitas run [--profile <name>] -- <cmd> [args...]");
    }

    for (; i < argc_; ++i) {
        out.cmd.push_back(argv_[i]);
    }

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

Args ArgsParser::parse_profile(int start, Args out)
{
    if (start >= argc_) {
        throw std::runtime_error("Usage: vanitas profile <subcommand>\nSubcommands: list");
    }

    std::string sub = argv_[start];

    if (sub == "list") {
        out.mode = Mode::ProfileList;
        return out;
    }

    throw std::runtime_error("Unknown subcommand: profile " + sub + "\nUsage: vanitas profile list");
}

} // namespace vanitas
