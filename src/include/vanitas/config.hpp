#pragma once

#include <optional>
#include <string>
#include <toml.hpp>

namespace vanitas {

struct Config
{
        std::optional<std::string> profile;
        bool color = true;
        std::string format = "text";
};

Config load_user_config();
std::optional<toml::value> load_user_config_value();
std::filesystem::path config_path();

} // namespace vanitas
