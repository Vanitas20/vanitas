#pragma once

#include <string>
#include <vector>

namespace vanitas {

enum Mode {
    Help,
    File,
    Run,
    Pipe
};

struct Args
{
        Mode mode = Mode::Help;
        std::string profile = "default";
        std::string file;
        std::vector<std::string> cmd;
};

class ArgsParser
{
    public:
        ArgsParser(int argc, char *const argv[]);
        Args parse();

    private:
        int argc_;
        char *const *argv_;

        Args parse_file();
        Args parse_pipe();
        Args parse_run();
        Args parse_help();
};

} // namespace vanitas
