#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "mq_ipc.h"

#define PMODE 0655

static mqd_t mClientServer=-1;
static mqd_t mServerClient=-1;

int createMessageQ(const char* fd, int flags)
{
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 512;

    mqd_t handle = mq_open(fd, flags, PMODE, &attr);
    if (handle == -1) {
        printf("Failed to open Q at %s: %s \n", fd, strerror(errno));
        return -1;
    }

    return handle;
}

int sendInternal(const char* payload, int timeoutInSec, bool block)
{
    if (mClientServer == -1) {
        return -1;
    }

    if (block) {
        if (mq_send(mClientServer, payload, strlen(payload)+1, 0) == -1) {
            printf("Failed to send message: %s \n", strerror(errno));
            return -1;
        }else{
            printf("mq message sent.\n");
        }
    } else {
        struct   timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += timeoutInSec;

        if (mq_timedsend(mClientServer, payload, strlen(payload)+1, 0, &tm) == -1) {
            printf("Failed to send message: %s \n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

int receiveInternal(char* buffer, int bufferSize, int timeoutInSec, bool block)
{
    /* mqd_t qFd = getMessageQ(fd); */

    if (mServerClient == -1) {
        return -1;
    }

    if (block) {
        if (mq_receive(mServerClient, buffer, bufferSize, 0) == -1) {
            printf("Failed to receive message: %s \n", strerror(errno));
            return -1;
        }
    } else {
        struct   timespec tm;
        clock_gettime(CLOCK_REALTIME, &tm);
        tm.tv_sec += timeoutInSec;

        if (mq_timedreceive(mServerClient, buffer, bufferSize, 0, &tm) == -1) {
            printf("Failed to receive message: %s \n", strerror(errno));
            return -1;
        }
    }

    return 0;
}

//============== public APIs ===================

int mq_ipc_connect(const char* clientServer, const char* serverClient)
{    
    // Write-only to send message to server
    mClientServer = createMessageQ(clientServer, O_WRONLY);

    // Read-only to receive messages from server.
    mServerClient = createMessageQ(serverClient, O_RDONLY);

    if ((mServerClient == -1 ) || (mClientServer == -1)) {
        printf(" failed to conncet to server \n");
        return -1;
    }else{
        printf("mq connected.\n");
    }
    
    return 0;
}

int mq_ipc_disconnect()
{
    mq_close(mClientServer);
    mq_close(mServerClient);
    return 0;
}

int mq_ipc_send(const char* payload)
{
    return sendInternal(payload, 0, true);
}

int mq_ipc_receive(char* buffer, int bufferSize)
{
    return receiveInternal(buffer, bufferSize, 0, true);
}

int mq_ipc_send_timed(const char* payload, int timeoutInSec)
{
    return sendInternal(payload, timeoutInSec, false);
}

int mq_ipc_receive_timed(char* buffer, int bufferSize, int timeoutInSec)
{
    return receiveInternal(buffer, bufferSize, timeoutInSec, false);
}


