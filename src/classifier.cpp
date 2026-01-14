#include <regex>

#include "vanitas/block_builder.hpp"
#include "vanitas/classifier.hpp"

namespace vanitas {

static std::string join_lines(const std::vector<std::string> &lines)
{
    std::string out;
    for (size_t i = 0; i < lines.size(); ++i) {
        out += lines[i];
        if (i + 1 < lines.size())
            out += '\n';
    }
    return out;
}

Classifier::Classifier(const Profile &p) : p_(p), errors_count(0), warn_count(0) {}

std::vector<Item> Classifier::classify(const std::vector<Block> &blocks)
{
    std::vector<Item> out;
    out.reserve(blocks.size());

    static const std::regex re_level_err(R"(^ERR\b)");
    static const std::regex re_level_wrn(R"(^WRN\b)");

    for (const auto &bl : blocks) {
        if (bl.lines.empty())
            continue;

        const std::string &head = bl.lines.front();
        std::string all = join_lines(bl.lines);

        // 1) fast-path
        if (std::regex_search(head, re_level_err)) {
            ++errors_count;
            out.push_back({Type::Error, head, all});
            continue;
        }
        if (std::regex_search(head, re_level_wrn)) {
            ++warn_count;
            out.push_back({Type::Warn, head, all});
            continue;
        }

        // 2) profile rules
        if (!p_.err.empty() && any_search(p_.err, head)) {
            ++errors_count;
            out.push_back({Type::Error, head, all});
            continue;
        }
        if (!p_.wrn.empty() && any_search(p_.wrn, head)) {
            ++warn_count;
            out.push_back({Type::Warn, head, all});
            continue;
        }
        if (!p_.tests.empty() && any_search(p_.tests, all)) {
            out.push_back({Type::Tests, head, all});
            continue;
        }

        // 3) default
        out.push_back({Type::Info, head, all});
    }
    return out;
}
} // namespace vanitas
