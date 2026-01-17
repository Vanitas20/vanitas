#include "commands/include/profile.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <toml.hpp>
#include <unordered_set>
#include <utility>
#include <vector>

#include "vanitas/config.hpp"

namespace vanitas::cli {
namespace fs = std::filesystem;

static std::vector<std::pair<std::string, fs::path>> list_file_profiles(const fs::path &dir)
{
    std::vector<std::pair<std::string, fs::path>> out;

    std::error_code ec;
    if (!fs::exists(dir, ec) || ec)
        return out;

    for (const auto &entry : fs::directory_iterator(dir, ec)) {
        if (ec)
            break;

        const fs::path p = entry.path();
        if (!entry.is_regular_file(ec) || ec)
            continue;
        if (p.extension() != ".toml")
            continue;

        out.emplace_back(p.stem().string(), p);
    }

    std::sort(out.begin(), out.end(), [](const auto &a, const auto &b) { return a.first < b.first; });
    return out;
}

static std::vector<std::string> list_inline_profiles(const std::optional<toml::value> &cfgv)
{
    std::vector<std::string> names;
    if (!cfgv)
        return names;

    // [profiles.*] -> table
    const toml::table profiles = toml::find_or(*cfgv, "profiles", toml::table{});

    names.reserve(profiles.size());
    for (const auto &kv : profiles) {
        names.push_back(kv.first);
    }

    std::sort(names.begin(), names.end());
    return names;
}

int ProfileCommand::execute()
{
    // Selected profile (для UX)
    const vanitas::Config cfg = vanitas::load_user_config();
    const std::string selected = args_.profile ? *args_.profile : (cfg.profile ? *cfg.profile : std::string("default"));

    const std::string selected_src = args_.profile ? "CLI --profile" : (cfg.profile ? "config.profile" : "default");

    const auto cfgv = vanitas::load_user_config_value();

    const fs::path profiles_dir = pm_.profiles_dir();
    const fs::path config_path = vanitas::config_path();

    const auto file_profiles = list_file_profiles(profiles_dir);
    const auto inline_profiles = list_inline_profiles(cfgv);

    std::unordered_set<std::string> inline_set(inline_profiles.begin(), inline_profiles.end());

    std::cout << "Selected: " << selected << " (source: " << selected_src << ")\n";

    std::cout << "\nFrom files (" << profiles_dir.string() << "):\n";
    if (file_profiles.empty()) {
        std::cout << "  (none)\n";
    } else {
        for (const auto &[name, path] : file_profiles) {
            std::cout << "  - " << name << " (path: " << path.string() << ")";
            if (inline_set.count(name)) {
                std::cout << " (shadowed by config)";
            }
            std::cout << "\n";
        }
    }

    std::cout << "\nFrom config (" << config_path.string() << "):\n";
    if (!cfgv) {
        std::cout << "  (no config file)\n";
    } else if (inline_profiles.empty()) {
        std::cout << "  (no [profiles.*] entries)\n";
    } else {
        for (const auto &name : inline_profiles) {
            std::cout << "  - " << name << " (source: config.toml:[profiles." << name << "])\n";
        }
    }

    return 0;
}

} // namespace vanitas::cli
