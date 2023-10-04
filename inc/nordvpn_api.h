/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#ifndef NORDVPN_API_H_
#define NORDVPN_API_H_

#include <stdbool.h>
#include "nordvpn_server.h"
#include "str.h"

/**
 * @brief nordvpn binary location.
 */
#define NORDVPN "/usr/bin/nordvpn"

/**
 * @brief The error codes of the API.
 */
typedef enum {
    OK = 0,         // no error
    UNKNOWN_ERROR,  // error not listed
    NO_SESSION,     // session not initialised
    NOT_FOUND,      // binary not found
    ALREADY_LOGGED, // logging in when already logged in
    FAILED_PIPE,    // failed creating the pipe for the binary
    FAILED_FORK,    // failed forking process
    FAILED_EXECUTE, // binary execution failed
    FAILED_READ     // failed reading the binary output
} nordvpn_error_t;

typedef struct {
    bool is_online;
    nordvpn_country_t country;
    str ip;
    str name;
    str proto;
} nordvpn_host_t;

typedef struct {
    str version;
    int pipe[2];
    bool is_active;
    str user;
    str expiry;
} nordvpn_session_t;

typedef nordvpn_session_t* nordvpn_session_ptr;
typedef nordvpn_host_t* nordvpn_host_ptr;

/**
 * @brief Getter for the singleton NordVPN session data object.
 */
nordvpn_session_ptr nordvpn_get_session();

/**
 * @brief Getter for the singleton NordVPN host data object.
 */
nordvpn_host_ptr nordvpn_get_host();

/**
 * @brief Starts the session and synchronizes state with NordVPN binary.
 * @return 0 if no error occurred, otherwise, the error code.
 */
nordvpn_error_t nordvpn_open();

/**
 * @brief Converts an API error status into a string message.
 * @param error The error code to convert.
 */
str nordvpn_error(nordvpn_error_t);

/**
 * @brief Closes session and frees objects used with the NordVPN state.
 */
void nordvpn_close();

/**
 * @brief Updates login and status information.
 */
void nordvpn_refresh();

/**
 * @brief Requests a link to log in to NordVPN.
 * @param out_link The str object to be filled with the login HTTP link for NordVPN. 
 * @return 0 if no error occurs, the error code otherwise.
 */
nordvpn_error_t nordvpn_login(str*);

/**
 * @brief Attempts to log out from NordVPN, if logged in.
 * @return 0 if no error occurs, the error code otherwise.
 */
nordvpn_error_t nordvpn_logout();

/**
 * @brief Attempts to quickly connect to NordVPN's closest server.
 * @return 0 if no error occurs, the error code otherwise.
 */
nordvpn_error_t nordvpn_connect();

/**
 * @brief Attempts to connect to the NordVPN's specified server or specialty server.
 * @param server The server name to connect to.
 * @return 0 if no error occurs, the error code otherwise.
 */
nordvpn_error_t nordvpn_server_connect(str);

/**
 * @brief Disconnects from any current NordVPN server.
 * @return 0 if no error occurs, the error code otherwise.
 */
nordvpn_error_t nordvpn_disconnect();

#endif /* NORDVPN_API_H_ */
