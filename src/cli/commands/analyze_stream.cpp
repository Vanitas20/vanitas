#include "commands/include/analyze_stream.hpp"
#include <vector>

#include "vanitas/block_builder.hpp"
#include "vanitas/classifier.hpp"
#include "vanitas/normalizer.hpp"

namespace vanitas::cli {

int analyze_stream(std::istream &in, const vanitas::Profile &prof)
{
    vanitas::Normalizer n;
    vanitas::BlockBuilder builder(prof);
    vanitas::Classifier clas;

    std::vector<char> buf(4096);

    while (in) {
        in.read(buf.data(), buf.size());
        std::streamsize s = in.gcount();
        if (s <= 0)
            break;

        auto events = n.feed(std::string_view(buf.data(), (size_t)s));
        auto blocks = builder.push(events);
        vanitas::print_items(clas.classify(blocks));
    }

    auto tail_events = n.flush();
    auto tail_blocks = builder.push(tail_events);
    vanitas::print_items(clas.classify(tail_blocks));

    auto tail_blocks2 = builder.flush();
    vanitas::print_items(clas.classify(tail_blocks2));

    return 0;
}

} // namespace vanitas::cli
