#pragma once

#include <string>
#include <vector>

#include "vanitas/profile.hpp"

namespace vanitas {

struct Event; // vanitas::Event

struct Block
{
        std::vector<std::string> lines;
        bool has_status = false;
};

class BlockBuilder
{
    public:
        BlockBuilder(const Profile &p);

        std::vector<Block> flush();
        std::vector<Block> push(const std::vector<Event> &events);

    private:
        const Profile &p_;
        Block current_;
        bool has_current_;

        bool is_firstline(std::string_view s);
        bool is_continuation(std::string_view s);
};

} // namespace vanitas
