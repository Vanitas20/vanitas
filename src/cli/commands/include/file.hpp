#pragma once

#include "command.hpp"
#include "vanitas/args_parser.hpp"
#include "vanitas/profile.hpp"

namespace vanitas::cli {
class FileCommand final : public ICommand
{
    public:
        explicit FileCommand(const vanitas::Args &a, const vanitas::Profile &prof) : args(a), prof_(prof) {}
        int execute() override;

    private:
        const vanitas::Args &args;
        const vanitas::Profile &prof_;
};
} // namespace vanitas::cli
