#include "commands/include/pipe.hpp"
#include <iostream>

#include "commands/include/analyze_stream.hpp"

namespace vanitas::cli {
int PipeCommand::execute()
{
    auto prof = vanitas::default_profile();
    return vanitas::cli::analyze_stream(std::cin, prof);
}
} // namespace vanitas::cli
