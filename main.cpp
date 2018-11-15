/* 
 * File:   main.cpp
 * Author: de
 *
 * Created on 
 */

#include "main.h"

void hexdump(char *data, int size) {
    now = localtime(&t);
    printf("%02d-%02d-%02d %02d:%02d:%02d:RX:", now->tm_year + 1900, now->tm_mon + 1,
            now->tm_mday, now->tm_hour,
            now->tm_min, now->tm_sec);
    for (int z = 0; z < size; z++) {
        printf("%02x:", *(data + z) & 0xff);
    }
    printf("\r\n");
}

void getlocalip() {
    char hostname[MAX_HOSTNAME_LEN];
    struct hostent* hostinfo;

    /* lookup local hostname */
    gethostname(hostname, MAX_HOSTNAME_LEN);

    printf("Localhost is %s\n", hostname);

    /* use gethostbyname to get host's IP address */
    if ((hostinfo = gethostbyname(hostname)) == NULL) {
        perror("gethostbyname() failed");
    }
    localip.s_addr = *((unsigned long *) hostinfo->h_addr_list[0]);
    printf("interface# %d\n", sizeof (hostinfo->h_addr_list) / sizeof (hostinfo));
    //printf("address in hex 0x%08x\n", localip.s_addr);
    printf("ip %s\n", inet_ntoa(localip));

}

// open multicasting socket

void mopen(char *multicastIP, unsigned short multicastPort) {

    multicastTTL = 1;
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        perror("socket() failed");

    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &multicastTTL,
            sizeof (multicastTTL)) < 0)
        perror("setsockopt() failed");

    memset(&multicastAddr, 0, sizeof (multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);
    multicastAddr.sin_port = htons(multicastPort);
}
// send multicasting

void msend(char *data, int size) {
    if (
            sendto(sock, data, size, 0, (struct sockaddr *) &multicastAddr, sizeof (multicastAddr))
            != size) {
        perror("sendto() sent a different number of bytes than expected");
    }
}
// close multi and deregister multicasting group

void mclose() {
    close(sock);
}

// open localhost socket with port

void *loserver(void *) {
    cout << "loserver start" << endl;
    /*   struct sockaddr_in losrvaddr, loclientaddr;
       int losrvsock;
       if ((losrvsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
           perror("Failed to create socket");
           exit(EXIT_FAILURE);
       }
       memset(&losrvaddr, 0, sizeof (losrvaddr));
       losrvaddr.sin_family = AF_INET;
       losrvaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
       losrvaddr.sin_port = htons(port);
       char buffer[1024];
       while (1) {
           socklen_t clen = sizeof (loclientaddr);
           if (recvfrom(losrvsock, buffer, sizeof (buffer), 0, 
                   (struct sockaddr *) &losrvaddr, &clen) < 0) {
               perror("server receive fail");
               exit(EXIT_FAILURE);
           }
           cout << "received ****" << endl;
           usleep (100000);
       }
       close (losrvsock); */
}

// open client

void loopen() {
    if ((lsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    memset(&loaddr, 0, sizeof (loaddr));
    loaddr.sin_family = AF_INET;
    //loaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    loaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    loaddr.sin_port = htons(port);
}
// send udp data ot localhost

void losend(char *data, int size) {
    if (sendto(lsock, data, size, 0, (struct sockaddr *) &loaddr, sizeof (loaddr)) != size) {
        perror("local, number of bytes sent");
        exit(EXIT_FAILURE);
    }


}
// receive udp data

void lorec(char * rdata, int size) {
    // recvfrom(lsock,rdata,10000,0,NULL,NULL);
}
// close localhost socket

void loclose() {
    close(lsock);
}

void set_sighup_handler(t_sighup_handler handler) {
    user_sighup_handler = handler;
    printf("exit daemon\n");
    exit(0);
}

void sighup_handler(int sig) {
    got_sighup = 1;
    printf("server stop !!!\n");

}

void setup_sighup(void) {
    struct sigaction act;
    int err;

    act.sa_handler = sighup_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    err = sigaction(SIGHUP, &act, NULL);
    if (err) {
        perror("sigaction");
    }
}

void *readThread(void *) {
    DWORD dwBytesInQueue = 0;
    EVENT_HANDLE eh;
    FT_STATUS ftStatus;
    DWORD Status;
    DWORD BytesReceived;
    DWORD RxBytes;
    DWORD TxBytes;
    DWORD EventDWord;
    char RxBuffer[1024];
    pthread_mutex_init(&eh.eMutex, NULL);
    pthread_cond_init(&eh.eCondVar, NULL);
    // eh = CreateEvent (NULL, false, false, ""); // autoreset, non-signal state
    pthread_mutex_lock(&hmutex);
    ftStatus = FT_SetEventNotification(ftHandle, FT_EVENT_RXCHAR | FT_EVENT_MODEM_STATUS, (PVOID) & eh);
    if (ftStatus != FT_OK) {
        cerr << "error ev" << endl;
        exit(-1);
    }
    pthread_mutex_unlock(&hmutex);
    while (true) {
        pthread_mutex_lock(&hmutex);
        FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
        if (EventDWord & FT_EVENT_MODEM_STATUS) {
            // modem status event detected, so get current modem status
            FT_GetModemStatus(ftHandle, &Status);
            if (Status & 0x00000010) {
                cout << "CTS high" << endl;
            } else {
                cout << "CTS low" << endl;
            }
            if (Status & 0x00000020) {
                cout << "DSR high" << endl;
            } else {
                cout << "DSR low" << endl;
            }
        }
        if (EventDWord & FT_EVENT_LINE_STATUS) {
            // line status change

        }
        if (RxBytes > 0) {
            for (int z = 0; z < sizeof (RxBuffer) / sizeof (char); z++) {
                RxBuffer[z] = 0x00;
            }
            ftStatus = FT_Read(ftHandle, RxBuffer, RxBytes, &BytesReceived);
            if (ftStatus != FT_OK) {
                cerr << "receive error" << endl;
                exit(-1);
            }
            // cout << RxBuffer << endl;
            losend(RxBuffer, BytesReceived);
            hexdump(RxBuffer, BytesReceived);
        }
        pthread_mutex_unlock(&hmutex);
        pthread_mutex_lock(&eh.eMutex);
        pthread_cond_wait(&eh.eCondVar, &eh.eMutex);
        pthread_mutex_unlock(&eh.eMutex);
    }
}

void *writeThread(void *) {
    long count = 0;
    FT_STATUS ftStatus;
    DWORD ret;
    DWORD RxBytes;
    DWORD TxBytes;
    DWORD EventDWord;
    char cline [256];
    for (int z = 0; z < 256; z++) {
        cline[z] = z;
    }
    cout << "size = " << sizeof (cline) / sizeof (char) << endl;
    while (1) {
        // cout << "size = " << sizeof (cline) << endl;
        pthread_mutex_lock(&hmutex);
        ftStatus = FT_Write(ftHandle, cline, sizeof (cline) / sizeof (char), &ret);
        if (ftStatus != FT_OK) {
            cerr << "error writing data" << endl;
            exit(-1);
        }
        FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &EventDWord);
        pthread_mutex_unlock(&hmutex);
        //  cout << "writing data : " << cline << endl;
        usleep(500000);
        //cline[0] = (char) count & 0xff;
        //cline[sizeof (cline) - 3] = (char) count & 0xff;
        count++;
    }
}

void getList() {
    FT_STATUS st;
    DWORD numberdevice;
    int i;
    // get number of attached device from list
    st = FT_ListDevices((PVOID) & numberdevice, NULL, FT_LIST_NUMBER_ONLY);
    if (st != FT_OK) {
        cerr << "error get list" << endl;
        exit(-1);
    }
    char *devicedesc [numberdevice + 1];
    char descblock [64 * 256];
    for (int i = 0; i < numberdevice; ++i)
        devicedesc[i] = descblock + i * 256;
    devicedesc[numberdevice] = NULL;
    st = FT_ListDevices((PVOID) devicedesc, (PVOID) & numberdevice, FT_LIST_ALL | (DWORD) (1 << 1));
    if (st != FT_OK) {
        cerr << "error get list" << endl;
        exit(-1);
    }
    for (int z = 0; z < numberdevice; ++z) {
        cout << "device : " << devicedesc [z] << endl;
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    char portname[6];
    int portbaud;
  //  DWORD portformat;
  //  DWORD datasize = FT_BITS_8;
  //  DWORD stopbit = FT_STOP_BITS_1;
  //  DWORD polarity = FT_PARITY_NONE;

    t = time(0);
    port = 10001;
    setup_sighup();
    now = localtime(&t);
  /*   cout << "start datetime :: "
            << (now->tm_year + 1900) << '-'
            << (now->tm_mon + 1) << '-'
            << now->tm_mday << " "
            << now->tm_hour << ":"
            << now->tm_min << ":"
            << now->tm_sec << " "
            << endl;
    cout << "usage: sio <portname> <baud> <data> <stop> <polarity> <udp-port>" << endl;
    cout << "portname: PRA09A, PRA01B, PRA01C, PRA01D, ...., PRA11A, PRA11B, PRA11C, PRA11D, etc" << endl;
    cout << "baud: 1200,2400,9600,19200,38400,115200" << endl;
    cout << "data= 7/8 , stop=1/2, polarity=odd/even/none" << endl;
    //

    if (strcmp(argv[1], "PRA09A")) {
        printf("com01a\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA09B")) {
        printf("com01b\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA09C")) {
        printf("com01c\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA09D")) {
        printf("com01d\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA10A")) {
        printf("com02a\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA10B")) {
        printf("com02b\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA10C")) {
        printf("com02c\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA10D")) {
        printf("com02d\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA11A")) {
        printf("com03a\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA11B")) {
        printf("com03b\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA11C")) {
        printf("com03c\n");
        strncpy(portname, "PRA09A", 6);
    } else
        if (strcmp(argv[1], "PRA11D")) {
        printf("com03d\n");
        strncpy(portname, "PRA09A", 6);
    } else {
        printf("illegal com port name ");
    }

    portbaud = atoi(argv[2]);
    if (portbaud < 1200) portbaud = 1200;
    if (portbaud != 1200 || portbaud != 2400 ||
            portbaud != 9600 || portbaud
            != 19200 || portbaud != 38400) {
        printf("illegal baudrate");
    }

    if (strcmp(argv[1], "odd")) {
        printf("8n1");

    } else if (strcmp(argv[1], "even")) {
        printf("8e1");
        
    } else if (strcmp(argv[1], "none")) {
        printf("8o1");

    } */
    //
    pthread_t rthread_id;
    pthread_t wthread_id;
    pthread_t srvthread_id;
    // init mux
    pthread_mutex_init(&hmutex, NULL);

    FT_STATUS ftStatus;
    //getList();
    // open       
    char name [] = "PRA09 C";
    ftStatus = FT_OpenEx((PVOID) name, FT_OPEN_BY_DESCRIPTION, &ftHandle);
    if (ftStatus != FT_OK) {
        cerr << "error opening" << endl;
        exit(-1);
    }
    cout << "opened " << endl;
    ftStatus = FT_ResetDevice(ftHandle);
    if (ftStatus != FT_OK) {
        cerr << "error reset" << endl;
        exit(-1);
    }
    ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
    if (ftStatus != FT_OK) {
        cerr << "error clear" << endl;
        exit(-1);
    }
    ftStatus = FT_SetBaudRate(ftHandle, 19200);
    if (ftStatus != FT_OK) {
        cerr << "error set baudrate" << endl;
        exit(-1);
    }
    ftStatus = FT_SetTimeouts(ftHandle, 30000, 0); // receive timeout
    if (ftStatus != FT_OK) {
        cerr << "error set timeout" << endl;
        exit(-1);
    }
    
    ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_EVEN);
    if (ftStatus != FT_OK) {
        cerr << "error set format" << endl;
        exit(-1);
    }
    ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0); // flow control
    if (ftStatus != FT_OK) {
        cerr << "error set flow" << endl;
        exit(-1);
    }
    ftStatus = FT_SetDtr(ftHandle); // set DTR
    if (ftStatus != FT_OK) {
        cerr << "error set dtr" << endl;
        exit(-1);
    }
    ftStatus = FT_SetRts(ftHandle); // set DTR
    if (ftStatus != FT_OK) {
        cerr << "error set rts" << endl;
        exit(-1);
    }
    // get host ip and interface number
    getlocalip();
    pthread_create(&srvthread_id, NULL, loserver, NULL);
    //open multicasting port
    // mopen("230.1.1.1", 10001);
    // lo open
    loopen();
    //send start message to multicast address and port
    char starttext[] = "start...hello...";
    // msend(starttext, sizeof (starttext) / sizeof (char));
    losend(starttext, sizeof (starttext) / sizeof (char));
    // 
    pthread_create(&rthread_id, NULL, readThread, NULL);
    pthread_create(&wthread_id, NULL, writeThread, NULL);
    cout << "waiting...." << endl;

    pthread_join(rthread_id, NULL);
    pthread_join(wthread_id, NULL);
    pthread_join(srvthread_id, NULL);
    cout << "end" << endl;
    pthread_mutex_lock(&hmutex);
    ftStatus = FT_Close(ftHandle);
    pthread_mutex_unlock(&hmutex);
    if (ftStatus != FT_OK) {
        cerr << "error opening" << endl;
        exit(-1);
    }

    pthread_mutex_destroy(&hmutex);
    mclose();
    loclose();
    cout << "closed" << endl;
    return 0;
}


