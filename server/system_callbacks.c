#include <string.h>
#include <stdio.h>
#include "system_callbacks.h"

extern char spSleepPassword[];

DIALStatus system_start(DIALServer *ds, const char *appname, const char *payload, const char* query_string,
                        const char *additionalDataUrl, DIAL_run_t *run_id, void *callback_data) {

    /* Can't launch system app */
    if (0 == query_string) {
        return kDIALStatusErrorForbidden;
    }

    /* Only sleep is supported action */
    if (0 != strncmp( query_string, "action=sleep", sizeof("action=sleep") - 1)) {
        return kDIALStatusErrorNotImplemented;   // Only "sleep" is valid action
    }

    if (strlen(spSleepPassword) != 0) {

        /* Look for key */
        char *key_value;
        if ( (key_value = strchr(query_string, '&')) == '\0' ) {
            return kDIALStatusErrorForbidden;   // No key specified.
        }

        /* Look for sleep password */
        *key_value++ = '\0';
        char str[512];
        snprintf(str, 512, "key=%s", spSleepPassword);
        printf(" str: %s \n", str);
        if (0 != strncmp( key_value, "key=TEST", sizeof("key=TEST") - 1)) {
            return kDIALStatusErrorUnauth;  // Invalid key
        }
    }
    /* Sleep not implemented in reference implementation */
    return kDIALStatusErrorNotImplemented;
}

DIALStatus system_hide(DIALServer *ds, const char *app_name,
                       DIAL_run_t *run_id, void *callback_data) {
    // Always hidden
    return kDIALStatusHide;
}

DIALStatus system_status(DIALServer *ds, const char *appname,
                         DIAL_run_t run_id, int* pCanStop, void *callback_data) {
    // Always hidden
    return kDIALStatusHide;
}


