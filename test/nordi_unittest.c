#include "nordi_unittest.h"

SUITES(suites) = {
    SUITE("/nordvpn-server", server_tests),
    SUITE("/nordvpn-api", api_tests),
    SUITE("/nordi-routines", routine_tests),
};

int
main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
    int result = 0;
    for (int suite = 0; suite < sizeof(suites) / sizeof(*suites); suite++) {
        result += munit_suite_main(&(suites[suite]), (void*)"nordi", argc, argv);
    }
    return result;
}
