#include "cmd_client.h"
#include "header.h"
#include "msg.h"

int main(int argc, char const *argv[])
{
    char ip[20] = DEFAULT_IP;
    int port = DEFAULT_PORT;
    char username[20] = "noname";
    int overtime=3;  //超时时间
    // 获取时间
    time_t rawtime;
    struct tm *ptminfo;

    time(&rawtime);
    ptminfo = localtime(&rawtime);



    if (argc == 2) // 只有一个参数时
    {
        port = atoi(argv[1]); // 将参数作为端口号
    }
    else if (argc == 3) // 有两个参数时
    {
        strcpy(ip, argv[1]);  // 将第一个参数作为IP地址
        port = atoi(argv[2]); // 将第二个参数作为端口号
    }
    else if (argc == 4) // 有三个参数时
    {
        strcpy(ip, argv[1]);  // 将第一个参数作为IP地址
        port = atoi(argv[2]); // 将第二个参数作为端口
        strcpy(username, argv[3]); // 将第三个参数作为用户名（可能为匿名用户）
    }
    else if (argc > 4) // 有四个及以上参数时
    {
        strcpy(ip, argv[1]);  // 将第一个参数作为IP地址
        port = atoi(argv[2]); // 将第二个参数作为端口
        strcpy(username, argv[3]); // 将第三个参数作为用户名（可能为匿名用户）
        overtime=argv[4];  //第四个参数为超时时间
    }

    int sockfd, len, result;
    struct sockaddr_in6 address;
    struct timeval timeout = {overtime,0};

    // 创建套接字
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    /*  AF_INET6:        ARPA因特网协议（UNIX网络套接字）
        SOCK_STREAM:    流套接字
        0:              使用默认协议	                */
    if (sockfd == -1)
    {
        client_log("socket函数出错，客户端创建套接字失败");
        return -1;
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval))==-1)  //设置发送超时
    {
        client_log("发送超时设置失败");
    }

    if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval))==-1)  //设置接收超时
    {
        client_log("接收超时设置失败");
    }
    
    // 套接字地址
    address.sin6_family = AF_INET6;            // 域
    address.sin6_addr = in6addr_any; // IP地址
    address.sin6_port = htons(port);          // 端口号

    // 计算长度
    len = sizeof(address);

    // 请求连接
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if (result == -1)
    {
        client_log("connect函数出错，客户端请求连接失败，请检查服务器是否正在运行");
        return -1;
    }

    struct ftpmsg msg;

    // 开启命令行

    client_log("连接服务器成功");
    while (client_login(sockfd,username) != 1)
    {
    }

    // 登录成功后的循环，命令执行完则continue，quit或exit命令则break
    char line[MAX_LENGTH];
    char cmd[MAX_LENGTH];
    while (1)
    {
        printf("myftp > ");
        gets(line);
        gettoken(cmd, line);
        if (cmd == NULL) // 指令为空
        {
            continue;
        }
        else if (!strcmp(cmd, "lmkdir"))
        {
            client_lmkdir(line);
        }
        else if (!strcmp(cmd, "lrmdir"))
        {
            client_lrmdir(line);
        }
        else if (!strcmp(cmd, "lpwd"))
        {
            client_lpwd(line);
        }
        else if (!strcmp(cmd, "lcd"))
        {
            client_lcd(line);
        }
        else if (!strcmp(cmd, "dir"))
        {
            client_dir(line);
        }
        else if (!strcmp(cmd, "mkdir"))
        {
            client_mkdir(sockfd, line);
        }
        else if (!strcmp(cmd, "rmdir"))
        {
            client_rmdir(sockfd, line);
        }
        else if (!strcmp(cmd, "pwd"))
        {
            client_pwd(sockfd);
        }
        else if (!strcmp(cmd, "cd"))
        {
            client_cd(sockfd, line);
        }
        else if (!strcmp(cmd, "ls"))
        {
            client_ls(sockfd, line);
        }
        else if (!strcmp(cmd, "put"))
        {
            client_put(sockfd, line);
        }
        else if (!strcmp(cmd, "get"))
        {
            client_get(sockfd, line);
        }
        else if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit") || !strcmp(cmd, "q"))
        {
            send_simple(sockfd, C_QUIT);
            break;
        }
        else if (!strcmp(cmd, "help"))
        {
            client_help();
        }
        else
        {
            client_log("错误的指令，使用help命令查看指令列表");
        }
    }

    // 关闭套接字
    close(sockfd);

    return 0;
}
