#include "client_widget.h"
#include "log_in.h"
#include <QApplication>

/*
聊天室功能模块:(分析功能实现优先级,先搭好框架,再分块实现,逐步完善)
1. TCP连接
2. 收发string消息
3. 维护在线用户列表
4. 收发文件

 * 客户端实现思路:
 * 1. 明确客户端基本功能逻辑
 * 2. 设计登录对话框UI
 * 3. 实现登录功能:发送用户连接请求和用户名称
 * 4. 实现main函数逻辑
 * 5. 设计客户端主窗口UI
 * 6. 先完成基本框架和最基本的功能,再后续进行扩展
 * 7. 完成主窗口初始化
 * 8. 实现主窗口发送接收消息功能
 * 9. 实现主窗口用户列表更新功能
 * 10.实现主窗口发送接收文件功能
*/
int main(int argc, char *argv[])
{
    QApplication ChatRoom(argc, argv);
    log_in loginDialog;
    if (loginDialog.exec() == QDialog::Accepted)
    {
        client_widget client(nullptr,loginDialog.getsocket());
        client.initialize(loginDialog.getname());
        client.show();
        return ChatRoom.exec();
    }
    return ChatRoom.exec();
}
