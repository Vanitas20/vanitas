#pragma once

#include <optional>
#include <string>
#include <vector>

namespace vanitas {

enum Mode {
    Help,
    File,
    Run,
    Pipe,
    ProfileList,
};

struct Args
{
        Mode mode = Mode::Help;
        std::optional<std::string> profile;
        std::string file;
        std::vector<std::string> cmd;
        bool dump_config = false;
        bool dump_profile = false;
};

class ArgsParser
{
    public:
        ArgsParser(int argc, char *const argv[]);
        Args parse();

    private:
        int argc_;
        char *const *argv_;

        Args parse_file(int start, Args out);
        Args parse_pipe(int start, Args out);
        Args parse_run(int start, Args out);
        Args parse_help();
        Args parse_profile(int start, Args out);
};

} // namespace vanitas
