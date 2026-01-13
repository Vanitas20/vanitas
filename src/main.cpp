#include <fstream>
#include <iostream>
#include <vector>

#include "vanitas/block_builder.hpp"
#include "vanitas/classifier.hpp"
#include "vanitas/normalizer.hpp"
#include "vanitas/profile.hpp"
// #include "vanitas/profile_manager.hpp"

int main(int argc, char *const argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    // vanitas::ProfileManager profile_manager;
    vanitas::Profile def = vanitas::default_profile();

    vanitas::Normalizer n;
    vanitas::Classifier clas;
    vanitas::BlockBuilder builder(def);

    std::vector<char> buf(4096);
    while (true) {
        file.read(buf.data(), buf.size());
        std::streamsize s = file.gcount();
        if (s <= 0)
            break;

        auto events = n.feed(std::string_view(buf.data(), (size_t)s));
        auto blocks = builder.push(events);
        auto items = clas.classify(blocks);

        vanitas::print_items(items);
    }

    auto tail_events = n.flush();
    auto tail_blocks = builder.push(tail_events);
    print_items(clas.classify(tail_blocks));

    auto tail_blocks2 = builder.flush();
    print_items(clas.classify(tail_blocks2));

    return 0;
}
