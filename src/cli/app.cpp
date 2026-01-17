#include "app.hpp"
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <toml.hpp>

#include "commands/include/file.hpp"
#include "commands/include/help.hpp"
#include "commands/include/pipe.hpp"
#include "commands/include/profile.hpp"
#include "commands/include/run.hpp"
#include "vanitas/args_parser.hpp"
#include "vanitas/config.hpp"
#include "vanitas/profile_manager.hpp"

namespace vanitas::cli {
namespace fs = std::filesystem;

struct ProfileDump
{
        std::vector<std::string> firstline;
        std::vector<std::string> continuation;
        std::vector<std::string> err;
        std::vector<std::string> wrn;
        std::vector<std::string> tests;
};

static ProfileDump dump_from_value_fallback_empty(const toml::value &v)
{
    ProfileDump out;
    out.firstline = toml::find_or(v, "firstline", "patterns", std::vector<std::string>{});
    out.continuation = toml::find_or(v, "continuation", "patterns", std::vector<std::string>{});
    out.err = toml::find_or(v, "classify", "err", std::vector<std::string>{});
    out.wrn = toml::find_or(v, "classify", "wrn", std::vector<std::string>{});
    out.tests = toml::find_or(v, "classify", "tests", std::vector<std::string>{});
    return out;
}

static ProfileDump merge_dump(const ProfileDump &base, const toml::value &overlay)
{
    ProfileDump out = base;

    auto apply = [&](std::vector<std::string> &dst, const std::optional<std::vector<std::string>> &pats) {
        if (pats)
            dst = *pats;
    };

    apply(out.firstline, toml::find<std::optional<std::vector<std::string>>>(overlay, "firstline", "patterns"));
    apply(out.continuation, toml::find<std::optional<std::vector<std::string>>>(overlay, "continuation", "patterns"));
    apply(out.err, toml::find<std::optional<std::vector<std::string>>>(overlay, "classify", "err"));
    apply(out.wrn, toml::find<std::optional<std::vector<std::string>>>(overlay, "classify", "wrn"));
    apply(out.tests, toml::find<std::optional<std::vector<std::string>>>(overlay, "classify", "tests"));

    return out;
}

static std::optional<toml::value> try_parse_file(const fs::path &p)
{
    const auto r = toml::try_parse(p.string());
    if (r.is_err())
        return std::nullopt;
    return r.unwrap();
}

static void print_list(const char *name, const std::vector<std::string> &v)
{
    std::cout << name << " (" << v.size() << ")\n";
    for (const auto &s : v)
        std::cout << "  - " << s << "\n";
}

int CliApp::run(int argc, char *const argv[])
{
    try {
        vanitas::ArgsParser parser(argc, argv);
        vanitas::Args args = parser.parse();

        if (args.mode == vanitas::Mode::Help) {
            HelpCommand cmd(args);
            std::exit(cmd.execute());
        }

        vanitas::ProfileManager pm;
        pm.ensure_default_profiles();

        if (args.mode == vanitas::Mode::ProfileList) {
            ProfileCommand cmd(args, pm);
            std::exit(cmd.execute());
        }

        const vanitas::Config cfg = vanitas::load_user_config();
        auto cfgv = vanitas::load_user_config_value();
        const std::string profile_name = args.profile.value_or(cfg.profile.value_or("default"));

        const char *profile_selected_src = args.profile ? "CLI --profile" : (cfg.profile ? "config.profile" : "default");

        if (args.dump_config) {
            std::cout << "Effective config\n";
            std::cout << "  profile = " << profile_name << " (source: " << profile_selected_src << ")\n";
            std::cout << "  color  = " << (cfg.color ? "true" : "false") << "\n";
            std::cout << "  format = " << cfg.format << "\n";
            std::exit(0);
        }

        if (args.dump_profile) {
            std::cout << "Selected profile: " << profile_name << " (source: " << profile_selected_src << ")\n";

            try {
                (void)pm.load_effective(profile_name, cfgv);
                std::cout << "OK: resolved and compiled (extends applied)\n";
                std::exit(0);
            } catch (const std::exception &e) {
                std::cerr << e.what() << "\n";
                std::exit(2);
            }
        }

        vanitas::Profile prof = pm.load_effective(profile_name, cfgv);

        int rc = 0;
        switch (args.mode) {
        case vanitas::Mode::File:
            rc = FileCommand(args, prof).execute();
            break;
        case vanitas::Mode::Pipe:
            rc = PipeCommand(args, prof).execute();
            break;
        case vanitas::Mode::Run:
            rc = RunCommand(args, prof).execute();
            break;
        default:
            rc = 2;
            break;
        }

        std::exit(rc);
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        std::exit(2);
    }
}
} // namespace vanitas::cli
