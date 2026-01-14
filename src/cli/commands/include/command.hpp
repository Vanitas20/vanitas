#pragma once

namespace vanitas::cli {
struct ICommand
{
        virtual ~ICommand() = default;
        virtual int execute() = 0;
};
} // namespace vanitas::cli
