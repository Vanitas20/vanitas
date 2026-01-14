#include "commands/include/pipe.hpp"
#include <iostream>

#include "commands/include/analyze_stream.hpp"

namespace vanitas::cli {
int PipeCommand::execute() { return vanitas::cli::analyze_stream(std::cin, prof_); }
} // namespace vanitas::cli
