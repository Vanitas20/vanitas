// commands/src/run.cpp
#include "commands/include/run.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "vanitas/block_builder.hpp"
#include "vanitas/classifier.hpp"
#include "vanitas/normalizer.hpp"

namespace vanitas::cli {

int RunCommand::execute()
{
    if (args.cmd.empty()) {
        std::cerr << "Usage: vanitas run [--profile <name>] -- <cmd> [args...]\n";
        return 2;
    }

    int p[2];
    if (pipe(p) != 0) {
        std::cerr << "run: pipe() failed: " << std::strerror(errno) << "\n";
        return 1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "run: fork() failed: " << std::strerror(errno) << "\n";
        close(p[0]);
        close(p[1]);
        return 1;
    }

    if (pid == 0) {
        close(p[0]);

        if (dup2(p[1], STDOUT_FILENO) < 0)
            _exit(127);
        if (dup2(p[1], STDERR_FILENO) < 0)
            _exit(127);
        close(p[1]);

        std::vector<char *> argv;
        argv.reserve(args.cmd.size() + 1);
        for (auto &s : args.cmd)
            argv.push_back(const_cast<char *>(s.c_str()));
        argv.push_back(nullptr);

        execvp(argv[0], argv.data());
        _exit(127);
    }

    close(p[1]);

    FILE *in = fdopen(p[0], "rb");
    if (!in) {
        std::cerr << "run: fdopen() failed: " << std::strerror(errno) << "\n";
        close(p[0]);
        return 1;
    }

    vanitas::Normalizer norm;
    vanitas::BlockBuilder builder(prof_);
    vanitas::Classifier classifier(prof_);

    std::vector<char> buf(4096);
    while (true) {
        size_t n = fread(buf.data(), 1, buf.size(), in);
        if (n == 0)
            break;

        auto events = norm.feed(std::string_view(buf.data(), n));
        auto blocks = builder.push(events);
        vanitas::print_items(classifier.classify(blocks));
    }

    {
        auto tail_events = norm.flush();
        auto tail_blocks = builder.push(tail_events);
        vanitas::print_items(classifier.classify(tail_blocks));
    }
    {
        auto tail_blocks2 = builder.flush();
        vanitas::print_items(classifier.classify(tail_blocks2));
    }

    fclose(in);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        std::cerr << "run: waitpid() failed: " << std::strerror(errno) << "\n";
        return 1;
    }

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    if (WIFSIGNALED(status))
        return 128 + WTERMSIG(status);
    return 1;
}

} // namespace vanitas::cli
