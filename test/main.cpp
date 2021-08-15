
#include "cli.h"

#include <cxxopts.hpp>

// the entrypoint could live in the test.h, but cxxopts inflates the compilation time drastically, so the cli parsing is separated

int main(int argc, char** argv) {
    cxxopts::Options options(argv[0]);

    options.add_options()
            ("l,list-tests", "List all registered tests")
            ("t,run-tests", "Specify which tests to run", cxxopts::value<std::vector<std::string>>())
            ("h,help", "Print usage")
            ;

    const auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    bool is_listing_tests = result["list-tests"].as<bool>();
    auto run_tests = result.count("run-tests") > 0
            ? result["run-tests"].as<std::vector<std::string>>()
            : std::vector<std::string>{};

    return foreign::test::detail::run_tests(is_listing_tests, run_tests);
}
