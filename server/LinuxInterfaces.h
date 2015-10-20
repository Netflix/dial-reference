#ifndef LINUXINTERFACES_H_
#define LINUXINTERFACES_H_

#include <arpa/inet.h>
#include <dirent.h>
#include <linux/limits.h>
#include <linux/wireless.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

/* Fetch the MACAddress of a network interface */
#define MACBUFSIZE 32
#define WAKEUPTIMEOUT 15

typedef struct
{
    char macAddress[MACBUFSIZE];
}NetInterface;

NetInterface getDefaultNetworkInterfaces();

#endif 
