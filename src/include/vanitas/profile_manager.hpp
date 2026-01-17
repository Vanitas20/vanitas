#pragma once

#include <filesystem>
#include <toml.hpp>

#include "vanitas/profile.hpp"

namespace vanitas {

class ProfileManager
{
    public:
        void ensure_default_profiles();
        Profile load(const std::string &name_or_path);
        Profile load_from_value(const Profile &base, const toml::value &v);

        std::optional<toml::value> try_load_file_value(const std::string &name_or_path);

        struct ProfileValueSource
        {
                toml::value value;
                std::string source;
        };

        std::optional<ProfileValueSource> try_load_profile_value(const std::string &name, const std::optional<toml::value> &cfgv);
        toml::value merge_profile_values(const toml::value &base, const toml::value &overlay);
        Profile load_effective(const std::string &name, const std::optional<toml::value> &cfgv);

        std::filesystem::path base_dir() const;
        std::filesystem::path profiles_dir() const;
};
} // namespace vanitas
