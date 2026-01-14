#pragma once

#include "command.hpp"
#include "vanitas/args_parser.hpp"

namespace vanitas::cli {
class RunCommand final : public ICommand
{
    public:
        explicit RunCommand(const Args &a) : args(a) {}
        int execute() override;

    private:
        Args args;
};
} // namespace vanitas::cli
