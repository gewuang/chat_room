#ifndef _PUB_H_
#define _PUB_H_

#include <list>
using namespace std;

//C2S
#define C2S_LOGIN               0X01
#define C2S_LOGOUT              0X02
#define C2S_ONLINE_LIST         0X03


//S2S
#define S2C_LOGIN_OK            0X01
#define S2C_ALREADY_LOGOUT      0X02
#define S2C_SOMEONE_LOGIN       0X03
#define S2C_SOMEONE_LOGOUT      0X04
#define S2C_ONLINE_LIST         0X05

//C2C
#define C2C_CHAT                0X06

#define MSG_LEN                 512 
#define NAME_LEN                16 

#define ERR_EXIT(m) \
        do  \
        {   \
                perror(m);  \
                exit(EXIT_FAILURE); \
        }while(0)


typedef struct message
{
    int cmd;
    char body[MSG_LEN];
}MESSAGE;

typedef struct user_info 
{
    char username[NAME_LEN];
    unsigned int ip;
    unsigned short port;
}USER_INFO;

typedef struct chat_msg 
{
    char username[NAME_LEN];
    char msg[MSG_LEN];
}CHAT_MSG;

typedef list<USER_INFO> USER_LIST; 

#endif

