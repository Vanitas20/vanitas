#pragma once

#include <string>
#include <vector>

namespace vanitas {

enum EvKind {
    Line,
    Status
};

struct Event
{
        EvKind kind;
        std::string text;
};

class Normalizer
{
    public:
        std::vector<Event> feed(std::string_view chunk);
        std::vector<Event> flush();

    private:
        enum class State {
            Text,
            SeenEsc,
            CSI
        };
        State state_ = State::Text;
        std::string line_;
        bool last_was_cr_ = false;
};

} // namespace vanitas
