#pragma once

#include <regex>
#include <vector>

namespace vanitas {
struct Profile
{
        std::vector<std::regex> firstline;
        std::vector<std::regex> continuation;

        std::vector<std::regex> err;
        std::vector<std::regex> wrn;
        std::vector<std::regex> tests;
};

bool any_match(const std::vector<std::regex> &rs, std::string_view s);
bool any_search(const std::vector<std::regex> &rs, const std::string &s);
Profile default_profile();

} // namespace vanitas
