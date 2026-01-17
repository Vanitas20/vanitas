#pragma once

#include "command.hpp"
#include "vanitas/args_parser.hpp"
#include "vanitas/profile_manager.hpp"

namespace vanitas::cli {
class ProfileCommand final : public ICommand
{
    public:
        explicit ProfileCommand(const vanitas::Args &args, vanitas::ProfileManager &pm) : args_(args), pm_(pm) {}
        int execute() override;

    private:
        const vanitas::Args &args_;
        const vanitas::ProfileManager &pm_;
};
} // namespace vanitas::cli
