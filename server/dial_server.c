/*
 * Copyright (c) 2014-2020 Netflix, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETFLIX, INC. AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NETFLIX OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dial_data.h"
#include "dial_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "mongoose.h"
#include "url_lib.h"

// TODO: Partners should define this port
#define DIAL_PORT (56789)
#define DIAL_DATA_SIZE (8*1024)

static const char * const gLocalhost = "127.0.0.1";
static const char * const gHttpsProto = "https://";

struct DIALApp_ {
    struct DIALApp_ *next;
    struct DIALAppCallbacks callbacks;
    struct DIALData_ *dial_data;
    void *callback_data;
    DIAL_run_t run_id;
    DIALStatus state;
    char *name;
    char payload[DIAL_MAX_PAYLOAD];
    int useAdditionalData;
    char corsAllowedOrigin[256];

};

typedef struct DIALApp_ DIALApp;

struct DIALServer_ {
    struct mg_context *ctx;
    struct DIALApp_ *apps;
    pthread_mutex_t mux;
};

/**
 * Acquire the DIAL server mutex.
 *
 * @return 1 if acquisition succeeded, 0 if it failed.
 */
static int ds_lock(DIALServer *ds) {
    int err = 0;
    if ((err = pthread_mutex_lock(&ds->mux)) != 0) {
        printf("Unable to acquire DS mutex: [%d]", err);
        return 0;
    }
    return 1;
}

/**
 * Release the DIAL server mutex.
 *
 * @return 1 if release succeeded, 0 if it failed.
 */
static int ds_unlock(DIALServer *ds) {
    int err = 0;
    if ((err = pthread_mutex_unlock(&ds->mux)) != 0) {
        printf("Unable to release DS mutex: [%d]", err);
        return 0;
    }
    return 1;
}

/**
 * Finds an application in the DIAL server application linked list.
 *
 * @param ds the DIAL server.
 * @param app_name application name.
 * @return a pointer to the DIAL application; the pointer value may be NULL if
 *         the application was not found.
 */
static DIALApp **find_app(DIALServer *ds, const char *app_name) {
    DIALApp *app;
    DIALApp **ret = &ds->apps;

    for (app = ds->apps; app != NULL; ret = &app->next, app = app->next) {
        if (!strcmp(app_name, app->name)) {
            break;
        }
    }
    return ret;
}

/**
 * URL-unescape the string, then XML-escape it.
 *
 * @param dst the destination XML-escaped string. Must be large enough for
 *        the resulting string (technically as much as 6x the raw string
 *        length based on the implementation in url_lib.c).
 * @param src the URL-escaped string.
 * @param src_size size of the URL-escaped string, including trailing NULL.
 * @return true if successful, false if out-of-memory.
 */
static int url_decode_xml_encode(char *dst, char *src, size_t src_size) {
    char *url_decoded_key = (char *) malloc(src_size + 1);
    if (url_decoded_key == NULL) {
        return 0;
    }
    urldecode(url_decoded_key, src, src_size);
    xmlencode(dst, url_decoded_key, 2 * src_size);
    free(url_decoded_key);
    return 1;
}

/**
 * Checks if a payload string contains invalid characters.
 *
 * @param pPayload payload string.
 * @param numBytes length of payload string in bytes (excluding trailing NULL).
 *
 * @return 1 if the payload contains an unprintable or non-ASCII character.
 */
static int isBadPayload(const char* pPayload, int numBytes) {
    int i = 0;
    fprintf( stderr, "Payload: checking %d bytes\n", numBytes);
    for (; i < numBytes; i++) {
        // High order bit should not be set
        // 0x7F is DEL (non-printable)
        // Anything under 32 is non-printable
        if (((pPayload[i] & 0x80) == 0x80) || (pPayload[i] == 0x7F)
                || (pPayload[i] <= 0x1F))
            return 1;
    }
    return 0;
}

static void handle_app_start(struct mg_connection *conn,
                             const struct mg_request_info *request_info,
                             const char *app_name,
                             const char *origin_header) {
    char additional_data_param[DIAL_MAX_ADDITIONALURL] = {0, };
    char body[DIAL_MAX_PAYLOAD + sizeof(additional_data_param) + 2] = {0, };
    DIALApp *app;
    DIALServer *ds = request_info->user_data;
    int body_size;

    if (!ds_lock(ds)) {
        mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
        return;
    }
    app = *find_app(ds, app_name);
    if (!app) {
        mg_send_http_error(conn, 404, "Not Found", "Not Found");
    } else {
        body_size = mg_read(conn, body, sizeof(body) - 1);
        if (body_size > DIAL_MAX_PAYLOAD) {
            mg_send_http_error(conn, 413, "413 Request Entity Too Large",
                               "413 Request Entity Too Large");
        } else if (isBadPayload(body, body_size)) {
            mg_send_http_error(conn, 400, "400 Bad Request", "400 Bad Request");
        } else {
            char laddr[INET6_ADDRSTRLEN];
            const struct sockaddr_in *addr =
                    (struct sockaddr_in *) &request_info->local_addr;
            inet_ntop(addr->sin_family, &addr->sin_addr, laddr, sizeof(laddr));
            in_port_t dial_port = DIAL_get_port(ds);

            if (app->useAdditionalData) {
                // Construct additionalDataUrl=http://host:port/apps/app_name/dial_data
                snprintf(additional_data_param, DIAL_MAX_ADDITIONALURL,
                        "additionalDataUrl=http%%3A%%2F%%2Flocalhost%%3A%d%%2Fapps%%2F%s%%2Fdial_data%%3F",
                        dial_port, app_name);
            }
            fprintf(stderr, "Starting the app with params %s\n", body);
            app->state = app->callbacks.start_cb(ds, app_name, body,
                                                 request_info->query_string,
                                                 additional_data_param, 
                                                 &app->run_id,
                                                 app->callback_data);
            if (app->state == kDIALStatusRunning) {
                mg_printf(
                        conn,
                        "HTTP/1.1 201 Created\r\n"
                        "Content-Type: text/plain\r\n"
                        "Location: http://%s:%d/apps/%s/run\r\n"
                        "Access-Control-Allow-Origin: %s\r\n"
                        "\r\n",
                        laddr, dial_port, app_name, origin_header);
                // copy the payload into the application struct
                memset(app->payload, 0, DIAL_MAX_PAYLOAD);
                memcpy(app->payload, body, body_size);
            } else if (app->state == kDIALStatusErrorForbidden) {
                mg_send_http_error(conn, 403, "Forbidden", "Forbidden");
            } else if (app->state == kDIALStatusErrorUnauth) {
                mg_send_http_error(conn, 401, "Unauthorized", "Unauthorized");
            } else if (app->state == kDIALStatusErrorNotImplemented) {
                mg_send_http_error(conn, 501, "Not Implemented", "Not Implemented");
            } else {
                mg_send_http_error(conn, 503, "Service Unavailable",
                                   "Service Unavailable");
            }
        }
    }
    ds_unlock(ds);
}

static void handle_app_status(struct mg_connection *conn,
                              const struct mg_request_info *request_info,
                              const char *app_name,
                              const char *origin_header) {
    DIALApp *app;
    int canStop = 0;
    DIALServer *ds = request_info->user_data;

    // determin client version
    char *clientVersionStr = parse_param(request_info->query_string, "clientDialVer");
    double clientVersion = 0.0;
    if (clientVersionStr){
        clientVersion = atof(clientVersionStr);
        free(clientVersionStr);
    }
    
    if (!ds_lock(ds)) {
        mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
        return;
    }
    app = *find_app(ds, app_name);
    if (!app) {
        mg_send_http_error(conn, 404, "Not Found", "Not Found");
        ds_unlock(ds);
        return;
    }

    char dial_data[DIAL_DATA_SIZE] = {0,};
    char *end = dial_data + DIAL_DATA_SIZE;
    char *p = dial_data;

    for (DIALData* first = app->dial_data; first != NULL; first = first->next) {
        p = smartstrncpy(p, "    <", end - p);
        size_t key_length = strlen(first->key);
        char *encoded_key = (char *) malloc(2 * key_length + 1);
        if (encoded_key == NULL) {
            mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
            ds_unlock(ds);
            return;
        }
        if (!url_decode_xml_encode(encoded_key, first->key, key_length)) {
            mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
            free(encoded_key); encoded_key = NULL;
            ds_unlock(ds);
            return;
        };

        size_t value_length = strlen(first->value);
        char *encoded_value = (char *) malloc(2 * value_length + 1);
        if (encoded_value == NULL) {
            mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
            free(encoded_key); encoded_key = NULL;
            ds_unlock(ds);
            return;
        }
        if (!url_decode_xml_encode(encoded_value, first->value, value_length)) {
            mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
            free(encoded_value); encoded_value = NULL;
            free(encoded_key); encoded_key = NULL;
            ds_unlock(ds);
            return;
        }

        p = smartstrncpy(p, encoded_key, end - p);
        p = smartstrncpy(p, ">", end - p);
        p = smartstrncpy(p, encoded_value, end - p);
        p = smartstrncpy(p, "</", end - p);
        p = smartstrncpy(p, encoded_key, end - p);
        p = smartstrncpy(p, ">", end - p);
        free(encoded_key); encoded_key = NULL;
        free(encoded_value); encoded_value = NULL;
    }

    app->state = app->callbacks.status_cb(ds, app_name, app->run_id, &canStop,
                                          app->callback_data);

    DIALStatus localState = app->state;
    
    // overwrite app->state if cilent version < 2.1    
    if (clientVersion < 2.09 && localState==kDIALStatusHide){
        localState=kDIALStatusStopped;
    }
    
    char dial_state_str[20];
    switch(localState){
    case kDIALStatusHide:
        strcpy (dial_state_str, "hidden");
        break;
    case kDIALStatusRunning:
        strcpy (dial_state_str, "running");
        break;
    default:
        strcpy (dial_state_str, "stopped");
    }
    
    mg_printf(
            conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/xml\r\n"
            "Access-Control-Allow-Origin: %s\r\n"
            "\r\n"
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
            "<service xmlns=\"urn:dial-multiscreen-org:schemas:dial\" dialVer=%s>\r\n"
            "  <name>%s</name>\r\n"
            "  <options allowStop=\"%s\"/>\r\n"
            "  <state>%s</state>\r\n"
            "%s"
            "  <additionalData>\n"
            "%s"
            "\n  </additionalData>\n"
            "</service>\r\n",
            origin_header,            
            DIAL_VERSION,
            app->name,
            canStop ? "true" : "false",
            dial_state_str,
            localState == kDIALStatusStopped ?
                    "" : "  <link rel=\"run\" href=\"run\"/>\r\n",
            dial_data);
    ds_unlock(ds);
}

static void handle_app_stop(struct mg_connection *conn,
                            const struct mg_request_info *request_info,
                            const char *app_name,
                            const char *origin_header) {
    DIALApp *app;
    DIALServer *ds = request_info->user_data;
    int canStop = 0;

    if (!ds_lock(ds)) {
        mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
        return;
    }

    // Special handling for system app
    if (strcmp(app_name, "system") == 0) {
        mg_send_http_error(conn, 403, "Forbidden", "Forbidden");  // Can't stop system app.
    } else {
        app = *find_app(ds, app_name);

        // update the application state
        if (app) {
            app->state = app->callbacks.status_cb(ds, app_name, app->run_id,
                                                  &canStop, app->callback_data);
        }

        if (!app || app->state == kDIALStatusStopped) {
            mg_send_http_error(conn, 404, "Not Found", "Not Found");
        } else {
            app->callbacks.stop_cb(ds, app_name, app->run_id, app->callback_data);
            app->state = kDIALStatusStopped;
            mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/plain\r\n"
                      "Access-Control-Allow-Origin: %s\r\n"
                      "\r\n",
                      origin_header);
        }
    }
    ds_unlock(ds);
}

static void handle_app_hide(struct mg_connection *conn,
                            const struct mg_request_info *request_info,
                            const char *app_name,
                            const char *origin_header) {
    DIALApp *app;
    DIALServer *ds = request_info->user_data;
    int canStop = 0;

    if (!ds_lock(ds)) {
        mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
        return;
    }
    app = *find_app(ds, app_name);
  
    // update the application state
    if (app) {
        app->state = app->callbacks.status_cb(ds, app_name, app->run_id,
                                              &canStop, app->callback_data);
    }
    
    if (!app || (app->state != kDIALStatusRunning && app->state != kDIALStatusHide)) {
        mg_send_http_error(conn, 404, "Not Found", "Not Found");
    } else {
        // not implemented in reference
        DIALStatus status = app->callbacks.hide_cb(ds, app_name, app->run_id, app->callback_data);
        if (status != kDIALStatusHide){
            fprintf(stderr, "Hide not implemented for reference.\n");
            mg_send_http_error(conn, 501, "Not Implemented",
                               "Not Implemented");
        } else {
            app->state = kDIALStatusHide;
            mg_printf(conn, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/plain\r\n"
                      "Access-Control-Allow-Origin: %s\r\n"
                      "\r\n",
                      origin_header);
        }
    }
    ds_unlock(ds);
}

static void handle_dial_data(struct mg_connection *conn,
                             const struct mg_request_info *request_info,
                             const char *app_name,
                             const char *origin_header,
                             int use_payload) {
    char body[DIAL_DATA_MAX_PAYLOAD + 2] = {0, };

    DIALApp *app;
    DIALServer *ds = request_info->user_data;

    if (!ds_lock(ds)) {
        mg_send_http_error(conn, 500, "500 Internal Server Error", "500 Internal Server Error");
        return;
    }
    app = *find_app(ds, app_name);
    if (!app) {
        mg_send_http_error(conn, 404, "Not Found", "Not Found");
        ds_unlock(ds);
        return;
    }
    int nread;
    if (!use_payload) {
        if (request_info->query_string) {
            int qs_len = strlen(request_info->query_string);
            if (qs_len > DIAL_DATA_MAX_PAYLOAD) {
                mg_send_http_error(conn, 413, "413 Request Entity Too Large",
                                   "413 Request Entity Too Large");
                ds_unlock(ds);
                return;
            }
            strncpy(body, request_info->query_string, DIAL_DATA_MAX_PAYLOAD);
            nread = qs_len;
        } else {
          nread = 0;
        }
    } else {
        nread = mg_read(conn, body, DIAL_DATA_MAX_PAYLOAD);
    }
    body[nread] = '\0';

    if (isBadPayload(body, nread)) {
        mg_send_http_error(conn, 400, "400 Bad Request", "400 Bad Request");
        ds_unlock(ds);
        return;
    }


    free_dial_data(&app->dial_data);

    app->dial_data = parse_params(body);
    store_dial_data(app->name, app->dial_data);
    free_dial_data(&app->dial_data);

    mg_printf(conn, "HTTP/1.1 200 OK\r\n"
              "Access-Control-Allow-Origin: %s\r\n"
              "\r\n",
              origin_header);

    ds_unlock(ds);
}

/**
 * Returns true if the origin is acceptable based on the candidate value.
 * The candidate may accept any subdomain if its domain begins with *. and
 * may require an exact port number match if it includes a colon to
 * indicate a port number.
 *
 * This function assumes that the candidate value is well-formed, meaning
 * it will not include invalid characters or a non-numeric port number.
 * 
 * @param origin the origin header value, which must begin with https://
 * @param candidate the authorized origin value, which must begin with
 *        https://
 * @return true if accepted and false if not.
 */
static int host_matches(const char *origin, const char *candidate) {
    // Make sure there is something to compare.
    if (!origin || !candidate)
        return 0;
    
    // Make sure the origin and candidate both begin with HTTPS.
    const size_t https_len= strlen(gHttpsProto);
    if (strncmp(origin, gHttpsProto, https_len) != 0 ||
        strncmp(candidate, gHttpsProto, https_len) != 0)
    {
        return 0;
    }
    
    // For the rest of the check, we only care about the hostname and optional
    // port number.
    const char * origin_host = origin + https_len;
    const char * host = candidate + https_len;

    // Set the initial lengths for comparison.
    size_t origin_len = strlen(origin_host);
    size_t host_len = strlen(host);

    // Look for port numbers.
    const char * origin_colon = strrchr(origin_host, ':');
    const char * host_colon = strrchr(host, ':');

    // If the host contains a port number (indicated by a colon)...
    if (host_colon != NULL) {
        // If the host port number is 443, then accept the origin host if it
        // does not have any port number under the assumption we already
        // verified https://
        if (strlen(host_colon) == 4 &&
            strncmp(host_colon, ":443", 4) == 0 &&
            origin_colon == NULL)
        {
            // We will ignore the host port number of 443.
            host_len = host_colon - host;
        }

        // Other port numbers must match exactly. So leave the host length
        // untouched.
    }

    // Otherwise ignore any port number in the origin.
    else if (origin_colon != NULL) {
       origin_len = origin_colon - origin_host;
    }

    // At this point, the origin length excludes any port number if the host
    // does not specify one, and the host length excludes its port number if
    // it was 443 and there is no origin port number.
    //
    // If either length is zero then fail.
    if (origin_len == 0 || host_len == 0)
        return 0;

    // Check to see if the host permits subdomains.
    const char * wildcard = "*.";
    const int acceptSubdomain = (host_len > strlen(wildcard))
        ? strncmp(host, wildcard, strlen(wildcard)) == 0
        : 0;

    // If the host accepts subdomains, verify that the origin ends with the
    // portion of the host that occurs after the subdomain wildcard.
    if (acceptSubdomain) {
        // The origin must be at least as long as the host.
        if (origin_len < host_len)
            return 0;

        // Skip the subdomain of the origin, which should equate to the
        // wildcard of the host. Likewise skip the wildcard of the host.
        const char * origin_domain = strchr(origin_host, '.');
        const char * host_domain = host + 1;
        if (!origin_domain || !host_domain)
            return 0;

        // Remove from comparison the characters we skipped.
        origin_len -= (origin_domain - origin_host);
        host_len -= 1;

        // The remainder must be an exact match.
        return (origin_len == host_len &&
                strncmp(origin_domain, host_domain, origin_len) == 0);
    }

    // Otherwise the host and origin must be an exact match.
    return (origin_len == host_len &&
            strncmp(origin_host, host, origin_len) == 0);
}

/**
 * Returns true if the origin is acceptable based on the candidate value.
 * The origin must be an exact match to the candidate, unless the
 * candidate is of the form 'scheme://*' or 'scheme:*' in which case
 * everything before the wildcard '*' character must be an exact match but
 * anything is accepted in place of the wildcard.
 *
 * This function assumes that the candidate value is well-formed, meaning
 * it will not include invalid chracters and it will be a valid URI.
 *
 * @param origin the origin header value.
 * @param candidate the authorized origin value.
 * @return true if accepted and false if not.
 */
static int origin_matches(const char *origin, const char *candidate) {
    // Make sure there is something to compare.
    if (!origin || !candidate)
        return 0;

    // If the candidate consists of a scheme followed by wildcard,
    // require an exact match of the scheme specifier.
    size_t origin_len = strlen(origin);
    size_t candidate_len = strlen(candidate);
    if (candidate_len > 1 && candidate[candidate_len - 1] == '*') {
        // The origin must be at least as long as the candidate for a
        // wildcard match to succeed.
        if (origin_len < candidate_len)
            return 0;

        fprintf(stderr, "comparing %s to %s len %zu\n", origin, candidate, candidate_len);
        return strncmp(origin, candidate, candidate_len - 1) == 0;
    }

    // Require an exact match.
    return (origin_len == strlen(candidate) &&
        strncmp(origin, candidate, origin_len) == 0);
}

// str contains a white space separated list of strings (only supports SPACE characters for now)
static int is_uri_in_list(const char *origin, const char *list) {
    // Make sure there is something to compare.
    if (!origin || !list)
        return 0;

    int isHttps = (strncmp(origin, gHttpsProto, strlen(gHttpsProto)) == 0);
    
    const char * scanPointer = list;
    const char * spacePointer;
    unsigned int substringSize = 257;
    char *candidate = (char *)malloc(substringSize);
    if (!candidate) {
        return 0;
    }
    while ((spacePointer = strchr(scanPointer, ' ')) != NULL) {
        int copyLength = spacePointer - scanPointer;      
      
        // protect against buffer overflow
        if (copyLength >= substringSize) {
            substringSize = copyLength + 1;
            free(candidate);
            candidate = (char *)malloc(substringSize);
            if (!candidate) {
                return 0;
            }
        }

        memcpy(candidate, scanPointer, copyLength);
        candidate[copyLength] = '\0';
        //printf("found %s \n", candidate);
        // If the URI begins with https://, perform a host comparison because
        // any port numbers must be handled specially. Otherwise perform a
        // regular match.
        if ((isHttps && host_matches(origin, candidate)) ||
            (!isHttps && origin_matches(origin, candidate)))
        {
            free(candidate); candidate = NULL;
            return 1;
        }
        scanPointer = scanPointer + copyLength + 1; // assumption: only 1 character
    }
    free(candidate); candidate = NULL;
    return ((isHttps && host_matches(origin, scanPointer)) ||
            (!isHttps && origin_matches(origin, scanPointer)));
}

static int is_allowed_origin(DIALServer* ds, char * origin, const char * app_name) {
    fprintf(stderr, "checking %s for %s\n", origin, app_name);

    if (!origin || strlen(origin)==0) {
        return 1;
    }
    
    if (!ds_lock(ds)) {
        // If we can't check, fail in favor of safety.
        return 0;
    }
    DIALApp *app;
    int result = 0;
    for (app = ds->apps; app != NULL; app = app->next) {
        if (!strcmp(app->name, app_name)) {
            if (!app->corsAllowedOrigin[0] ||
                is_uri_in_list(origin, app->corsAllowedOrigin)) {
                result = 1;
                break;
            }
        }
    }
    ds_unlock(ds);

    return result;
}

#define APPS_URI "/apps/"
#define RUN_URI "/run"
#define HIDE_URI "/hide"

static void *options_response(DIALServer *ds, struct mg_connection *conn, char *origin_header, const char* app_name, const char* methods)
{    
    mg_printf(
              conn,
              "HTTP/1.1 204 No Content\r\n"
              "Access-Control-Allow-Methods: %s\r\n"
              "Access-Control-Max-Age: 86400\r\n"
              "Access-Control-Allow-Origin: %s\r\n"
              "Content-Length: 0"
              "\r\n",
              methods,
              origin_header);
    return "done";
}

static void *request_handler(enum mg_event event, struct mg_connection *conn,
                             const struct mg_request_info *request_info) {
    DIALServer *ds = request_info->user_data;

    fprintf(stderr, "Received request %s\n", request_info->uri);
    char *host_header = {0,};
    char *origin_header = {0,};
    for (int i = 0; i < request_info->num_headers; ++i) {
        if (!strcmp(request_info->http_headers[i].name, "Host")  ||
            !strcmp(request_info->http_headers[i].name, "host")) {
            host_header = request_info->http_headers[i].value;
        } else if (!strcmp(request_info->http_headers[i].name, "Origin") ||
                   !strcmp(request_info->http_headers[i].name, "origin")) {
            origin_header = request_info->http_headers[i].value;
        }
    }
    fprintf(stderr, "Origin %s, Host: %s\n", origin_header, host_header);
    if (event == MG_NEW_REQUEST) {
        // URL ends with run
        if (strlen(request_info->uri) > strlen(RUN_URI) + strlen(APPS_URI)
            && !strncmp(request_info->uri + strlen(request_info->uri) - strlen(RUN_URI), RUN_URI, strlen(RUN_URI)))
        {
            // Maximum app name length of 255 characters.
            char app_name[256] = {0, };
            int appname_len = strlen(request_info->uri) - strlen(RUN_URI) - strlen(APPS_URI);
            if (appname_len > 255) {
                appname_len = 255;
            }
            strncpy(app_name, request_info->uri + strlen(APPS_URI), appname_len);

            // Check authorized origins.
            if (origin_header && !is_allowed_origin(ds, origin_header, app_name)) {
                mg_send_http_error(conn, 403, "Forbidden", "Forbidden");
                return "done";
            }

            // Return OPTIONS.
            if (!strcmp(request_info->request_method, "OPTIONS")) {
                return options_response(ds, conn, origin_header, app_name, "DELETE, OPTIONS");
            }

            // DELETE non-empty app name
            if (app_name[0] != '\0'
                && !strcmp(request_info->request_method, "DELETE"))
            {
                handle_app_stop(conn, request_info, app_name, origin_header);
            } else {
                mg_send_http_error(conn, 501, "Not Implemented",
                                   "Not Implemented");
            }
        }
        // URI starts with "/apps/" and is followed by an app name
        else if (strlen(request_info->uri) > strlen(APPS_URI)
                 && !strncmp(request_info->uri, APPS_URI, strlen(APPS_URI))
                 && !strchr(request_info->uri + strlen(APPS_URI), '/'))
        {
            const char *app_name;
            app_name = request_info->uri + strlen(APPS_URI);

            // Check authorized origins.
            if (origin_header && !is_allowed_origin(ds, origin_header, app_name)) {
                mg_send_http_error(conn, 403, "Forbidden", "Forbidden");
                return "done";
            }

            // Return OPTIONS.
            if (!strcmp(request_info->request_method, "OPTIONS")) {
                return options_response(ds, conn, origin_header, app_name, "GET, POST, OPTIONS");
            }

            // start app
            if (!strcmp(request_info->request_method, "POST")) {
                handle_app_start(conn, request_info, app_name, origin_header);
            // get app status
            } else if (!strcmp(request_info->request_method, "GET")) {
                handle_app_status(conn, request_info, app_name, origin_header);
            } else {
                mg_send_http_error(conn, 501, "Not Implemented", "Not Implemented");
            }
        }
        // URI that ends with HIDE_URI
        else if (strlen(request_info->uri) > strlen(HIDE_URI) + strlen(RUN_URI) + strlen(APPS_URI)
                 && !strncmp(request_info->uri + strlen(request_info->uri) - strlen(HIDE_URI), HIDE_URI, strlen(HIDE_URI)))
        {
            // Maximum app name length of 255 characters.
            char app_name[256] = {0, };
            int appname_len = strlen(request_info->uri) - strlen(RUN_URI) - strlen(HIDE_URI) - strlen(APPS_URI);
            if (appname_len > 255) {
                appname_len = 255;
            }
            strncpy(app_name, request_info->uri + strlen(APPS_URI), appname_len);

            // Check authorized origins.
            if (origin_header && !is_allowed_origin(ds, origin_header, app_name)) {
                mg_send_http_error(conn, 403, "Forbidden", "Forbidden");
                return "done";
            }

            // Return OPTIONS.
            if (!strcmp(request_info->request_method, "OPTIONS")) {
                return options_response(ds, conn, origin_header, app_name, "POST, OPTIONS");
            }

            // hide app
            if (app_name[0] != '\0' && !strcmp(request_info->request_method, "POST")) {
                handle_app_hide(conn, request_info, app_name, origin_header);
            } else {
                mg_send_http_error(conn, 501, "Not Implemented", "Not Implemented");
            }
        }
        // URI is of the form */app_name/dial_data
        else if (strstr(request_info->uri, DIAL_DATA_URI)) {
            char laddr[INET6_ADDRSTRLEN];
            const struct sockaddr_in *addr =
                    (struct sockaddr_in *) &request_info->remote_addr;
            inet_ntop(addr->sin_family, &addr->sin_addr, laddr, sizeof(laddr));
            if ( !strncmp(laddr, gLocalhost, strlen(gLocalhost)) ) {
                char *app_name = parse_app_name(request_info->uri);
                if (app_name == NULL) {
                    mg_send_http_error(conn, 500, "Internal Error", "Internal Error");
                } else {
                    // Check authorized origins (still applicable via loopback).
                    if (origin_header && !is_allowed_origin(ds, origin_header, app_name)) {
                        mg_send_http_error(conn, 403, "Forbidden", "Forbidden");
                        return "done";
                    }

                    // Return OPTIONS.
                    if (!strcmp(request_info->request_method, "OPTIONS")) {
                        void *ret = options_response(ds, conn, origin_header, app_name, "POST, OPTIONS");
                        free(app_name);
                        return ret;
                    }

                    // deliver data payload
                    int use_payload = strcmp(request_info->request_method, "POST") ? 0 : 1;
                    handle_dial_data(conn, request_info, app_name, origin_header,
                                     use_payload);

                    free(app_name);
                }
            } else {
                // If the request is not from local host, return an error
                mg_send_http_error(conn, 403, "Forbidden", "Forbidden");
            }
        } else {
            mg_send_http_error(conn, 404, "Not Found", "Not Found");
        }
        return "done";
    } else if (event == MG_EVENT_LOG) {
        fprintf( stderr, "MG: %s\n", request_info->log_message);
        return "done";
    }
    return NULL;
}

DIALServer *DIAL_create() {
    DIALServer *ds = calloc(1, sizeof(DIALServer));
    if (ds == NULL) {
        return NULL;
    }
    if (pthread_mutex_init(&ds->mux, NULL) != 0) {
        free(ds); ds = NULL;
        return NULL;
    }
    return ds;
}

int DIAL_start(DIALServer *ds) {
    ds->ctx = mg_start(&request_handler, ds, DIAL_PORT);
    return (ds->ctx != NULL);
}

void DIAL_stop(DIALServer *ds) {
    mg_stop(ds->ctx);
    pthread_mutex_destroy(&ds->mux);
}

in_port_t DIAL_get_port(DIALServer *ds) {
    struct sockaddr sa;
    socklen_t len = sizeof(sa);
    if (!mg_get_listen_addr(ds->ctx, &sa, &len)) {
        return 0;
    }
    return ntohs(((struct sockaddr_in *) &sa)->sin_port);
}

int DIAL_register_app(DIALServer *ds, const char *app_name,
                      struct DIALAppCallbacks *callbacks, void *user_data,
                      int useAdditionalData,
                      const char* corsAllowedOrigin) {
    DIALApp **ptr, *app;

    if (!ds_lock(ds)) {
        return -1;
    }
    ptr = find_app(ds, app_name);
    if (*ptr != NULL) {  // app already registered
        ds_unlock(ds);
        return 0;
    } else {
        app = malloc(sizeof(DIALApp));
        if (app == NULL) {
            ds_unlock(ds);
            return -1;
        }
        app->callbacks = *callbacks;
        app->name = strdup(app_name);
        if (app->name == NULL) {
            free(app); app = NULL;
            ds_unlock(ds);
            return -1;
        }
        app->next = *ptr;
        app->state = kDIALStatusStopped;
        app->callback_data = user_data;
        app->dial_data = retrieve_dial_data(app->name);
        app->useAdditionalData = useAdditionalData;
        app->corsAllowedOrigin[0] = '\0';
        if (corsAllowedOrigin && strlen(corsAllowedOrigin) < sizeof(app->corsAllowedOrigin)) {
            strcpy(app->corsAllowedOrigin, corsAllowedOrigin);
        } else {
            return -1;
        }
        *ptr = app;
        ds_unlock(ds);
        return 1;
    }
}

int DIAL_unregister_app(DIALServer *ds, const char *app_name) {
    DIALApp **ptr, *app;

    if (!ds_lock(ds)) {
        return -1;
    }
    ptr = find_app(ds, app_name);
    if (*ptr == NULL) {  // no such app
        ds_unlock(ds);
        return 0;
    } else {
        app = *ptr;
        *ptr = app->next;
        free(app->name); app->name = NULL;
        free(app); app = NULL;
        ds_unlock(ds);
        return 1;
    }
}

const char * DIAL_get_payload(DIALServer *ds, const char *app_name) {
    const char * pPayload = NULL;
    DIALApp **ptr, *app;

    // NOTE: Don't grab the mutex as we are calling this function from
    // inside the application callback which already has the lock.
    //ds_lock(ds);
    ptr = find_app(ds, app_name);
    if (*ptr != NULL) {
        app = *ptr;
        pPayload = app->payload;
    }
    //ds_unlock(ds);
    return pPayload;
}

