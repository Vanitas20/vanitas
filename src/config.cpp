#include "vanitas/config.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace vanitas {

static std::filesystem::path home_dir()
{
#ifdef _WIN32
    const char *h = std::getenv("USERPROFILE");
#else
    const char *h = std::getenv("HOME");
#endif
    if (!h)
        throw std::runtime_error("HOME/USERPROFILE is not set");
    return std::filesystem::path(h);
}

std::filesystem::path config_path() { return home_dir() / ".vanitas" / "config.toml"; }

Config load_user_config()
{
    Config cfg;

    const auto p = config_path();
    if (!std::filesystem::exists(p)) {
        return cfg;
    }

    const auto r = toml::try_parse(p.string());
    if (r.is_err()) {
        std::cerr << "WARN: Failed to parse config " << p.string() << "\n";
        std::cerr << toml::format_error(r.unwrap_err().at(0));
        std::cerr << "WARN: Using built-in defaults.\n";
        return cfg;
    }

    const toml::value v = r.unwrap();

    try {
        cfg.profile = toml::find<std::string>(v, "profile");
    } catch (...) {
        cfg.profile.reset();
    }

    cfg.color = toml::find_or(v, "color", cfg.color);
    cfg.format = toml::find_or(v, "format", cfg.format);
    return cfg;
}

std::optional<toml::value> load_user_config_value()
{
    const auto p = config_path();
    if (!std::filesystem::exists(p)) {
        return std::nullopt;
    }

    const auto r = toml::try_parse(p.string());
    if (r.is_err()) {
        std::cerr << "WARN: Failed to parse " << p.string() << "\n";
        std::cerr << toml::format_error(r.unwrap_err().at(0)) << "\n";
        return std::nullopt;
    }

    return r.unwrap();
}

} // namespace vanitas
