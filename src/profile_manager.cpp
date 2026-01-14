#include "vanitas/profile_manager.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <toml.hpp>

#include "vanitas/profile.hpp"

namespace vanitas {

struct RawProfile
{
        std::vector<std::string> firstline;
        std::vector<std::string> continuation;
        std::vector<std::string> err;
        std::vector<std::string> wrn;
        std::vector<std::string> tests;
};

static std::filesystem::path get_home_dir()
{
#ifdef _WIN32
    const char *home = std::getenv("USERPROFILE");
#else
    const char *home = std::getenv("HOME");
#endif
    if (!home)
        throw std::runtime_error("HOME/USERPROFILE is not set");
    return std::filesystem::path(home);
}

std::filesystem::path ProfileManager::base_dir() const { return get_home_dir() / ".vanitas"; }
std::filesystem::path ProfileManager::profiles_dir() const { return base_dir() / "profiles"; }

static std::vector<std::string> get_patterns(const toml::value &root, const std::string &table, const std::string &key)
{
    return toml::find<std::vector<std::string>>(root, table, key);
}

static std::vector<std::string> get_patterns_or_empty(const toml::value &root, const std::string &table, const std::string &key)
{
    return toml::find_or(root, table, key, std::vector<std::string>{});
}

void ProfileManager::ensure_default_profiles()
{
    std::filesystem::create_directories(profiles_dir());
    auto def = profiles_dir() / "default.toml";

    if (std::filesystem::exists(def)) {
        return;
    }

    toml::value root(toml::table{
        {   "firstline",                                  toml::table{{"patterns", toml::array{"^ERR\\b", "^WRN\\b", "^ERROR:", "^WARN:"}}}},
        {"continuation",                                               toml::table{{"patterns", toml::array{"^\\s+", "^stack traceback:"}}}},
        {    "classify", toml::table{{"err", toml::array{"^ERR\\b", "\\berror\\b"}},
 {"wrn", toml::array{"^WRN\\b", "\\bwarning\\b"}},
 {"tests", toml::array{"\\bPASSED\\b", "\\bFAILED\\b", "short test summary info"}}}                                },
    });

    std::ofstream f(def);
    if (!f)
        throw std::runtime_error("Cannot create default profile file");

    f << "# default vanitas profile\n";
    f << toml::format(root);
}

static RawProfile parse_profile_value(const toml::value &v)
{
    RawProfile out;
    out.firstline = toml::find_or(v, "firstline", "patterns", std::vector<std::string>{});
    out.continuation = toml::find_or(v, "continuation", "patterns", std::vector<std::string>{});
    out.err = toml::find_or(v, "classify", "err", std::vector<std::string>{});
    out.wrn = toml::find_or(v, "classify", "wrn", std::vector<std::string>{});
    out.tests = toml::find_or(v, "classify", "tests", std::vector<std::string>{});
    return out;
}

static std::vector<std::regex> compile_regex_list(const std::vector<std::string> &patterns, const char *field_name)
{
    std::vector<std::regex> out;
    out.reserve(patterns.size());
    for (const auto &pat : patterns) {
        try {
            out.emplace_back(pat);
        } catch (const std::regex_error &e) {
            throw std::runtime_error(std::string("Invalid regex in ") + field_name + ": '" + pat + "': " + e.what());
        }
    }
    return out;
}

static Profile compile_profile(const RawProfile &raw)
{
    Profile p;
    p.firstline = compile_regex_list(raw.firstline, "firstline.patterns");
    p.continuation = compile_regex_list(raw.continuation, "continuation.patterns");
    p.err = compile_regex_list(raw.err, "classify.err");
    p.wrn = compile_regex_list(raw.wrn, "classify.wrn");
    p.tests = compile_regex_list(raw.tests, "classify.tests");
    return p;
}

static bool is_effectively_empty(const Profile &p)
{
    return p.firstline.empty() && p.continuation.empty() && p.err.empty() && p.wrn.empty() && p.tests.empty();
}

Profile ProfileManager::load(const std::string &name_or_path)
{
    std::filesystem::path p(name_or_path);
    if (!p.has_parent_path()) {
        p = profiles_dir() / (name_or_path + ".toml");
    }

    const auto r = toml::try_parse(p.string());
    if (r.is_err()) {
        std::cerr << "WARN: Failed to parse profile '" << p.string() << "': " << toml::format_error(r.unwrap_err().at(0))
                  << "\nWARN: Falling back to built-in default profile.\n";
        return default_profile();
    }

    try {
        const toml::value v = r.unwrap();
        RawProfile raw = parse_profile_value(v);
        Profile prof = compile_profile(raw);

        if (is_effectively_empty(prof)) {
            std::cerr << "WARN: Profile '" << p.string() << "' has no rules; using built-in default profile.\n";
            return default_profile();
        }
        return prof;
    } catch (const std::regex_error &e) {
        std::cerr << "WARN: Invalid regex in profile '" << p.string() << "': " << e.what()
                  << "\nWARN: Falling back to built-in default profile.\n";
        return default_profile();
    } catch (const std::exception &e) {
        std::cerr << "WARN: Failed to load profile '" << p.string() << "': " << e.what()
                  << "\nWARN: Falling back to built-in default profile.\n";
        return default_profile();
    }
}

} // namespace vanitas
