#ifndef NORDVPN_API_UNITTEST_H_
#define NORDVPN_API_UNITTEST_H_

#include "../src/nordvpn_api.c"
#include "nordi_unittest.h"

#define MOCKED_VERSION   "NordVPN Version 3.16.6"
#define MOCKED_EMAIL     "example@mail.org"
#define MOCKED_EXPIRY    "Active (Expires on Jan 1st, 2077)"
#define MOCKED_PROTO     "UDP"
#define MOCKED_IP        "100.200.300.400"
#define MOCKED_HOSTNAME  "ab999.nordvpn.com"
#define MOCKED_COUNTRY   PORTUGAL
#define MOCKED_LINK      "http://nordvpn.com/api/user/loginstuff"
#define MOCKED_GOODLINK  "Proceed here: http://nordvpn.com/api/user/loginstuff\n"
#define MOCKED_BADLINK   "Timed out trying to get link\n"
#define MOCKED_LOGOUT    "logout"
#define MOCKED_DISSTATUS "Status: Disconnected\n"
#define MOCKED_CONSTATUS                                                                                                                   \
    "Status: Connected\n"                                                                                                                  \
    "Hostname: ab999.nordvpn.com\n"                                                                                                        \
    "IP: 100.200.300.400\n"                                                                                                                \
    "Country: Portugal\n"                                                                                                                  \
    "City: Somewhere\n"                                                                                                                    \
    "Current technology: NORDLYNX\n"                                                                                                       \
    "Current protocol: UDP\n"                                                                                                              \
    "Transfer: 100.50 MiB received, 50.10 MiB sent\n"                                                                                      \
    "Uptime: 59 minutes 30 seconds\n"
#define MOCKED_ACCOUNT                                                                                                                     \
    "Account Information:\n"                                                                                                               \
    "Email Address: example@mail.org\n"                                                                                                    \
    "VPN Service: Active (Expires on Jan 1st, 2077)\n"

#define MAX_API_CALLS_ 10

typedef struct {
    int error[MAX_API_CALLS_];
    const char* output[MAX_API_CALLS_];
    const char** args[MAX_API_CALLS_];
    int index;
    int max_index;
} _mock_result_t;

static _mock_result_t _mock_result = {};

#define reset_mock_results() memset(&_mock_result, 0, sizeof(_mock_result_t))
#define add_mock_result(_error, _output, _args)                                                                                            \
    _mock_result.error[_mock_result.max_index] = _error;                                                                                   \
    _mock_result.output[_mock_result.max_index] = _output;                                                                                 \
    _mock_result.args[_mock_result.max_index] = _args;                                                                                     \
    _mock_result.index = 0;                                                                                                                \
    _mock_result.max_index++

nordvpn_error_t
_mock_execute_nordvpn(nordvpn_session_ptr session, char* buffer, const char** args) {
    if (_mock_result.index >= _mock_result.max_index) {
        // rotate if max index is reached
        _mock_result.index = 0;
    }
    int index = _mock_result.index++;
    // assert arguments
    int expected_args = 0, actual_args = 0;
    while (_mock_result.args[index][expected_args++] != NULL) {}
    while (args[actual_args++] != NULL) {}
    assert_int(expected_args, ==, actual_args);
    for (int i = 0; args[i] != NULL; i++) {
        assert_string_equal(args[i], _mock_result.args[index][i]);
    }
    // copy mocked output to buffer
    strcpy(buffer, _mock_result.output[index]);
    // return mocked error
    return (nordvpn_error_t)_mock_result.error[index];
}

#endif /* NORDVPN_API_UNITTEST_H_ */
