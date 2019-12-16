#include "cmd_server.h"
#include "header.h"
#include "msg.h"

int main(int argc, char const *argv[])
{
    char ip[20] = DEFAULT_IP;
    int port = DEFAULT_PORT;
    char user[20] = DEFAULT_USER;
    if (argc == 2) // 只有一个参数时
    {
        port = atoi(argv[1]); // 将参数作为端口号
    }
    else if (argc == 3) // 有两个（或更多）参数时
    {
        strcpy(ip, argv[1]);  // 将第一个参数作为IP地址
        port = atoi(argv[2]); // 将第二个参数作为端口号
    }
    else if (argc == 4)
    {
        strcpy(ip, argv[1]);  // 将第一个参数作为IP地址
        port = atoi(argv[2]); // 将第二个参数作为端口号
        strcpy(user,argv[3]);
    }

    int server_sockfd, client_sockfd, server_len, client_len, result;
    struct sockaddr_in6 server_address, client_address;

    // 创建套接字
    server_sockfd = socket(AF_INET6, SOCK_STREAM , 0);
    /*  AF_INET:        ARPA因特网协议（UNIX网络套接字）
        SOCK_STREAM:    流套接字
        SOCK_NONBLOCK:  非阻塞
        0:              使用默认协议	                */
    if (server_sockfd == -1)
    {
        server_log("socket函数出错，服务器创建套接字失败，请检查端口号是否被占用");
        return -1;
    }

    // 套接字地址
    server_address.sin6_family = AF_INET6;            // 域
    server_address.sin6_addr= in6addr_any; // IP地址
    server_address.sin6_port = htons(port);          // 端口号

    // 计算长度
    server_len = sizeof(server_address);

    // 命名套接字
    result = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    if (result == -1)
    {
        server_log("bind函数出错，服务器命名套接字失败");
        return -1;
    }
    while(1){
        // 创建套接字队列
        result = listen(server_sockfd, 5);
        if (result == -1)
        {
            server_log("listen函数出错，服务器创建套接字队列失败");
            return -1;
        }

        // 服务器启动
        server_log("服务器已启动，正在等待连接");
        printf("服务器已启动，正在等待连接，IP地址：%s，端口号：%d\n", ip, port);
        client_len = sizeof(client_address);

        // 重新打开终端，设置非阻塞
        int tfd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
        if (tfd == -1)
        {
            server_log("open函数出错（/dev/tty）");
            return -1;
        }
        else
        {
            // printf("打开/dev/tty成功\n");
        }

        int connected = 0; // 1表示已连接，0表示未连接
        struct ftpmsg msg;
        char line[MAX_LENGTH];
        char *cmd[MAX_ARGC];
        int pid;
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
        if((pid = fork())<0){
            server_log("fork failed");

        }else if(pid == 0 ){
            connected = 1 ;
            close(server_sockfd);
            while (1)
            {
                // 若没有连接，则尝试接收连接
                if (!connected)
                {
                    // 接受连接
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
                    if (client_sockfd == -1)
                    {
                        if (errno == EWOULDBLOCK)
                        {
                            // 没有收到连接请求，重新尝试接受连接
                            // printf("没有收到连接请求\n");
                        }
                        else
                        {
                            server_log("accept函数出错，服务器接受连接失败");
                            return -1;
                        }
                    }
                    else
                    {
                        server_log("连接成功，等待用户登录");
                        connected = 1;
                    }
                }

                // 若已连接，则接收指令
                if (connected)
                {
                    // printf("服务器等待中……");
                    recv_msg(client_sockfd, &msg);
                    // printf("recv_msg，返回值为%d\n", recv_msg(client_sockfd, &msg));
                    switch (msg.type)
                    {
                        case LOGIN:
                            if(strcmp(user,"anonym")==0)
                                msg.data = user;
                            c_login(client_sockfd, msg.data);
                            break;
                        case C_MKDIR:
                            c_mkdir(client_sockfd, msg.data);
                            break;
                        case C_RMDIR:
                            c_rmdir(client_sockfd, msg.data);
                            break;
                        case C_PWD:
                            c_pwd(client_sockfd);
                            break;
                        case C_CD:
                            c_cd(client_sockfd, msg.data);
                            break;
                        case C_LS:
                            c_ls(client_sockfd, msg.data);
                            break;
                        case C_PUT:
                            c_put(client_sockfd);
                            break;
                        case C_GET:
                            c_get(client_sockfd, msg.data);
                            break;
                        case C_QUIT:
                            server_log("客户端连接已断开，正在等待重新连接");
                            printf("客户端连接已断开，正在等待重新连接，IP地址：%s，端口号：%d\n", ip, port);
                            msg.type = DEFAULT;
                            connected = 0;
                            break;
                        default:
                            break;
                    }
                }

                result = read(tfd, line, MAX_LENGTH);
                if (result == -1)
                {
                    if (errno == EWOULDBLOCK)
                    {
                    }
                    else
                    {

                        server_log("read函数出错，读取指令失败");
                    }
                }
                else
                {
                    line[result - 1] = '\0';
                    // printf("result = %d, line = %s\n", result, line);
                }
            }
            close(client_sockfd);
            return -1;
        }
        else{
            close(client_sockfd);
        }

    }
}
