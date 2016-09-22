#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nf_appmanager.h"
#include "mq_ipc.h"
#include "jsmn.h"
#include "url_lib.h"

#define IPC_BUF_SIZE 5*1024   // account for 4kb payload
#define IPC_TIMEOUT       2

static char *defaultLaunchParam = "source_type=12";
static char * mq_send_ch = "/fromAppManager";
static char * mq_receive_ch = "/toAppManager";
static char * mq_tx_msg_base = " {\"args\":\"%s\",\"message\":%d}";

static int s_run_id = 0;

typedef enum ControllerCommand {
    //-----------------------------------
    //  Commands sent to AM
    //-----------------------------------
    CMD_CNT_START,             // Launch the app
    CMD_CNT_HIDE,            // Suspend app
    CMD_CNT_START_FROM_HIDE,             // Resume app
    CMD_CNT_STOP=6,     // Terminate app nicely because the user killed Netflix.
    CMD_CNT_GET_STATUS=8,         // Provide status of the app
}ControllerCommand_t;

typedef enum ControllerResp {
    //-----------------------------------
    //  Resp sent from AM
    //-----------------------------------
    RESP_OK,
    RESP_ERR,
}ControllerResp_t;

typedef enum NetflixStatus {
    //-----------------------------------
    // Netflix status
    //-----------------------------------
    STATUS_NETFLIX_RUNNING,
    STATUS_NETFLIX_HIDE1,
    STATUS_NETFLIX_HIDE2,
    STATUS_NETFLIX_STOPPED,
}NetflixStatus_t;


void errorMsg()
{
    printf("\nIn this mode, a matching application manager must be started and listen on the appropriate IPC.\n\n");
    exit(1);
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
      return 0;
	}
	return -1;
}

NetflixStatus_t parse_nf_state(char* json_str, jsmntok_t* tokens, int num_tokens)
{
    for (int i=0; i<num_tokens; i++){
        if (jsoneq(json_str, &tokens[i], "state") == 0) {
            char * tempStr = strndup(json_str+tokens[i+1].start, tokens[i+1].end - tokens[i+1].start);
            printf ("token: %s\n", tempStr);
            NetflixStatus_t status = (NetflixStatus_t) (atoi(tempStr));
            free(tempStr);
            return status;
        }
    }
    return STATUS_NETFLIX_STOPPED;
}

DIALStatus get_nf_state()
{
    //connect
    if (mq_ipc_connect(mq_send_ch, mq_receive_ch))
        exit(1);
    //send status request
    char tx_buf[IPC_BUF_SIZE];
    snprintf(tx_buf, sizeof(tx_buf), mq_tx_msg_base, "", CMD_CNT_GET_STATUS);
    if (mq_ipc_send_timed(tx_buf, IPC_TIMEOUT))
        errorMsg();
    //recieve status
    char rx_buf[IPC_BUF_SIZE];
    if(mq_ipc_receive_timed(rx_buf, sizeof(rx_buf), IPC_TIMEOUT))
        errorMsg();
    ATRACE("recieved: \n %s\n", rx_buf);

    mq_ipc_disconnect();
        
    // parse response
    jsmn_parser json_parser;
    jsmntok_t tokens[32]; /* We expect no more than 32 tokens */
    int num_tokens;
    jsmn_init(&json_parser);
    num_tokens = jsmn_parse(&json_parser, rx_buf, strlen(rx_buf), tokens, 32);

    // get status
    NetflixStatus_t am_status = parse_nf_state(rx_buf, tokens, num_tokens);
        
    switch (am_status){
    case STATUS_NETFLIX_HIDE1:
    case STATUS_NETFLIX_HIDE2:
        // hide state
        return kDIALStatusHide;
        break;
    case STATUS_NETFLIX_STOPPED:
        // stopped state
        return kDIALStatusStopped;
        break;
    case STATUS_NETFLIX_RUNNING:
        // running state
        return kDIALStatusRunning;
        break;        
    default:
        // error state
        exit(1);
    }

    return kDIALStatusStopped;
}

ControllerResp_t parse_am_status(char* buf)
{
    jsmn_parser json_parser;
    jsmntok_t tokens[32]; /* We expect no more than 32 tokens */
    int num_tokens;
    jsmn_init(&json_parser);
    num_tokens = jsmn_parse(&json_parser, buf, strlen(buf), tokens, 32);

    for (int i=0; i<num_tokens; i++){
        if (jsoneq(buf, &tokens[i], "state") == 0) {
            char * tempStr = strndup(buf+tokens[i+1].start, tokens[i+1].end - tokens[i+1].start);
            printf ("token: %s\n", tempStr);
            ControllerResp_t status = (ControllerResp_t) (atoi(tempStr));
            free(tempStr);
            return status;
        }
    }
    return RESP_ERR;
}

/******************* public API ********************/
DIALStatus am_netflix_start(DIALServer *ds, const char *appname,
                         const char *payload, const char *additionalDataUrl,
                         DIAL_run_t *run_id, void *callback_data)
{
    DIALStatus current_state = get_nf_state();
    ATRACE("current state: %d\n", current_state);
    
    ATRACE ("start netflix via app manager..\n");
    
    char tx_buf[IPC_BUF_SIZE];
    char rx_buf[IPC_BUF_SIZE];
    mq_ipc_connect(mq_send_ch, mq_receive_ch);

    char sQueryParam[DIAL_MAX_PAYLOAD+DIAL_MAX_ADDITIONALURL+40];
    memset( sQueryParam, 0, sizeof(sQueryParam) );
    strcat( sQueryParam, "-Q ");
    strcat( sQueryParam, defaultLaunchParam );
    if(strlen(payload)){
        char * pUrlEncodedParams;
        pUrlEncodedParams = url_encode( payload );
        if( pUrlEncodedParams ){
            strcat( sQueryParam, "&dial=");
            strcat( sQueryParam, pUrlEncodedParams );
            free( pUrlEncodedParams );
        }
    }
    if(strlen(additionalDataUrl)){
        strcat(sQueryParam, "&");
        strcat(sQueryParam, additionalDataUrl);
    }
    
    switch (current_state) {
    case kDIALStatusHide:
        // resume Netflix
        snprintf(tx_buf, sizeof(tx_buf), mq_tx_msg_base, sQueryParam, CMD_CNT_START_FROM_HIDE);
        break;
    case kDIALStatusStopped:
        // start Netflix
        s_run_id++;
        snprintf(tx_buf, sizeof(tx_buf), mq_tx_msg_base, sQueryParam, CMD_CNT_START);
        break;
    default:
        // Already running.  Just get status
        snprintf(tx_buf, sizeof(tx_buf), mq_tx_msg_base, "", CMD_CNT_GET_STATUS);
        break;
    }
    
    if (mq_ipc_send_timed(tx_buf, IPC_TIMEOUT))
        errorMsg();
    if (mq_ipc_receive_timed(rx_buf, sizeof(rx_buf), IPC_TIMEOUT))
        errorMsg();
    ATRACE("recieved: \n %s\n", rx_buf);

    mq_ipc_disconnect();
    
    return get_nf_state();
}

DIALStatus am_netflix_hide(DIALServer *ds, const char *app_name,
                               DIAL_run_t *run_id, void *callback_data)
{
    //connect
    if (mq_ipc_connect(mq_send_ch, mq_receive_ch))
        exit(1);
    //send status request
    char tx_buf[IPC_BUF_SIZE];
    snprintf(tx_buf, sizeof(tx_buf), mq_tx_msg_base, "", CMD_CNT_HIDE);
    if (mq_ipc_send_timed(tx_buf, IPC_TIMEOUT))
        errorMsg();
    //recieve status
    char rx_buf[IPC_BUF_SIZE];
    if(mq_ipc_receive_timed(rx_buf, sizeof(rx_buf), IPC_TIMEOUT))
        errorMsg();
    ATRACE("recieved: \n %s\n", rx_buf);
    mq_ipc_disconnect();

    return get_nf_state();
}

DIALStatus am_netflix_status(DIALServer *ds, const char *appname,
                          DIAL_run_t run_id, int* pCanStop, void *callback_data)
{
    return get_nf_state();
}

void am_netflix_stop(DIALServer *ds, const char *appname, DIAL_run_t run_id,
                         void *callback_data)
{
    //connect
    if (mq_ipc_connect(mq_send_ch, mq_receive_ch))
        exit(1);
    //send status request
    char tx_buf[IPC_BUF_SIZE];
    snprintf(tx_buf, sizeof(tx_buf), mq_tx_msg_base, "", CMD_CNT_STOP);
    if (mq_ipc_send_timed(tx_buf, IPC_TIMEOUT))
        errorMsg();
    //recieve status
    char rx_buf[IPC_BUF_SIZE];
    if(mq_ipc_receive_timed(rx_buf, sizeof(rx_buf), IPC_TIMEOUT))
        errorMsg();
    ATRACE("recieved: \n %s\n", rx_buf);
    mq_ipc_disconnect();    
}
