#include <stdio.h>
#include "pub.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>  
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>


//chat client list
USER_LIST client_list;

void chat_srv(int sock);
void do_login(MESSAGE& msg, int sock, struct sockaddr_in* cliaddr);
void do_logout(MESSAGE& msg, int sock, struct sockaddr_in* cliaddr);
void do_sendlist(int sock, struct sockaddr_in* cliaddr);

void do_sendlist(int sock, struct sockaddr_in* cliaddr)
{
    MESSAGE msg;
    USER_LIST::iterator it;

    msg.cmd = htonl(S2C_ONLINE_LIST);
    sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   

    int count = htonl((int)client_list.size());
    sendto(sock, &count, sizeof(count), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   

    //send online list
    for(it = client_list.begin(); it != client_list.end(); ++it)
    {
        sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   
    }

}

void do_logout(MESSAGE& msg, int sock, struct sockaddr_in* cliaddr)
{
    USER_LIST::iterator it;
    printf("has a user logout: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));

    for(it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(strcmp(it->username, msg.body) == 0)
        {
            break;
        }
    }
    if(it != client_list.end())
    {
        client_list.erase(it); 
    }

    for(it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(strcmp(it->username, msg.body) == 0)
        {
            continue;
        }

        struct sockaddr_in peeraddr;
        memset(&peeraddr, 0, sizeof(peeraddr));
        peeraddr.sin_family = AF_INET;
        peeraddr.sin_port = it->port;
        peeraddr.sin_addr.s_addr = it->ip;

        msg.cmd = htonl(S2C_SOMEONE_LOGOUT);

        if(sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) == -1) 
        {
            ERR_EXIT("sendto");
        }
    }

}

void do_login(MESSAGE& msg, int sock, struct sockaddr_in* cliaddr)
{
    USER_INFO user;
    strcpy(user.username, msg.body);
    user.ip = cliaddr->sin_addr.s_addr;
    user.port = cliaddr->sin_port;

    //search user
    USER_LIST::iterator it;

    for(it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(strcmp(it->username, msg.body) == 0)
        {
            break;
        }
    }

    //list without user
    if(it == client_list.end())
    {
        printf("has a user login: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));
        client_list.push_back(user);
        
        //login ok
        MESSAGE reply_msg;

        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_LOGIN_OK);
        sendto(sock, &reply_msg, sizeof(MESSAGE), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   

        //send online user count
        int count = htonl((int)client_list.size());
        sendto(sock, &count, sizeof(count), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   


        printf("sending user list information to: %s <-> %s:%d\n", msg.body, inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port));

        //send online list
        for(it = client_list.begin(); it != client_list.end(); ++it)
        {
            sendto(sock, &*it, sizeof(USER_INFO), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   
        }

        //send other users that some new user login
        for(it = client_list.begin(); it != client_list.end(); ++it)
        {
            if(strcmp(it->username, msg.body) == 0)
            {
                continue;
            }

            struct sockaddr_in peeraddr;
            memset(&peeraddr, 0, sizeof(peeraddr));
            peeraddr.sin_family = AF_INET;
            peeraddr.sin_port = it->port;
            peeraddr.sin_addr.s_addr = it->ip;

            msg.cmd = htonl(S2C_SOMEONE_LOGIN);
            memcpy(msg.body, &user, sizeof(USER_INFO));

            if(sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr)) == -1) 
            {
                ERR_EXIT("sendto");
            }
        }
    }
    else        //find user
    {
        printf("user %s has already logined\n", msg.body);

        MESSAGE reply_msg;
        memset(&reply_msg, 0, sizeof(reply_msg));
        reply_msg.cmd = htonl(S2C_ALREADY_LOGOUT);
        sendto(sock, &reply_msg, sizeof(msg), 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr));   
    }
}

void chat_srv(int sock)
{
    struct sockaddr_in cliaddr;
    socklen_t clilen;
    int n;
    MESSAGE msg;

    while(1)
    {
        memset(&msg, 0, sizeof(msg));
        clilen = sizeof(cliaddr);
        n = recvfrom(sock, &msg, sizeof(msg), 0, (struct sockaddr *)&cliaddr, &clilen);

        if(n < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            ERR_EXIT("recvfrom");
        }

        int cmd = ntohl(msg.cmd);

        switch(cmd)
        {
            case C2S_LOGIN:
            {
                do_login(msg, sock, &cliaddr);
            }
            break;
            case C2S_LOGOUT:
            {
                do_logout(msg, sock, &cliaddr);
            }
            break;
            case C2S_ONLINE_LIST:
            {
                do_sendlist(sock, &cliaddr);
            }
            break;
            default:
            break;
        }
    }

}

int main(void)
{
    int sock = 0;

    if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        ERR_EXIT("bind");
    }

    chat_srv(sock);


    return 0;
}

