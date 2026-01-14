#include "app.hpp"
#include <cstdlib>
#include <exception>
#include <iostream>

#include "commands/include/file.hpp"
#include "commands/include/help.hpp"
#include "commands/include/pipe.hpp"
#include "commands/include/run.hpp"
#include "vanitas/args_parser.hpp"
#include "vanitas/profile_manager.hpp"

namespace vanitas::cli {
int CliApp::run(int argc, char *const argv[])
{
    try {
        vanitas::ArgsParser parser(argc, argv);
        vanitas::Args args = parser.parse();

        vanitas::ProfileManager pm;
        pm.ensure_default_profiles();

        vanitas::Profile prof;
        try {
            prof = pm.load(args.profile);
        } catch (const std::exception &e) {
            throw;
        }

        int rc = 0;

        switch (args.mode) {
        case vanitas::Mode::Help: {
            HelpCommand cmd(args);
            rc = cmd.execute();
            break;
        }
        case vanitas::Mode::File: {
            FileCommand cmd(args, prof);
            rc = cmd.execute();
            break;
        }
        case vanitas::Mode::Pipe: {
            PipeCommand cmd(args, prof);
            rc = cmd.execute();
            break;
        }
        case vanitas::Mode::Run: {
            RunCommand cmd(args, prof);
            rc = cmd.execute();
            break;
        }
        }

        std::exit(rc);
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        std::exit(2);
    }
}
} // namespace vanitas::cli
