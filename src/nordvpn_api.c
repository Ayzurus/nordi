/**
 * Copyright (c) 2023 Ayzurus
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "nordvpn_api.h"

#define MAX_BUFFER         1024
#define STATUS_LINE_COUNT  7
#define ACCOUNT_LINE_COUNT 3
#define UNIQUE_LINE_COUNT  1
#define DELIM              str_lit(": ")
#define PIPEIN             1
#define PIPEOUT            0

// fields
#define F_STATUS           0
#define F_HOSTNAME         1
#define F_IP               2
#define F_                 3

// Macro to join list of strings into array of NordVPN arguments
#define NARGS(...)         ((const char*[]){NORDVPN, __VA_ARGS__, NULL})

// When API mock is enabled, change the NordVPN binary calls to a mock function
#ifdef NORDVPN_API_UNITTEST_H_
nordvpn_error_t _mock_execute_nordvpn(nordvpn_session_ptr, char*, const char**);
#define execute_nordvpn(...) _mock_execute_nordvpn(__VA_ARGS__)
#else
#define execute_nordvpn(...) _execute_nordvpn(__VA_ARGS__)
#endif

// Error messages
static const char* ERROR_MESSAGES[] = {"OK", "An unknown/unidentified error occurred", "The nordvpn binary was not found",
                                       "Failed to execute a command on nordvpn", "Failed to read the result of a nordvpn command"};

// remove ending and leading carriage-returns from string
static char*
str_trim_cr(char* source) {
    str cr = str_lit("\r");
    int end = strlen(source) - 1;
    while (str_has_suffix(str_ref(source), cr)) {
        source[end] = 0;
        end--;
    }
    return source[0] == '\r' ? (strrchr(source, '\r') + 1) : source;
}

// Splits a given C string in lines and stores them into an array of str objects
static int
str_split_lines(char* cstr, str out_strs[], int max_lines) {
    if (cstr == NULL) {
        return 0;
    }
    int line_count = 0;
    // check carriage-returns (due to nordvpn output loading gimmick)
    char* cstr_start = str_trim_cr(cstr);
    // replace all new-lines with null terminations to segment the string
    for (int i = 0; cstr[i] != 0 && line_count < max_lines; i++) {
        if (cstr[i] != '\n') {
            continue;
        }
        cstr[i] = 0;
        out_strs[line_count] = str_acquire(cstr_start);
        cstr_start += str_len(out_strs[line_count]) + 1;
        line_count++;
    }
    // output whole string as single line if no lines are found on expected multi line strings
    if (line_count == 0 && max_lines > 0) {
        out_strs[0] = str_acquire(cstr_start);
        line_count++;
    }
    return line_count;
}

// Search str object for a sub string
static char*
str_contains(str source, str target) {
    return strstr(str_ptr(source), str_ptr(target));
}

// Returns the second half of the split on the first occurrence of delim
static str
str_split_value(str source, str delim) {
    char* sub_source = str_contains(source, delim);
    if (sub_source == NULL) {
        return str_null;
    }
    return str_ref(sub_source + str_len(delim));
}

// Returns the first half of the split on the first occurrence of delim
static str
str_split_key(str source, str delim) {
    str value = str_split_value(source, delim);
    return str_ref_chars(str_ptr(source), str_len(source) - (str_len(value) + str_len(delim)));
}

nordvpn_session_ptr
nordvpn_get_session() {
    static nordvpn_session_t session = {
        .version = str_null,
    };
    return &session;
}

nordvpn_host_ptr
nordvpn_get_host() {
    static nordvpn_host_t host = {
        .ip = str_null,
        .hostname = str_null,
        .last_server = str_null,
        .proto = str_null,
    };
    return &host;
}

str
nordvpn_error(nordvpn_error_t error) {
    return str_ref(ERROR_MESSAGES[error]);
}

// Run a NordVPN command and fill the given buffer with its output
static nordvpn_error_t
_execute_nordvpn(nordvpn_session_ptr session, char* buffer, const char* arguments[]) {
    int child_pid = fork();
    if (child_pid < 0) {
        return FAILED_FORK;
    }
    if (child_pid == 0) {
        if (dup2(session->pipe[PIPEIN], STDOUT_FILENO) >= 0 && dup2(session->pipe[PIPEIN], STDERR_FILENO) >= 0) {
            execv(NORDVPN, arguments);
        }
        // either dup2 failed or execv failed if the child process arrives here
        perror("ERROR");
        fsync(session->pipe[PIPEIN]);
        exit(EXIT_FAILURE);
    }
    int status = 0;
    waitpid(child_pid, &status, 0);
    if (!WIFEXITED(status)) {
        return FAILED_EXECUTE;
    }
    if (read(session->pipe[PIPEOUT], buffer, MAX_BUFFER) < 0) {
        return FAILED_READ;
    }
    if (str_has_prefix(str_ref(buffer), str_lit("ERROR:"))) {
        return FAILED_EXECUTE;
    }
    return OK;
}

// Update the host data for the given session
static nordvpn_error_t
nordvpn_update_status(nordvpn_session_ptr session) {
    nordvpn_host_ptr host = nordvpn_get_host();
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    nordvpn_error_t result = execute_nordvpn(session, buffer, NARGS("status"));
    if (result != OK) {
        return result;
    }
    str output[STATUS_LINE_COUNT];
    int output_lines = str_split_lines(buffer, output, STATUS_LINE_COUNT);
    host->is_online = output_lines > 1 && str_has_suffix(output[0], str_lit("Connected"));
    if (host->is_online) {
        str_cpy(&(host->hostname), str_split_value(output[1], DELIM));
        str_cpy(&(host->last_server), str_split_key(host->hostname, str_lit(".")));
        str_cpy(&(host->ip), str_split_value(output[2], DELIM));
        str country = str_split_value(output[3], DELIM);
        str_cpy(&(host->proto), str_split_value(output[STATUS_LINE_COUNT - 1], DELIM));
        for (int n = 0; n < COUNTRY_COUNT; n++) {
            if (str_eq(country, NORDVPN_COUNTRY_STR[n])) {
                host->country = (nordvpn_country_t)n;
            }
        }
    } else {
        str_clear(&(host->hostname));
        str_clear(&(host->ip));
        str_clear(&(host->proto));
    }
    return OK;
}

// Update the account data for the given session
static nordvpn_error_t
nordvpn_update_account(nordvpn_session_ptr session) {
    if (!session->is_active) {
        return NO_SESSION;
    }
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    nordvpn_error_t result = execute_nordvpn(session, buffer, NARGS("account"));
    if (result != OK) {
        return result;
    }
    str output[ACCOUNT_LINE_COUNT];
    if (str_split_lines(buffer, output, ACCOUNT_LINE_COUNT) == ACCOUNT_LINE_COUNT) {
        str_cpy(&(session->user), str_split_value(output[1], DELIM));
        str_cpy(&(session->expiry), str_split_value(output[2], DELIM));
    } else {
        str_clear(&(session->user));
        str_clear(&(session->expiry));
    }
    return OK;
}

nordvpn_error_t
nordvpn_open() {
    // Setup a session and update NordVPN version info
    nordvpn_session_ptr session = nordvpn_get_session();
    if (pipe(session->pipe) < 0) {
        return FAILED_PIPE;
    }
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    nordvpn_error_t result = execute_nordvpn(session, buffer, NARGS("version"));
    if (result != OK) {
        return result;
    }
    str output[UNIQUE_LINE_COUNT];
    int output_lines = str_split_lines(buffer, output, UNIQUE_LINE_COUNT);
    if (output_lines <= 0 || !str_has_prefix(output[0], str_lit("NordVPN"))) {
        return UNKNOWN_ERROR;
    }
    str_cpy(&(session->version), output[0]);
    session->is_active = true;
    // Synchronize with NordVPN data
    result = nordvpn_update_account(session) + nordvpn_update_status(session);
    if (result != OK) {
        nordvpn_close(); // close session if synchronization fails
    }
    return result;
}

void
nordvpn_close() {
    nordvpn_session_ptr session = nordvpn_get_session();
    if (!session->is_active) {
        return;
    }
    str_clear(&(session->user));
    str_clear(&(session->expiry));
    str_clear(&(session->version));
    close(session->pipe[PIPEIN]);
    close(session->pipe[PIPEOUT]);
    session->is_active = false;
    nordvpn_host_ptr host = nordvpn_get_host();
    if (host->is_online) {
        str_clear(&(host->ip));
        str_clear(&(host->hostname));
        str_clear(&(host->last_server));
        str_clear(&(host->proto));
    }
    host->is_online = false;
}

void
nordvpn_refresh() {
    nordvpn_session_ptr session = nordvpn_get_session();
    if (session->is_active) {
        nordvpn_update_account(session);
        nordvpn_update_status(session);
    }
}

nordvpn_error_t
nordvpn_login(str* out_link) {
    *out_link = str_null;
    nordvpn_session_ptr session = nordvpn_get_session();
    if (!session->is_active) {
        return NO_SESSION;
    }
    if (!str_is_empty(session->user)) {
        return ALREADY_LOGGED;
    }
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    nordvpn_error_t result = execute_nordvpn(session, buffer, NARGS("login"));
    if (result != OK) {
        return result;
    }
    str output[UNIQUE_LINE_COUNT];
    int output_lines = str_split_lines(buffer, output, UNIQUE_LINE_COUNT);
    if (output_lines <= 0) {
        return FAILED_READ;
    }
    str_cpy(out_link, str_split_value(output[0], DELIM));
    if (!str_has_prefix(*out_link, str_lit("http://"))) {
        str_clear(out_link);
        return FAILED_EXECUTE;
    }
    return OK;
}

nordvpn_error_t
nordvpn_logout() {
    nordvpn_session_ptr session = nordvpn_get_session();
    if (!session->is_active) {
        return NO_SESSION;
    }
    if (str_is_empty(session->user)) {
        return ALREADY_LOGGED;
    }
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    nordvpn_error_t result = execute_nordvpn(session, buffer, NARGS("logout"));
    nordvpn_refresh();
    if (result != OK && !str_is_empty(session->user)) {
        return result;
    }
    return OK;
}

nordvpn_error_t
nordvpn_connect() {
    return nordvpn_server_connect(str_null);
}

nordvpn_error_t
nordvpn_server_connect(str server) {
    nordvpn_session_ptr session = nordvpn_get_session();
    if (!session->is_active) {
        return NO_SESSION;
    }
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    const char** arguments = str_is_empty(server) ? NARGS("c") : NARGS("c", str_ptr(server));
    nordvpn_error_t result = execute_nordvpn(session, buffer, arguments);
    if (result != OK) {
        return result;
    }
    return nordvpn_update_status(session);
}

nordvpn_error_t
nordvpn_reconnect() {
    nordvpn_host_ptr host = nordvpn_get_host();
    return nordvpn_server_connect(host->last_server);
}

nordvpn_error_t
nordvpn_disconnect() {
    nordvpn_session_ptr session = nordvpn_get_session();
    if (!session->is_active) {
        return NO_SESSION;
    }
    char buffer[MAX_BUFFER];
    memset(buffer, 0, MAX_BUFFER);
    nordvpn_error_t result = execute_nordvpn(session, buffer, NARGS("d"));
    if (result != OK) {
        return result;
    }
    return nordvpn_update_status(session);
}
