#include "vanitas/normalizer.hpp"

namespace vanitas {

std::vector<Event> Normalizer::feed(std::string_view chunk)
{
    std::vector<Event> out;
    for (unsigned char c : chunk) {
        switch (state_) {
        case State::Text:
            if (c == 0x1B) {
                state_ = State::SeenEsc;
                break;
            }
            if (c == '\n') {
                out.push_back({EvKind::Line, line_});
                line_.clear();
                last_was_cr_ = false;
                break;
            }
            if (c == '\r') {
                out.push_back({EvKind::Status, line_});
                line_.clear();
                last_was_cr_ = true;
                break;
            }
            line_.push_back((char)c);
            break;

        case State::SeenEsc:
            if (c == '[') {
                state_ = State::CSI;
            } else {
                state_ = State::Text;
            }
            break;

        case State::CSI:
            if (c >= 0x40 && c <= 0x7E)
                state_ = State::Text;
            break;
        }
    }
    return out;
}

std::vector<Event> Normalizer::flush()
{
    std::vector<Event> out;

    state_ = State::Text;

    if (!line_.empty()) {
        out.push_back(Event{last_was_cr_ ? EvKind::Status : EvKind::Line, line_});
        line_.clear();
        last_was_cr_ = false;
    }

    return out;
}

} // namespace vanitas
