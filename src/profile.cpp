#include "vanitas/profile.hpp"

namespace vanitas {

bool any_match(const std::vector<std::regex> &rs, std::string_view s)
{
    return std::any_of(rs.begin(), rs.end(), [&](const std::regex &r) { return std::regex_search(s.begin(), s.end(), r); });
}

bool any_search(const std::vector<std::regex> &rs, const std::string &s)
{
    for (const auto &r : rs) {
        if (std::regex_search(s, r))
            return true;
    }
    return false;
}

Profile default_profile()
{
    Profile p;

    p.firstline.emplace_back(R"(^ERR\b)");
    p.firstline.emplace_back(R"(^WRN\b)");
    p.firstline.emplace_back(R"(^.*:\d+:\d+:\s+(fatal\s+)?error:\s+)");
    p.firstline.emplace_back(R"(^.*:\d+:\d+:\s+warning:\s+)");

    p.continuation.emplace_back(R"(^\s+)");
    p.continuation.emplace_back(R"(^stack traceback:)");
    p.continuation.emplace_back(R"(^Error executing )");

    return p;
}

} // namespace vanitas
