#include "nordvpn_api_unittest.h"

TEARDOWN(tear_down_test) {
    reset_mock_results();
    nordvpn_close();
}

static void
assert_empty_session() {
    nordvpn_session_ptr session = nordvpn_get_session();
    str empty_str = str_null;
    assert_false(session->is_active);
    assert_memory_equal(sizeof(str), &(session->expiry), &empty_str);
    assert_memory_equal(sizeof(str), &(session->user), &empty_str);
    assert_memory_equal(sizeof(str), &(session->version), &empty_str);
}

static void
assert_empty_user() {
    nordvpn_session_ptr session = nordvpn_get_session();
    str empty_str = str_null;
    assert_memory_equal(sizeof(str), &(session->expiry), &empty_str);
    assert_memory_equal(sizeof(str), &(session->user), &empty_str);
}

static void
assert_filled_session() {
    nordvpn_session_ptr session = nordvpn_get_session();
    assert_true(session->is_active);
    assert_string_equal(str_ptr(session->version), MOCKED_VERSION);
    assert_string_equal(str_ptr(session->user), MOCKED_EMAIL);
    assert_string_equal(str_ptr(session->expiry), MOCKED_EXPIRY);
}

static void
assert_empty_host() {
    nordvpn_host_ptr host = nordvpn_get_host();
    str empty_str = str_null;
    assert_false(host->is_online);
    assert_memory_equal(sizeof(str), &(host->ip), &empty_str);
    assert_memory_equal(sizeof(str), &(host->proto), &empty_str);
    assert_memory_equal(sizeof(str), &(host->hostname), &empty_str);
}

static void
assert_filled_host() {
    nordvpn_host_ptr host = nordvpn_get_host();
    assert_true(host->is_online);
    assert_int(host->country, ==, MOCKED_COUNTRY);
    assert_string_equal(str_ptr(host->hostname), MOCKED_HOSTNAME);
    assert_string_equal(str_ptr(host->ip), MOCKED_IP);
    assert_string_equal(str_ptr(host->last_server), MOCKED_LAST_SERVER);
}

static void
fill_session() {
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = true;
    session->user = str_lit(MOCKED_EMAIL);
    session->expiry = str_lit(MOCKED_EXPIRY);
    session->version = str_lit(MOCKED_VERSION);
}

static void
fill_host() {
    nordvpn_host_ptr host = nordvpn_get_host();
    host->is_online = true;
    host->country = MOCKED_COUNTRY;
    host->ip = str_lit(MOCKED_IP);
    host->hostname = str_lit(MOCKED_HOSTNAME);
    host->proto = str_lit(MOCKED_PROTO);
    host->last_server = str_lit(MOCKED_LAST_SERVER);
}

TEST(test_nordvpn_close) {
    fill_session();
    fill_host();
    nordvpn_close(); // call
    assert_empty_session();
    assert_empty_host();
}

TEST(test_nordvpn_open_success_dc) {
    add_mock_result(OK, MOCKED_VERSION, NARGS("version"));  // first call to nordvpn version
    add_mock_result(OK, MOCKED_ACCOUNT, NARGS("account"));  // second call to nordvpn account
    add_mock_result(OK, MOCKED_DISSTATUS, NARGS("status")); // third call to nordvpn status
    assert_int(nordvpn_open(), ==, OK);                     // call
    assert_filled_session();
    assert_empty_host();
}

TEST(test_nordvpn_open_success_con) {
    add_mock_result(OK, MOCKED_VERSION, NARGS("version"));  // first call to nordvpn version
    add_mock_result(OK, MOCKED_ACCOUNT, NARGS("account"));  // second call to nordvpn account
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status")); // third call to nordvpn status
    assert_int(nordvpn_open(), ==, OK);                     // call
    assert_filled_session();
    assert_filled_host();
}

TEST(test_nordvpn_open_fail_version) {
    add_mock_result(FAILED_EXECUTE, "", NARGS("version")); // first call to nordvpn version
    assert_int(nordvpn_open(), ==, FAILED_EXECUTE);        // call
    assert_empty_session();
    assert_empty_host();
}

TEST(test_nordvpn_open_fail_account) {
    add_mock_result(OK, MOCKED_VERSION, NARGS("version"));  // first call to nordvpn version
    add_mock_result(FAILED_EXECUTE, "", NARGS("account"));  // second call to nordvpn account
    add_mock_result(OK, MOCKED_DISSTATUS, NARGS("status")); // second call to nordvpn account
    assert_int(nordvpn_open(), ==, FAILED_EXECUTE);         // call
    assert_empty_session();
    assert_empty_host();
}

TEST(test_nordvpn_open_fail_status) {
    add_mock_result(OK, MOCKED_VERSION, NARGS("version")); // first call to nordvpn version
    add_mock_result(OK, MOCKED_ACCOUNT, NARGS("account")); // second call to nordvpn account
    add_mock_result(FAILED_EXECUTE, "", NARGS("status"));  // third call to nordvpn status
    assert_int(nordvpn_open(), ==, FAILED_EXECUTE);        // call
    assert_empty_session();
    assert_empty_host();
}

TEST(test_nordvpn_refresh_success) {
    add_mock_result(OK, MOCKED_ACCOUNT, NARGS("account"));  // first call to nordvpn account
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status")); // second call to nordvpn status
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = true;
    session->version = str_lit(MOCKED_VERSION);
    nordvpn_refresh(); // call
    assert_filled_host();
    assert_filled_session();
}

TEST(test_nordvpn_refresh_fail) {
    add_mock_result(OK, MOCKED_ACCOUNT, NARGS("account"));  // first call to nordvpn account
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status")); // second call to nordvpn status
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = false;
    nordvpn_refresh(); // call
    assert_empty_host();
    assert_empty_session();
}

TEST(test_nordvpn_login_success) {
    add_mock_result(OK, MOCKED_GOODLINK, NARGS("login"));
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = true;
    str link = str_null;
    assert_int(nordvpn_login(&link), ==, OK); // call
    assert_string_equal(str_ptr(link), MOCKED_LINK);
}

TEST(test_nordvpn_login_fail_session) {
    add_mock_result(OK, MOCKED_GOODLINK, NARGS("login"));
    str link = str_null;
    assert_int(nordvpn_login(&link), ==, NO_SESSION); // call
    assert_memory_equal(sizeof(str), &link, &str_null);
}

TEST(test_nordvpn_login_fail_already_in) {
    add_mock_result(OK, MOCKED_GOODLINK, NARGS("login"));
    fill_session();
    str link = str_null;
    assert_int(nordvpn_login(&link), ==, ALREADY_LOGGED); // call
    assert_memory_equal(sizeof(str), &link, &str_null);
}

TEST(test_nordvpn_login_fail_execute) {
    add_mock_result(FAILED_EXECUTE, "", NARGS("login"));
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = true;
    str link = str_null;
    assert_int(nordvpn_login(&link), ==, FAILED_EXECUTE); // call
    assert_memory_equal(sizeof(str), &link, &str_null);
}

TEST(test_nordvpn_login_fail_link) {
    add_mock_result(OK, MOCKED_BADLINK, NARGS("login"));
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = true;
    str link = str_null;
    assert_int(nordvpn_login(&link), ==, FAILED_EXECUTE); // call
    assert_memory_equal(sizeof(str), &link, &str_null);
}

TEST(test_nordvpn_logout_success) {
    add_mock_result(OK, MOCKED_LOGOUT, NARGS("logout"));
    add_mock_result(OK, MOCKED_LOGOUT, NARGS("account"));
    add_mock_result(OK, MOCKED_DISSTATUS, NARGS("status"));
    fill_session();
    fill_host();
    assert_int(nordvpn_logout(), ==, OK); // call
    assert_empty_user();
    assert_empty_host();
}

TEST(test_nordvpn_logout_fail_no_session) {
    add_mock_result(OK, MOCKED_LOGOUT, NARGS("logout"));
    assert_int(nordvpn_logout(), ==, NO_SESSION); // call
    assert_empty_session();
    assert_empty_host();
}

TEST(test_nordvpn_logout_fail_already_out) {
    add_mock_result(OK, MOCKED_LOGOUT, NARGS("logout"));
    nordvpn_session_ptr session = nordvpn_get_session();
    session->is_active = true;
    assert_int(nordvpn_logout(), ==, ALREADY_LOGGED); // call
    assert_empty_user();
    assert_empty_host();
}

TEST(test_nordvpn_logout_fail_execute) {
    add_mock_result(FAILED_EXECUTE, "", NARGS("logout"));
    add_mock_result(OK, MOCKED_ACCOUNT, NARGS("account"));
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status"));
    fill_session();
    fill_host();
    assert_int(nordvpn_logout(), ==, FAILED_EXECUTE); // call
    assert_filled_session();
    assert_filled_host();
}

TEST(test_nordvpn_connect_success_quick) {
    add_mock_result(OK, "", NARGS("c"));
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status"));
    fill_session();
    assert_int(nordvpn_connect(), ==, OK);                // call
    assert_int(nordvpn_server_connect(str_null), ==, OK); // call
    assert_filled_session();
    assert_filled_host();
}

TEST(test_nordvpn_connect_success_country) {
    add_mock_result(OK, "", NARGS("c", "Portugal"));
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status"));
    fill_session();
    assert_int(nordvpn_server_connect(NORDVPN_COUNTRY_STR[PORTUGAL]), ==, OK); // call
    assert_filled_host();
}

TEST(test_nordvpn_connect_success_group) {
    add_mock_result(OK, "", NARGS("c", "Double_VPN"));
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status"));
    fill_session();
    assert_int(nordvpn_server_connect(NORDVPN_GROUP_STR[DOUBLE_VPN]), ==, OK); // call
    assert_filled_host();
}

TEST(test_nordvpn_connect_fail_execute) {
    add_mock_result(FAILED_EXECUTE, "", NARGS("c"));
    fill_session();
    assert_int(nordvpn_server_connect(str_null), ==, FAILED_EXECUTE); // call
    assert_empty_host();
}

TEST(test_nordvpn_connect_fail_update) {
    add_mock_result(OK, "", NARGS("c"));
    add_mock_result(FAILED_EXECUTE, "", NARGS("status"));
    fill_session();
    assert_int(nordvpn_server_connect(str_null), ==, FAILED_EXECUTE); // call
    assert_empty_host();
}

TEST(test_nordvpn_disconnect_success_quick) {
    add_mock_result(OK, "", NARGS("d"));
    add_mock_result(OK, MOCKED_DISSTATUS, NARGS("status"));
    fill_session();
    fill_host();
    nordvpn_host_ptr host = nordvpn_get_host();
    assert_int(nordvpn_disconnect(), ==, OK); // call
    assert_empty_host();
    assert_string_equal(str_ptr(host->last_server), MOCKED_LAST_SERVER);
}

TEST(test_nordvpn_disconnect_fail_execute) {
    add_mock_result(FAILED_EXECUTE, "", NARGS("d"));
    fill_session();
    fill_host();
    nordvpn_host_ptr host = nordvpn_get_host();
    assert_int(nordvpn_disconnect(), ==, FAILED_EXECUTE); // call
    assert_filled_host();
}

TEST(test_nordvpn_disconnect_fail_update) {
    add_mock_result(OK, "", NARGS("d"));
    add_mock_result(FAILED_EXECUTE, "", NARGS("status"));
    fill_session();
    fill_host();
    nordvpn_host_ptr host = nordvpn_get_host();
    assert_int(nordvpn_disconnect(), ==, FAILED_EXECUTE); // call
    assert_filled_host();
}

TEST(test_nordvpn_reconnect_success_quick) {
    add_mock_result(OK, "", NARGS("c", MOCKED_LAST_SERVER));
    add_mock_result(OK, MOCKED_CONSTATUS, NARGS("status"));
    fill_session();
    nordvpn_host_ptr host = nordvpn_get_host();
    host->last_server = str_lit(MOCKED_LAST_SERVER);
    assert_int(nordvpn_reconnect(), ==, OK); // call
    assert_filled_host();
}

TEST(test_nordvpn_reconnect_fail_execute) {
    add_mock_result(FAILED_EXECUTE, "", NARGS("c", MOCKED_LAST_SERVER));
    fill_session();
    nordvpn_host_ptr host = nordvpn_get_host();
    host->last_server = str_lit(MOCKED_LAST_SERVER);
    assert_int(nordvpn_reconnect(), ==, FAILED_EXECUTE); // call
    assert_empty_host();
}

TEST(test_nordvpn_reconnect_fail_update) {
    add_mock_result(OK, "", NARGS("c", MOCKED_LAST_SERVER));
    add_mock_result(FAILED_EXECUTE, "", NARGS("status"));
    fill_session();
    nordvpn_host_ptr host = nordvpn_get_host();
    host->last_server = str_lit(MOCKED_LAST_SERVER);
    assert_int(nordvpn_reconnect(), ==, FAILED_EXECUTE); // call
    assert_empty_host();
}

TESTS(api_tests) = {
    TESTRUN("/close-all", test_nordvpn_close),
    TESTRUN("/open-ok-disconnected", test_nordvpn_open_success_dc),
    TESTRUN("/open-ok-connected", test_nordvpn_open_success_con),
    TESTRUN("/open-fail-version", test_nordvpn_open_fail_version),
    TESTRUN("/open-fail-account", test_nordvpn_open_fail_account),
    TESTRUN("/open-fail-status", test_nordvpn_open_fail_status),
    TESTRUN("/refresh-ok-session-on", test_nordvpn_refresh_success),
    TESTRUN("/refresh-fail-no-session", test_nordvpn_refresh_fail),
    TESTRUN("/login-ok-request", test_nordvpn_login_success),
    TESTRUN("/login-fail-session", test_nordvpn_login_fail_session),
    TESTRUN("/login-fail-already-in", test_nordvpn_login_fail_already_in),
    TESTRUN("/login-fail-execute", test_nordvpn_login_fail_execute),
    TESTRUN("/login-fail-badlink", test_nordvpn_login_fail_link),
    TESTRUN("/logout-ok-was-in", test_nordvpn_logout_success),
    TESTRUN("/logout-fail-was-out", test_nordvpn_logout_fail_already_out),
    TESTRUN("/logout-fail-no-session", test_nordvpn_logout_fail_no_session),
    TESTRUN("/logout-fail-execute", test_nordvpn_logout_fail_execute),
    TESTRUN("/connect-ok-quick", test_nordvpn_connect_success_quick),
    TESTRUN("/connect-ok-country", test_nordvpn_connect_success_country),
    TESTRUN("/connect-ok-group", test_nordvpn_connect_success_group),
    TESTRUN("/connect-fail-execute", test_nordvpn_connect_fail_execute),
    TESTRUN("/connect-fail-update", test_nordvpn_connect_fail_update),
    TESTRUN("/disconnect-ok-quick", test_nordvpn_disconnect_success_quick),
    TESTRUN("/disconnect-fail-execute", test_nordvpn_disconnect_fail_execute),
    TESTRUN("/disconnect-fail-update", test_nordvpn_disconnect_fail_update),
    TESTRUN("/reconnect-ok-quick", test_nordvpn_logout_fail_execute),
    TESTRUN("/reconnect-fail-execute", test_nordvpn_logout_fail_execute),
    TESTRUN("/reconnect-fail-update", test_nordvpn_logout_fail_execute),
    TESTEND,
};
