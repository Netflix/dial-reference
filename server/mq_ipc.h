#ifndef _MQ_IPC_H_
#define _MQ_IPC_H_

#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>
#include <stdbool.h>

int mq_ipc_connect(const char* clientServer, const char* serverClient);
int mq_ipc_disconnect();

int mq_ipc_send(const char* payload);
int mq_ipc_receive(char* buffer, int bufferSize);

int mq_ipc_send_timed(const char* payload, int timeoutInSec);
int mq_ipc_receive_timed(char* buffer, int bufferSize, int timeoutInSec);
    
#endif 
