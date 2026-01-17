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

        const vanitas::Config cfg = vanitas::load_user_config();
        const auto cfgv = vanitas::load_user_config_value();

        const std::string profile_name = args.profile.value_or(cfg.profile.value_or("default"));

        const char *profile_selected_src = args.profile ? "CLI --profile" : (cfg.profile ? "config.profile" : "default");

        if (args.dump_config) {
            std::cout << "Effective config\n";
            std::cout << "  profile = " << profile_name << " (source: " << profile_selected_src << ")\n";

            std::cout << "  color  = " << (cfg.color ? "true" : "false") << " (source: " << (cfgv ? "config/defaults" : "defaults")
                      << ")\n";
            std::cout << "  format = " << cfg.format << " (source: " << (cfgv ? "config/defaults" : "defaults") << ")\n";
            std::exit(0);
        }

        if (args.dump_profile) {
            const fs::path def_path = pm.profiles_dir() / "default.toml";
            ProfileDump base_dump;

            if (auto defv = try_parse_file(def_path)) {
                base_dump = dump_from_value_fallback_empty(*defv);
            }

            std::string overlay_src = "none";
            ProfileDump eff = base_dump;

            // 1) inline профіль з config.toml має пріоритет
            bool used_inline = false;
            if (cfgv) {
                try {
                    const toml::value pv = toml::find(*cfgv, "profiles", profile_name);
                    eff = merge_dump(base_dump, pv);
                    overlay_src = "config.toml:[profiles." + profile_name + "]";
                    used_inline = true;
                } catch (...) {
                }
            }

            if (!used_inline) {
                const fs::path p = pm.profiles_dir() / (profile_name + ".toml");
                if (auto fv = try_parse_file(p)) {
                    eff = merge_dump(base_dump, *fv);
                    overlay_src = p.string();
                } else {
                    overlay_src = "none (using default only)";
                }
            }

            std::cout << "Selected profile: " << profile_name << " (source: " << profile_selected_src << ")\n";
            std::cout << "Overlay source: " << overlay_src << "\n";
            std::cout << "Base: " << def_path.string() << "\n\n";

            print_list("firstline.patterns", eff.firstline);
            print_list("continuation.patterns", eff.continuation);
            print_list("classify.err", eff.err);
            print_list("classify.wrn", eff.wrn);
            print_list("classify.tests", eff.tests);

            std::exit(0);
        }

        if (args.mode == vanitas::Mode::ProfileList) {
            ProfileCommand cmd(args, pm);
            std::exit(cmd.execute());
        }

        vanitas::Profile prof;
        if (cfgv) {
            try {
                const toml::value pv = toml::find(*cfgv, "profiles", profile_name);
                prof = pm.load_from_value(pm.load("default"), pv);
            } catch (const std::exception &) {
                prof = pm.load(profile_name);
            }
        } else {
            prof = pm.load(profile_name);
        }

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
