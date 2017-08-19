#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <iomanip>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>

/*******************这两个类用来同时输出信息到文件和终端，参考自google***********/
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

    // Sync both teed buffers.
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

class CLIENT
{
public:
    CLIENT(int s):sockfd(s){
        is_auth = false;
        memset(recvbuf, 0, 4096);
        recv_res = 0;
        recv_full_tcp = 8;
        jobs = 1;
        termnum = 0;
        scrnum = 0;
    }

    int  sockfd;
    int  jobs;          //record the jobs to be done
    bool is_auth;       //has auth or not
    char ip[20];        //ip address of the client
    char recvbuf[4096]; //to recv data
    int  recv_res;      //the len of data of last recv
    int  recv_full_tcp; //the size of a full tcp data
    int  termnum;
    int  scrnum;

    void mysql_write();

    struct TCP_AUTH_CLIENT
    {
        TCP_HEAD    head;
        short   base_cpu ;
        short   base_ram ;
        short   base_flash ;
        short   base_inid;
        u_char  base_groupid[16];
        u_char  base_type[16];
        u_char  base_version[16];
        u_char  base_ethnum;
        u_char  base_syncum;
        u_char  base_asyncnum;
        u_char  base_switchnum;
        u_char  base_usbnum;
        u_char  base_prnum;
        u_char  pad[2];
        u_int   base_devid;
        u_short base_seq;
        u_char  pad1[2];
        u_char  base_authstr[32];
        u_int   base_randnum;
    }   *auth;

    struct  TCP_SYS_INFO
    {
        TCP_HEAD head;
        u_int   cput_1;
        u_int   cput_2;
        u_int   cput_3;
        u_int   cput_4;
        u_int   freemem;
    }  *sysinfo;

    struct  TCP_CONF_INFO
    {
        TCP_HEAD head;
        char     conf[8191 + 1];
    }  *confinfo;

    struct  TCP_PID_INFO
    {
        TCP_HEAD head;
        char    pro[8191 + 1];
    }  *pidinfo;

    struct  TCP_ETH_INFO
    {
        TCP_HEAD head;
        u_char  isexists;
        u_char  isconf;
        u_char  isup;
        u_char  pad;
        u_char  mac[6];
        u_short options;
        u_char  ip[48];
        u_int   data[16];
    }  *ethinfo[2];

    struct  TCP_USB_INFO
    {
        TCP_HEAD head;
        u_char   hasUSB;
        u_char   pad[3];
    }  *usbinfo;

    struct  TCP_TSERVER_INFO
    {
        TCP_HEAD head;
        u_char   asterm[16];
        u_char   ipterm[254];
        u_short  devnum;
    }  *tsinfo;

    struct  TCP_PRI_INFO
    {
        TCP_HEAD head;
        u_char   status;
        u_char   pad;
        u_short jobs;
        char   name[32];
    }  *prinfo;

    struct  TCP_TERM_INFO
    {
        TCP_HEAD head;
        u_char  port;
        u_char  port_conf;
        u_char  cur;
        u_char  scrnum;
        u_char  ip[4];
        u_char  type[12];
        u_char  status[8];

        struct  TCP_SCR_INFO
        {
            u_char  num;
            u_char  pad;
            u_short port;
            u_char  ip[4];
            u_char  protocol[12];
            u_char  status[8];
            u_char  tip[24];
            u_char  type[12];
            u_int   time;
            u_int   bytes[4];
            u_int   ping[3];
        } *scrinfo[16];
    } *terminfo[50];

    struct  TCP_USBFILE_INFO
    {
        TCP_HEAD head;
        char    pro[4095 + 1];
    } *fileinfo;

    struct  TCP_LIST_INFO
    {
        TCP_HEAD head;
        char   pad;
    } *listinfo;
};

class SERVER
{
public:
    SERVER();
    ~SERVER();
    static bool debug_print;
    static u_char KEY[];
    int     tcp_recv(int s, char *buf, int len);
    int     tcp_send(int s, char *buf, int len);
    void    tcp_print(const char *tcp, int size);

    void    init();
    void    parse_conf();
    void    start();

    void    send_authstr(CLIENT *client);
    int     read_from_client(CLIENT *client);
    bool    recv_auth(CLIENT *clienti);
    void    recv_sysinfo(char *info, CLIENT *client);
    void    recv_confinfo(char *info, int datalen, CLIENT *client);
    void    recv_pidinfo(char *info, int datalen, CLIENT *client);
    void    recv_ethinfo(char *info, CLIENT *client, short ethnum);
    void    recv_usbinfo(char *info, CLIENT *client);
    void    recv_tsinfo(char *info, CLIENT *client);
    void    recv_priinfo(char *info, CLIENT *client);
    void    recv_terminfo(char *info, CLIENT *client);
    void    recv_usbfile_info(char *info, int datalen, CLIENT *client);
    void    recv_prifile_info(char *info, int datalen, CLIENT *client);

    int     mysql_write(CLIENT *client);
    std::string getTime();
    std::string myendl();
    std::string to_ipaddress(u_char ip[]);
private:
    struct _DATABASE
    {
        int     port;
        std::string  ip;
        std::string    dbname;
        std::string    username;
        std::string    userpwd;
    } DATABASE;

    struct _CONNECT
    {
        int PORT;
        u_short CONNECT_TIME_GAP;
        u_short SAMPLE_TIME_GAP;
    } CONNECT;

    struct _SYSTEM
    {
        int replyTimeout;
        int tramsTimeout;
        int mainLogSize;
        int subLogSize;
        int BACKLOG;
        int MAXLEN;
    } SYSTEM;

    struct _DEBUG
    {
        int tmp_packet;
        int tmp_socket;
        int dev_packet;
        int dev_socket;
    } DEBUG;

    struct _PATH
    {
        std::string LOGFILE;
        std::string CONFILE;
        u_char secret[4096];
    }PATH;

    int     sockfd;
    teestream *logout;
    std::ofstream  *log;
};

#endif // SERVER_H
