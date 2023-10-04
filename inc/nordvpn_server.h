/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#ifndef NORDVPN_NODES_H_
#define NORDVPN_NODES_H_

#include "str.h"

/**
 * @brief Enum representing the available NordVPN country options.
 */
typedef enum {
    ALBANIA,
    GERMANY,
    POLAND,
    ARGENTINA,
    GREECE,
    PORTUGAL,
    AUSTRALIA,
    HONG_KONG,
    ROMANIA,
    AUSTRIA,
    HUNGARY,
    SERBIA,
    BELGIUM,
    ICELAND,
    SINGAPORE,
    BOSNIA_AND_HERZEGOVINA,
    INDONESIA,
    SLOVAKIA,
    BRAZIL,
    IRELAND,
    SLOVENIA,
    BULGARIA,
    ISRAEL,
    SOUTH_AFRICA,
    CANADA,
    ITALY,
    SOUTH_KOREA,
    CHILE,
    JAPAN,
    SPAIN,
    COLOMBIA,
    LATVIA,
    SWEDEN,
    COSTA_RICA,
    LITHUANIA,
    SWITZERLAND,
    CROATIA,
    LUXEMBOURG,
    TAIWAN,
    CYPRUS,
    MALAYSIA,
    THAILAND,
    CZECH_REPUBLIC,
    MEXICO,
    TURKEY,
    DENMARK,
    MOLDOVA,
    UKRAINE,
    ESTONIA,
    NETHERLANDS,
    UNITED_KINGDOM,
    FINLAND,
    NEW_ZELAND,
    UNITED_STATES,
    FRANCE,
    NORTH_MACEDONIA,
    VIETNAM,
    GEORGIA,
    NORWAY
} nordvpn_country_t;

/**
 * @brief Enum representing the available NordVPN node group options.
 */
typedef enum {
    AFRICA_MID_EAST_INDIA,
    ONION_OVER_VPN,
    ASIA_PACIFIC,
    P2P,
    DOUBLE_VPN,
    STANDARD_SERVERS,
    EUROPE,
    THE_AMERICAS
} nordvpn_group_t;

/**
 * @brief The number of country options.
 */
extern const int COUNTRY_COUNT;

/**
 * @brief The number of group options.
 */
extern const int GROUP_COUNT;

/**
 * @brief The string value of all available NordVPN countries in `nordvpn_country_t`.
 */
extern const str NORDVPN_COUNTRY_STR[];

/**
 * @brief The string value of all available NordVPN groups in `nordvpn_group_t`.
 */
extern const str NORDVPN_GROUP_STR[];

/**
 * @brief Converts the given index into a str object with the corresponding server name, if the index is inside the server count.
 * @return The server name if index matches any COUNTRY or GROUP index, otherwise an empty str.
 */
str nordvpn_node_from_index(int);

#endif /* NORDVPN_NODES_H_ */