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

        std::filesystem::path base_dir() const;
        std::filesystem::path profiles_dir() const;
};
} // namespace vanitas
