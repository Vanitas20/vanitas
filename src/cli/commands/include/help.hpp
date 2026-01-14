#pragma once

#include "command.hpp"
#include "vanitas/args_parser.hpp"

namespace vanitas::cli {
class HelpCommand final : public ICommand
{
    public:
        explicit HelpCommand(const Args &a) : args(a) {}
        int execute() override;

    private:
        Args args;
};
} // namespace vanitas::cli
