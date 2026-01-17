#include "vanitas/profile_manager.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <toml.hpp>
#include <unordered_set>

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

static std::optional<std::string> try_get_extends(const toml::value &v)
{
    try {
        return toml::find<std::string>(v, "extends");
    } catch (...) {
        return std::nullopt;
    }
}

static RawProfile parse_raw_profile(const toml::value &v) { return parse_profile_value(v); }

std::optional<toml::value> ProfileManager::try_load_file_value(const std::string &name_or_path)
{
    std::filesystem::path p(name_or_path);
    if (!p.has_parent_path()) {
        p = profiles_dir() / (name_or_path + ".toml");
    }

    if (!std::filesystem::exists(p)) {
        return std::nullopt;
    }

    const auto r = toml::try_parse(p.string());
    if (r.is_err()) {
        return std::nullopt;
    }
    return r.unwrap();
}

std::optional<ProfileManager::ProfileValueSource> ProfileManager::try_load_profile_value(const std::string &name,
                                                                                         const std::optional<toml::value> &cfgv)
{
    if (cfgv) {
        try {
            toml::value pv = toml::find(*cfgv, "profiles", name);
            return ProfileValueSource{pv, "config.toml:[profiles." + name + "]"};
        } catch (...) {
        }
    }

    std::filesystem::path p = profiles_dir() / (name + ".toml");
    if (auto v = try_load_file_value(name)) {
        return ProfileValueSource{*v, p.string()};
    }

    return std::nullopt;
}

toml::value ProfileManager::merge_profile_values(const toml::value &base, const toml::value &overlay)
{
    toml::value out = base;

    auto apply = [&](const std::string &table, const std::string &key) {
        const auto v = toml::find<std::optional<std::vector<std::string>>>(overlay, table, key);
        if (!v)
            return;

        if (!out.contains(table)) {
            out[table] = toml::table{};
        }
        if (!out.at(table).is_table()) {
            out[table] = toml::table{};
        }
        out[table][key] = *v;
    };

    apply("firstline", "patterns");
    apply("continuation", "patterns");
    apply("classify", "err");
    apply("classify", "wrn");
    apply("classify", "tests");

    return out;
}

Profile ProfileManager::load_effective(const std::string &name, const std::optional<toml::value> &cfgv)
{
    std::unordered_set<std::string> stack;

    std::function<toml::value(const std::string &)> resolve = [&](const std::string &cur) -> toml::value {
        if (stack.count(cur)) {
            throw std::runtime_error("Profile extends cycle detected at: " + cur);
        }
        stack.insert(cur);

        toml::value overlay = toml::table{};
        if (auto src = try_load_profile_value(cur, cfgv)) {
            overlay = src->value;
        } else {
            throw std::runtime_error("Profile not found: " + cur);
        }

        const std::string base_name = try_get_extends(overlay).value_or("default");

        toml::value base;
        if (cur == "default" && !try_get_extends(overlay).has_value()) {
            base = toml::value(toml::table{});
        } else {
            base = resolve(base_name);
        }

        toml::value merged = merge_profile_values(base, overlay);

        stack.erase(cur);
        return merged;
    };

    toml::value effv = resolve(name);
    RawProfile raw = parse_raw_profile(effv);
    Profile prof = compile_profile(raw);

    if (is_effectively_empty(prof)) {
        std::cerr << "WARN: Effective profile '" << name << "' has no rules; using built-in default profile.\n";
        return default_profile();
    }

    return prof;
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

Profile ProfileManager::load_from_value(const Profile &base, const toml::value &v)
{
    Profile out = base;

    auto apply = [&](std::vector<std::regex> &dst, const std::optional<std::vector<std::string>> &pats, const char *field_name) {
        if (!pats)
            return;
        dst = compile_regex_list(*pats, field_name);
    };

    const auto firstline = toml::find<std::optional<std::vector<std::string>>>(v, "firstline", "patterns");
    const auto continuation = toml::find<std::optional<std::vector<std::string>>>(v, "continuation", "patterns");
    const auto err = toml::find<std::optional<std::vector<std::string>>>(v, "classify", "err");
    const auto wrn = toml::find<std::optional<std::vector<std::string>>>(v, "classify", "wrn");
    const auto tests = toml::find<std::optional<std::vector<std::string>>>(v, "classify", "tests");

    apply(out.firstline, firstline, "firstline.patterns");
    apply(out.continuation, continuation, "continuation.patterns");
    apply(out.err, err, "classify.err");
    apply(out.wrn, wrn, "classify.wrn");
    apply(out.tests, tests, "classify.tests");

    return out;
}

} // namespace vanitas
