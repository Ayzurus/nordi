#include "nordvpn_server_unittest.h"

TEARDOWN(tear_down_test) {}

TEST(test_nordvpn_empty_index_ok) { assert_string_equal(str_ptr(nordvpn_node_from_index(0)), str_ptr(str_null)); }

TEST(test_nordvpn_country_index_ok) {
    assert_string_equal(str_ptr(nordvpn_node_from_index(ALBANIA + 1)), str_ptr(NORDVPN_COUNTRY_STR[ALBANIA]));
    assert_string_equal(str_ptr(nordvpn_node_from_index(NORWAY + 1)), str_ptr(NORDVPN_COUNTRY_STR[NORWAY]));
}

TEST(test_nordvpn_group_index_ok) {
    assert_string_equal(str_ptr(nordvpn_node_from_index(COUNTRY_COUNT + 1)), str_ptr(NORDVPN_GROUP_STR[AFRICA_MID_EAST_INDIA]));
    assert_string_equal(str_ptr(nordvpn_node_from_index(COUNTRY_COUNT + GROUP_COUNT)), str_ptr(NORDVPN_GROUP_STR[THE_AMERICAS]));
}

TEST(test_nordvpn_index_out_range) {
    assert_string_equal(str_ptr(nordvpn_node_from_index(-1)), str_ptr(str_null));
    assert_string_equal(str_ptr(nordvpn_node_from_index(COUNTRY_COUNT + GROUP_COUNT + 1)), str_ptr(str_null));
}

TESTS(server_tests) = {
    TESTRUN("/empty-index-ok", test_nordvpn_empty_index_ok),
    TESTRUN("/country-index-ok", test_nordvpn_country_index_ok),
    TESTRUN("/group-index-ok", test_nordvpn_group_index_ok),
    TESTRUN("/out-range-index-ok", test_nordvpn_index_out_range),
    TESTEND,
};
