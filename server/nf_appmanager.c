#include <stdio.h>
#include <stdlib.h>
#include "nf_appmanager.h"

DIALStatus am_netflix_start(DIALServer *ds, const char *appname,
                         const char *payload, const char *additionalDataUrl,
                         DIAL_run_t *run_id, void *callback_data)
{
    printf ("it's all wired..\n");
    return kDIALStatusRunning;
}

DIALStatus am_netflix_hide(DIALServer *ds, const char *app_name,
                               DIAL_run_t *run_id, void *callback_data)
{
    return kDIALStatusHide;
}

DIALStatus am_netflix_status(DIALServer *ds, const char *appname,
                          DIAL_run_t run_id, int* pCanStop, void *callback_data)
{
    return kDIALStatusStopped;
}

void am_netflix_stop(DIALServer *ds, const char *appname, DIAL_run_t run_id,
                         void *callback_data)
{
}
