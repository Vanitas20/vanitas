#include "vanitas/block_builder.hpp"

#include "vanitas/normalizer.hpp"
#include "vanitas/profile.hpp"

namespace vanitas {

bool BlockBuilder::is_firstline(std::string_view s) { return any_match(p_.firstline, s); }
bool BlockBuilder::is_continuation(std::string_view s) { return any_match(p_.continuation, s); }

BlockBuilder::BlockBuilder(const Profile &p) : p_(p), current_(Block{}), has_current_(false) {}

std::vector<Block> BlockBuilder::push(const std::vector<Event> &events)
{
    std::vector<Block> out;

    auto flush_current = [&]() {
        if (has_current_ && !current_.lines.empty()) {
            out.push_back(std::move(current_));
            current_ = Block{};
            has_current_ = false;
        }
    };

    for (const auto &ev : events) {
        if (ev.kind == EvKind::Status) {
            if (has_current_)
                current_.has_status = true;
            continue;
        }

        std::string_view line = ev.text;
        if (line.empty())
            continue;

        if (!has_current_) {
            current_.lines.emplace_back(line);
            has_current_ = true;
            continue;
        }

        if (is_firstline(line)) {
            flush_current();
            current_.lines.emplace_back(line);
            has_current_ = true;
            continue;
        }

        if (is_continuation(line)) {
            current_.lines.emplace_back(line);
            continue;
        }

        flush_current();
        current_.lines.emplace_back(line);
        has_current_ = true;
    }

    return out;
}

std::vector<Block> BlockBuilder::flush()
{
    std::vector<Block> out;
    if (has_current_ && !current_.lines.empty()) {
        out.push_back(std::move(current_));
        current_ = Block{};
        has_current_ = false;
    }
    return out;
}

} // namespace vanitas
