#include "nordi_unittest.h"

int
main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    SUITES(suites) = {
        SUITE("/nordvpn-server", server_tests),
        SUITE("/nordvpn-api", api_tests),
    };
    int suite = 0;
    int max_suites = sizeof(suites) / sizeof(*suites);
    int result = 0;
    while (suite < max_suites) {
        result += munit_suite_main(&(suites[suite]), (void*)"nordi", argc, argv);
        suite++;
    }
    return result;
}
