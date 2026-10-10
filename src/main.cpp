#include <micamgr/cli.hpp>
#include <micamgr/ui.hpp>

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string firstArg = argv[1];
        if (firstArg == "gui" || firstArg == "--gui") {
#ifdef _WIN32
            return micamgr::SoftwareManagerUI::runStandaloneWindow();
#else
            micamgr::CliHandler::printUsage();
            return 0;
#endif
        }
        return micamgr::CliHandler::run(argc, argv);
    }

    // Default desktop launch (e.g. from Start Menu or desktop icon)
#ifdef _WIN32
    return micamgr::SoftwareManagerUI::runStandaloneWindow();
#else
    micamgr::CliHandler::printUsage();
    return 0;
#endif
}
