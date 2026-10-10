#ifndef MICAMGR_CLI_HPP
#define MICAMGR_CLI_HPP

#include <vector>
#include <string>

namespace micamgr {

class CliHandler {
public:
    static int run(int argc, char* argv[]);
    static void printUsage();
    static void printVersion();
};

} // namespace micamgr

#endif // MICAMGR_CLI_HPP
