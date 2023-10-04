/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "nordvpn_server.h"

const int COUNTRY_COUNT = (int)NORWAY + 1;

const int GROUP_COUNT = (int)THE_AMERICAS + 1;

const str NORDVPN_COUNTRY_STR[] = {
    str_lit("Albania"),     str_lit("Germany"),       str_lit("Poland"),         str_lit("Argentina"),
    str_lit("Greece"),      str_lit("Portugal"),      str_lit("Australia"),      str_lit("Hong_Kong"),
    str_lit("Romania"),     str_lit("Austria"),       str_lit("Hungary"),        str_lit("Serbia"),
    str_lit("Belgium"),     str_lit("Iceland"),       str_lit("Singapore"),      str_lit("Bosnia_And_Herzegovina"),
    str_lit("Indonesia"),   str_lit("Slovakia"),      str_lit("Brazil"),         str_lit("Ireland"),
    str_lit("Slovenia"),    str_lit("Bulgaria"),      str_lit("Israel"),         str_lit("South_Africa"),
    str_lit("Canada"),      str_lit("Italy"),         str_lit("South_Korea"),    str_lit("Chile"),
    str_lit("Japan"),       str_lit("Spain"),         str_lit("Colombia"),       str_lit("Latvia"),
    str_lit("Sweden"),      str_lit("Costa_Rica"),    str_lit("Lithuania"),      str_lit("Switzerland"),
    str_lit("Croatia"),     str_lit("Luxembourg"),    str_lit("Taiwan"),         str_lit("Cyprus"),
    str_lit("Malaysia"),    str_lit("Thailand"),      str_lit("Czech_Republic"), str_lit("Mexico"),
    str_lit("Turkey"),      str_lit("Denmark"),       str_lit("Moldova"),        str_lit("Ukraine"),
    str_lit("Estonia"),     str_lit("Netherlands"),   str_lit("United_Kingdom"), str_lit("Finland"),
    str_lit("New_Zealand"), str_lit("United_States"), str_lit("France"),         str_lit("North_Macedonia"),
    str_lit("Vietnam"),     str_lit("Georgia"),       str_lit("Norway")};

const str NORDVPN_GROUP_STR[] = {
    str_lit("Africa_The_Middle_East_And_India"),
    str_lit("Onion_Over_VPN"),
    str_lit("Asia_Pacific"),
    str_lit("P2P"),
    str_lit("Double_VPN"),
    str_lit("Standard_VPN_Servers"),
    str_lit("Europe"),
    str_lit("The_Americas"),
};

str
nordvpn_node_from_index(int index) {
    if (index <= 0 || index > COUNTRY_COUNT + GROUP_COUNT) {
        return str_null;
    }
    if (index <= COUNTRY_COUNT) {
        return NORDVPN_COUNTRY_STR[index - 1];
    }
    return NORDVPN_GROUP_STR[index - COUNTRY_COUNT - 1];
}
