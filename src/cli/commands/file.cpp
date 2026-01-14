#include "commands/include/file.hpp"
#include <fstream>
#include <iostream>

#include "commands/include/analyze_stream.hpp"

namespace vanitas::cli {
int FileCommand::execute()
{
    std::ifstream file(args.file, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << args.file << "\n";
        return 1;
    }

    auto prof = vanitas::default_profile();
    return analyze_stream(file, prof);
}
} // namespace vanitas::cli
