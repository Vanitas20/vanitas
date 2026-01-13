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

Classifier::Classifier() : errors_count(0), warn_count(0) {}

std::vector<Item> Classifier::classify(const std::vector<Block> &blocks)
{
    std::vector<Item> out;
    out.reserve(blocks.size());

    static const std::regex re_level_err(R"(^ERR\b)");
    static const std::regex re_level_wrn(R"(^WRN\b)");

    static const std::regex re_error_cc(R"(^.*:\d+:\d+:\s+(fatal\s+)?error:\s+.*)");
    static const std::regex re_warn_cc(R"(^.*:\d+:\d+:\s+warning:\s+.*)");

    static const std::regex re_pytest(R"((\bPASSED\b)|(\bFAILED\b)|(short test summary info))");

    static const std::regex re_indent(R"(^\s+)");
    static const std::regex re_trace_hdr(R"(^stack traceback:)");
    static const std::regex re_lua_err(R"(^Error executing lua:)");
    static const std::regex re_generic_err_word(R"(\berror\b)");

    Type last = Type::Info;

    for (const auto &bl : blocks) {
        if (bl.lines.empty())
            continue;

        const std::string &head = bl.lines.front();
        std::string all = join_lines(bl.lines);

        // 1) ERR/WRN
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

        // 2) Pytest
        if (std::regex_search(all, re_pytest)) {
            out.push_back({Type::Tests, head, all});
            continue;
        }

        // 3) clang/gcc style
        if (std::regex_match(head, re_error_cc)) {
            ++errors_count;
            out.push_back({Type::Error, head, all});
            continue;
        }
        if (std::regex_match(head, re_warn_cc)) {
            ++warn_count;
            out.push_back({Type::Warn, head, all});
            continue;
        }

        // 4) Default
        out.push_back({Type::Info, head, all});
    }

    return out;
}
} // namespace vanitas
