#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <limits>
#include <iomanip>
#include <time.h>
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h> 
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>  
#include <arpa/inet.h>

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;

#define BUFSIZE 1024
#define MAX_SEND_BYTES 4096
class teebuf: public std::streambuf
{
public:
    teebuf(std::streambuf * sb1, std::streambuf * sb2)
        : sb1(sb1)
        , sb2(sb2)
    {
    }
private:
    virtual int overflow(int c)
    {
        if (c == EOF)
        {
            return !EOF;
        }
        else
        {
            int const r1 = sb1->sputc(c);
            int const r2 = sb2->sputc(c);
            return r1 == EOF || r2 == EOF ? EOF : c;
        }
    }

    virtual int sync()
    {
        int const r1 = sb1->pubsync();
        int const r2 = sb2->pubsync();
        return r1 == 0 && r2 == 0 ? 0 : -1;
    }
private:
    std::streambuf * sb1;
    std::streambuf * sb2;
};

class teestream : public std::ostream
{
public:
    teestream(std::ostream & o1, std::ostream & o2);
private:
    teebuf tbuf;
};

struct TCP_HEAD
{
    u_char  h1;
    u_char  h2;
    u_short headlen;
    u_short h3;
    u_short head_datalen;
};

//  全局变量TS_CONF存储配置信息
class TsConf
{
public:
    void        init();
    static const char KEY_AUTH[33];
    static const char LOG_PATH[];
    static const char CONF_PATH[];
    static const char CONFIG_PATH[];
    static const char PRO_PATH[];
    static const char USB_PATH[];
    static const char COUNT_PATH[];

    char        IP[20];
    int         PORT;
    int         PERCENT;
    int         EXIT;
    int         MIN_DEV_NUM;
    int         MAX_DEV_NUM;
    int         MIN_TTY_NUM;
    int         MAX_TTY_NUM;
    int         DEL_LOG;
    char        DEBUG[6];
    int         PRINT_DEBUG_INFO;
    int         TIME_GAP;
} ;

class Sender
{
public:
    Sender(){
        buffer = new char[MAX_SEND_BYTES];
        len = 0;
    }

    ~Sender(){
        delete buffer;
    }

    char    *buffer;
    int     len;
};

class CLIENT
{
public:
    CLIENT(int id);
    ~CLIENT();

    void    start();
    void    init();
    void    run();
    int     authentication(char *buf, int socket, u_char &async_num);
    void    version_request(int socket);
    void    send_sys_info_tosvr(int socket );
    void    send_conf_info_tosvr(int socket);
    void    send_pid_info_tosvr(int socket);
    void    send_eth_info_tosvr(int socket,short ethnum);
    void    send_asyn_info_tosvr(int socket);
    void    send_usb_info_tosvr(int socket, u_char  hasUSB);
    void    send_usb_fileinfo_tosvr(int socket);
    void    send_prinet_info_tosvr(int socket, u_char status);
    void    send_prinet_list_tosvr(int socket);
    void    send_tsevser_info_tosvr(int socket, u_char asynum, int &_devnum);
    void    send_scr_info_tosvr(int socket, u_char scrno);
    void    send_tty_info_tosvr(int socket, int &_scrnum);

    int     socketInit();
    void    tcp_print(const char *tcp, int size);
    int     tcp_send(int s, char *buf, int len);

    u_short readfile(const char* filename, u_char *buff, u_short &len);
    void    get_randstr(u_char str[], int len);
    void    mempush(char *src, int srclen);
    std::string getTime();

private:
    TCP_HEAD    *tcp_head_svr;
    teestream   *logout;
    Sender      sender;
    std::ofstream log;

    int         clientfd;
    int         devid;
    int         stdout_copy;

    bool        finished;
};


#endif // CLIENT_H
