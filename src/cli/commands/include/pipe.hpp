#pragma once

#include "command.hpp"
#include "vanitas/args_parser.hpp"

namespace vanitas::cli {
class PipeCommand final : public ICommand
{
    public:
        explicit PipeCommand(const Args &a) : args(a) {}
        int execute() override;

    private:
        Args args;
};
} // namespace vanitas::cli
