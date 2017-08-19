#include <iostream>
#include "client.h"
#include <sys/wait.h>
#include <errno.h>
#include <sys/stat.h>

using namespace std;

TsConf  TS_CONF ;

int main(int argc, char *argv[])
{
    int     amount = 1;
    int     devid  = 100101010 ;
    int     i ;
    if (argc < 3)
    {
        cout << "input the right forum like:<./ts 100101010 10>"<<endl;
        cout << "the default value is : ./ts 100101010 1" << endl;
    }
    else
    {
        amount = atoi(argv[2]);
        devid  = atoi(argv[1]);
    }

    pid_t   pid;

    TS_CONF.init();
    for (i=0; i<amount; i++)
    {
        if ( (pid = fork()) <= 0)
        {
            break;
        }
        if ( (i+1)%500 == 0)
        {
            while (wait(NULL) != -1);
        }
    }

    if (pid == 0)
    {
        CLIENT client(devid + i);
        client.start();
        exit(0);
    }
    else if (pid < 0)
    {
        cout << "fatal error: cannot fork " << endl;
        return -1;
    }

    while (wait(NULL) != -1);
    return 0;
}
