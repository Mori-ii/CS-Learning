#include <sys/socket.h>  // 引入socket相关头文件
#include <netinet/in.h>  // 引入网络地址数据结构
#include <arpa/inet.h>   // 引入网络地址转换函数
#include <sys/epoll.h>   // 引入epoll相关头文件
#include <stdexcept>     // 引入标准异常处理
#include <unistd.h>      // 引入Unix标准头文件
#include <string.h>      // 引入字符串处理函数
#include <cstring>       // 引入C标准字符串处理
#include <iostream>      // 引入输入输出流
#include <fcntl.h>       // 引入文件控制定义
#include <unordered_map> // 引入无序映射，存储客户端信息
#include <atomic>        // 引入原子操作，确保线程安全
#include <csignal>       // 引入信号处理
#include <sstream>       // 引入字符串流

/*
开发经验积累:
1. 实现思路: 首先分析功能实现优先级,搭好最基础功能框架,再分块实现,逐步完善
2. 模块实现: 在实现具体模块时先分析清楚要拆分成哪几个逻辑来实现,再分步实现
3. 封装: 要注意对各个功能模块进行合理封装(函数/类)提高可读性以便维护
4. 测试 : 分模块进行,每完成一个模块就要进行测试
5. debug : 测试时出现bug,根据程序的具体失控现象来判断该模块哪里出现了逻辑错误,并逐步进行利用插入输入输出等手段进行排查
6. 注释: 在每个部分之前简略介绍主体功能,在重要的API/易致错/特殊结构处进行标记
7. 错误处理: 利用API返回值/异常处理机制进行检查,如果错误则打印错误信息,并关闭程序/跳过当前操作
8. 变量命名: 尽可能采用小写,且能够准确表达出该变量含义的名称,以保证程序可读性(宏定义采用大写)
9. 谨慎修改:修改正常可运行的部分时一定要谨慎谨慎再谨慎,
           尤其是像C/S架构这种不同程序需要相互对接的情况下,修改时一定要考虑周全,比如MAX_NAMEBUFFER如果单方面修改就会导致难以察觉的错误

聊天室功能模块
1. TCP连接
2. 收发string消息
3. 维护在线用户列表
4. 收发文件

服务器基本框架:
1. Server_socket初始化
2. Server_socket绑定IP和端口
3. Server_socket监听模式
4. epoll初始化
5. 将Server_socket注册到epoll实例中
6. 开启事件循环
7. 如果Server_socket触发事件,说明有新的客户端请求连接
8. 接受连接请求并将新的Client_socket注册到epoll实例和客户列表中
9. 如果Client_socket触发事件,说明客户端发来了某种消息
10.解析消息类型并进行不同的处理
11.捕捉到终止程序信号时结束事件循环
12.释放服务器占用的资源并关闭服务器
*/

#define MAX_EVENTS 64
#define MAX_NAMEBUFFER 64

// 定义服务器存储客户端结构
struct Client
{
    std::string address;
    std::string Name = "Unknown";
    bool isRecving = false;
};

// 定义消息类型枚举--枚举类型顺序居然会导致程序问题??????
// 枚举类型变量在编译器眼中是int类型,当你使用一个枚举类型常量时,必须意识到你其实只是在使用一个int常量
// 这就要求服务器和客户端的枚举类型顺序必须一致,或者在二者程序中手动将相同的枚举类型也赋值为相同的int
enum MSG_type
{
    INITIAL,
    JOIN,
    EXIT,
    GROUP_MSG,
    FILE_MSG,
    FAILED,
};

// 定义消息头结构
struct MSG_header
{
    char sender_name[MAX_NAMEBUFFER];
    // 发送者名称不可以使用string类型,string类型长度不确定,使用收发函数时如果已知传输信息大小则可以避免很多麻烦
    MSG_type Type;
    size_t length;
};

// 定义无序映射，存储所有已连接的客户端信息
std::unordered_map<int, Client> clients;
std::atomic<bool> running(true);

int createServer();                                                             // 创建服务器
void setNonBlocking(int sock_fd);                                               // 设置连接非阻塞
int getPort();                                                                  // 获取端口
void CliConnection_handle(int Server_fd, int epoll_fd);                         // 处理客户端连接请求
MSG_header getHeader(int client_fd);                                            // 获取消息头
void JOIN_handle(int client_fd, MSG_header &header);                            // 处理JOIN信息
void GROUP_MSG_handle(int client_fd, MSG_header &header);                       // 处理群发消息
void broadcastMsg(int sender_fd, const char *buffer, const MSG_header &header); // 将buffer和header拼接广播
void FILE_MSG_handle(int client_fd, MSG_header &header);                        // 处理文件消息
void signalHandler(int sign);                                                   // 终止信号处理
void client_initial(int client_fd);                                             // 同步初始化在线用户列表,拼接在线用户名称并发送
void client_exit(int client_fd);                                                // 处理客户端退出逻辑

int main()
{
    // 捕捉终止程序的信号
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 创建服务器socket
    int Server_fd = createServer();

    // 创建epoll实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("Epoll create failed");
        close(Server_fd);
        return EXIT_FAILURE;
    }

    // 将服务器socket注册到epoll实例中
    struct epoll_event event;
    event.events = EPOLLIN; // 设置事件为输入可读,不设置水平/边沿触发,默认水平触发,客户端连接不会很频繁,且比较重要,使用水平触发
    event.data.fd = Server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, Server_fd, &event) == -1)
    {
        perror("Epoll ctl failed");
        close(Server_fd);
        close(epoll_fd);
        return EXIT_FAILURE;
    }

    std::cout << "Server started." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    // 事件循环启动
    while (running)
    {
        // 开始等待触发
        epoll_event events[MAX_EVENTS];
        int nfd = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfd < 0)
        {
            perror("Epoll_wait failed");
            close(Server_fd);
            close(epoll_fd);
            break;
        }

        // 处理每一个触发事件
        for (int i = 0; i < nfd; i++)
        {
            // 服务器socket被触发,说明有客户端连接上来,进行处理
            if (events[i].data.fd == Server_fd)
            {
                CliConnection_handle(Server_fd, epoll_fd);
            }
            else // 客户端socket被触发,有消息传来(当客户端退出时也会发来一个信号,getHeader中处理退出)
            {
                int Client_fd = events[i].data.fd;
                if (clients[Client_fd].isRecving == true)
                    continue; // 如果当前客户端正在处于接收文件中,那么此时接收到的是客户端发送文件碎片请求,直接忽略
                MSG_header header = getHeader(Client_fd);
                if (header.Type == FAILED)
                    continue;
                switch (header.Type)
                {
                case JOIN: // 客户端连接后立即自动发送,设置在服务器中的名称,同时对其进行初始化,并告知其他客户端
                    JOIN_handle(Client_fd, header);
                    break;

                case GROUP_MSG: // 群发消息,将消息头和群发消息再拼接回去进行转发
                    GROUP_MSG_handle(Client_fd, header);
                    break;

                case FILE_MSG:
                    FILE_MSG_handle(Client_fd, header);
                    break;
                }
            }
        }
    }
    std::cout << "Server is shutting down..." << std::endl;
    // 服务器关闭,清理向OS申请的资源和堆上创建的数据
    for (const auto &CLI : clients)
        close(CLI.first);
    close(Server_fd);
    close(epoll_fd);
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    return EXIT_SUCCESS;
}

void setNonBlocking(int sock_fd)
{
    // 直接设置文件描述符属性可能会意外地覆盖原有的设置，而使用 F_GETFL 获取当前状态可以确保只修改需要更改的部分
    int flags = fcntl(sock_fd, F_GETFL, 0);
    if (flags == EXIT_FAILURE)
    {
        perror("Fcntl failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    if (fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) == EXIT_FAILURE)
    {
        perror("Fcntl failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
}

int getPort()
{
    int PORT = 0;
    std::cout << "Please Enter the Server Binding PORT" << std::endl;
    while (true)
    {
        std::string input;
        std::cin >> input;

        try
        {
            PORT = std::stoi(input);
            if (PORT >= 1024 && PORT <= 65535)
                break;
            else
                std::cout << "PORT:" << PORT << " " << "is not valid."
                                                       "Please enter a port mumber between 1024 and 65535."
                          << std::endl;
        }
        catch (std::invalid_argument const &e)
        {
            std::cout << "Invalid input. Please enter a valid number." << std::endl;
        }
        catch (std::out_of_range const &e)
        {
            std::cout << "Input is out of range. Please enter a number within the range of an int." << std::endl;
        }
    }
    return PORT;
}

void CliConnection_handle(int Server_fd, int epoll_fd)
{
    struct sockaddr_in Client_addr;
    socklen_t Client_len = sizeof(Client_addr);
    int Client_fd = accept(Server_fd, (struct sockaddr *)&Client_addr, &Client_len);
    if (Client_fd < 0)
    {
        perror("Accept failed");
        kill(getpid(), SIGINT);
    }
    setNonBlocking(Client_fd);
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // 设置为边沿触发,提高性能
    event.data.fd = Client_fd;
    // 客户端socket注册到epoll实例
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, Client_fd, &event) < 0)
    {
        perror("Epoll_ctl client failed");
        kill(getpid(), SIGINT);
    }
    // 客户端socket注册到服务器客户列表
    clients[Client_fd] = {inet_ntoa(Client_addr.sin_addr), "UNKOWN", false};
    std::cout << "New connection from " << clients[Client_fd].address << std::endl;
}

MSG_header getHeader(int client_fd)
{
    // 先解析消息头,判断消息类型
    MSG_header header;
    header.Type = FAILED;
    // POSIX标准 recv与send参数:指针与要操作的数据大小
    ssize_t ByteRecv = recv(client_fd, &header, sizeof(MSG_header), 0);
    if (ByteRecv < 0)
    {
        perror("Recv header failed");
        std::cout << "Disconnect from " << clients[client_fd].address << std::endl;
        client_exit(client_fd);
    }
    // 客户端退出会发送一个信号,recv会将其捕捉并返回0,相当于在此处理了EXIT类型消息
    else if (ByteRecv == 0)
    {
        std::cout << "Disconnect from " << clients[client_fd].address << std::endl;
        client_exit(client_fd);
    }
    return header;
}

void JOIN_handle(int client_fd, MSG_header &header)
{
    clients[client_fd].Name = header.sender_name;
    client_initial(client_fd);
    for (const auto &client : clients)
    {
        if (client.first != client_fd)
        {
            if (!send(client.first, &header, sizeof(header), 0))
            {
                perror("JOIN send failed");
                client_exit(client_fd);
            }
        }
    }
}

void GROUP_MSG_handle(int client_fd, MSG_header &header)
{
    char *buffer = new char[header.length];
    if (!recv(client_fd, buffer, header.length, 0))
    {
        perror("GROUPmsg recv failed");
        client_exit(client_fd);
    }
    std::cout << "Received group message from " << clients[client_fd].address << std::endl;
    broadcastMsg(client_fd, buffer, header);
}

void broadcastMsg(int sender_fd, const char *buffer, const MSG_header &header)
{
    size_t total_size = sizeof(header) + header.length;
    char *send_buffer = new char[total_size];
    std::memcpy(send_buffer, &header, sizeof(header));
    std::memcpy(send_buffer + sizeof(header), buffer, header.length);
    ssize_t bytes_sent = 0;
    for (const auto &client : clients)
    {
        if (client.first != sender_fd)
        {
            bytes_sent = send(client.first, send_buffer, total_size, 0);
            if (bytes_sent < 0)
            {
                delete[] buffer;
                delete[] send_buffer;
                perror("Broadcast send failed");
                kill(getpid(), SIGINT);
                return;
            }
            else if (bytes_sent < header.length)
            {
                delete[] buffer;
                delete[] send_buffer;
                perror("Broadcast send failed partly");
                kill(getpid(), SIGINT);
                return;
            }
        }
    }
    std::cout << clients[sender_fd].address << " sents " << bytes_sent << " bytes to all clients." << std::endl;
    delete[] buffer;
    delete[] send_buffer;
}

void FILE_MSG_handle(int client_fd, MSG_header &header)
{
    clients[client_fd].isRecving = true;
    size_t totalBytes = header.length;
    size_t bytesRead = 0;
    const int CHUNK_SIZE = 4096;
    char buffer[CHUNK_SIZE];
    for (auto &client : clients)
    {
        if (client.first != client_fd)
        {
            ssize_t sentBytes = send(client.first, &header, sizeof(header), 0);
        }
    }
    while (bytesRead < totalBytes)
    {
        ssize_t result = recv(client_fd, buffer, CHUNK_SIZE, 0);
        if (result < 0) continue;


        bytesRead += result;

        //转发数据给其他客户端
        for (auto &client : clients)
        {
            if (client.first != client_fd)
            {
                ssize_t sentBytes = send(client.first, buffer, result, 0);
            }
        }
        std::cout << "已转发 " << bytesRead << " 字节" << std::endl;
    }
    std::cout << "文件转发完成，共转发 " << bytesRead << " 字节" << std::endl;
    clients[client_fd].isRecving = false;
}

void signalHandler(int sign)
{
    std::cout << std::endl
              << "Interrupt signal (" << sign << ") received." << std::endl;
    running = false;
}

void client_initial(int client_fd)
{
    // 使用ostringstream字符串流进行拼接
    MSG_header header;
    header.Type = INITIAL;
    std::strcpy(header.sender_name, "SERVER");

    std::ostringstream oss;

    for (const auto &user : clients)
    {
        if (user.first != client_fd)
        {
            oss << user.second.Name << ",";
        }
    }

    std::string userList = oss.str();

    if (userList.empty())
        return;
    else
        userList.pop_back();

    header.length = userList.length();
    char *messageBuffer = new char[header.length + sizeof(header)];

    std::memcpy(messageBuffer, &header, sizeof(header));
    std::memcpy(messageBuffer + sizeof(header), userList.c_str(), userList.length());
    // 直接对内存操作时,应将string类型转换成c_str,即字符数组的字符串形式

    ssize_t bytesSent = send(client_fd, messageBuffer, header.length + sizeof(header), 0);
    if (bytesSent < 0)
    {
        delete[] messageBuffer;
        perror("Initial send failed");
        kill(getpid(), SIGINT);
        return;
    }
    delete[] messageBuffer;
}

void client_exit(int client_fd)
{
    MSG_header header;
    header.Type = EXIT;
    strcpy(header.sender_name, clients[client_fd].Name.c_str());
    header.length = 0;
    for (const auto &client : clients)
    {
        if (client.first != client_fd)
            if (send(client.first, &header, sizeof(header), 0) < 0)
            {
                perror("EXIT send failed");
                kill(getpid(), SIGINT);
                return;
            }
    }
    clients.erase(client_fd);
    close(client_fd);
}

int createServer()
{
    // 创建服务器socket
    int Server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (Server_fd < 0)
    {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // 设置socket选项以便端口复用
    int opt = 1;
    if (setsockopt(Server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("Setsockopt failed");
        close(Server_fd);
        return EXIT_FAILURE;
    }

    // 设置服务器socket为非阻塞模式
    setNonBlocking(Server_fd);

    // 获取端口
    int PORT = getPort();
    // 绑定服务器socket到指定的IP和端口
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT); // host to net short
    if (bind(Server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(Server_fd);
        return EXIT_FAILURE;
    }

    // 设置服务器socket为监听模式
    if (listen(Server_fd, SOMAXCONN) < 0)
    {
        perror("Listen failed");
        close(Server_fd);
        return EXIT_FAILURE;
    }
    return Server_fd;
}