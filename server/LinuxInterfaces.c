#include "LinuxInterfaces.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Fetch the MACAddress of a network interface */
int getMACAddress(char* ifname, char* macBuf, unsigned int macBufSize)
{
    struct ifreq ifr;
    int sock;
    unsigned char chMAC[6];

    sock=socket(AF_INET,SOCK_DGRAM,0);
    strcpy( ifr.ifr_name, ifname );
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl( sock, SIOCGIFHWADDR, &ifr ) < 0) {
        close(sock);
        return -1;
    }
    memcpy(chMAC, ifr.ifr_hwaddr.sa_data, 6);
    if (macBufSize<32){
        close(sock);
        return -1;
    }else{
        sprintf(macBuf,"%02X:%02X:%02X:%02X:%02X:%02X",chMAC[0],chMAC[1],chMAC[2],chMAC[3],chMAC[4],chMAC[5]);
    }
    close(sock);
    return 0;
}

bool hasRoute(const char *nf, int flag)
{
    bool result = false;
    const char* fileName = "/proc/net/route";
    int r;
    FILE* fp = fopen(fileName, "r");

    if (fp) {
        char line[1024];
        int nfBegin = 0, nfEnd = 0, nfFlag = 0;
        const int nfLen = strlen(nf);
        while (!feof(fp)) {
            if (!fgets(line, sizeof(line), fp))
                break;
            r = sscanf(line, "%n%*s%n %*s %*s %d %*s %*s %*s %*s %*s %*s %*s", &nfBegin, &nfEnd, &nfFlag);
            if (r == 1 && nfBegin == 0 && nfEnd == nfLen && !strncmp(nf, line, nfLen) && (flag == -1 || nfFlag == flag)) {
                result = true;
                break;
            }
        }
        fclose(fp);
    }
    return result;
}

bool isDefault(char *nf)
{
    return hasRoute(nf, 0x03);
}

NetInterface getDefaultNetworkInterfaces()
{
    DIR *d;
    struct dirent *dir;
    d = opendir("/sys/class/net/");
    NetInterface defaultNet;
    if (d){
        while ((dir = readdir(d)) != NULL){
            if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
                if (hasRoute(dir->d_name, -1) && isDefault(dir->d_name)){
                    getMACAddress(dir->d_name, defaultNet.macAddress, MACBUFSIZE);
                    break;                
                }
            }
        }
        closedir(d);
    }
    return defaultNet;
}

