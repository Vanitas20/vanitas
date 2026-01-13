#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace vanitas {

struct Event; // vanitas::Event
struct Block; // vanitas::Block

enum Type {
    Info,
    Error,
    Warn,
    Tests,
};

struct Item
{
        Type type;
        std::string text;
        std::string details;
};

static void print_items(const std::vector<vanitas::Item> &items)
{
    for (auto &it : items) {
        switch (it.type) {
        case vanitas::Type::Error:
            std::cout << "ERROR: " << it.text << "\n";
            break;
        case vanitas::Type::Warn:
            std::cout << "WARN:  " << it.text << "\n";
            break;
        case vanitas::Type::Tests:
            std::cout << "TESTS: " << it.text << "\n";
            break;
        default:
            std::cout << "INFO:  " << it.text << "\n";
            break;
        }
    }
}

class Classifier
{
    public:
        Classifier();
        std::vector<Item> classify(const std::vector<Block> &blocks);

    private:
        size_t errors_count;
        size_t warn_count;
};
} // namespace vanitas
