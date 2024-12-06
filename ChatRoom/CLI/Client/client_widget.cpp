#include "client_widget.h"
#include "ui_client_widget.h"

client_widget::client_widget(QWidget *parent,QTcpSocket* Tcpsocket)
    : QWidget(parent)
    , tcpsocket(Tcpsocket)
    , ui(new Ui::client_widget)
{
    ui->setupUi(this);
    ui->user_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->msgsender_btn,&QPushButton::clicked,this,&client_widget::sendMsg);
    connect(ui->filesender_btn,&QPushButton::clicked,this,&client_widget::sendFile);
    connect(tcpsocket, &QTcpSocket::readyRead, this,&client_widget::onReadyRead);
    connect(ui->sender_edit, &QLineEdit::returnPressed, this, &client_widget::sendMsg);
}
void client_widget::onReadyRead() { if (!isRecving) readMsg();}
void client_widget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

client_widget::~client_widget()
{
    delete ui;
    delete tcpsocket;
}

void client_widget::initialize(const QString& username)
{
    MSG_header header;
    header.Type = JOIN;
    header.length = 0;
    std::strncpy(header.sender_name,username.toUtf8().constData(),sizeof(header.sender_name));
    qint64 bytesWritten = tcpsocket->write(reinterpret_cast<const char*>(&header),sizeof(header));
    if (bytesWritten == -1)
    {
        QMessageBox::critical(this, tr("错误"), tr("JOIN发送失败: %1").arg(tcpsocket->errorString()));
        return;
    }

    user_name = username;
    ui->Now_user->setText("当前在线用户:"+username);
    logintime = QDateTime::currentDateTime();
    ui->time->setText("登录时间: " + logintime.toString("yyyy-MM-dd HH:mm:ss"));
    timer = new QTimer(this);
    timer->start(1000);
    connect(timer, &QTimer::timeout, this, &client_widget::updateLoginTime);
}

void client_widget::updateLoginTime()
{
    qint64 seconds = logintime.secsTo(QDateTime::currentDateTime());
    ui->timing->setText("已登录时间: " + QString::number(seconds) + " 秒");
}

void client_widget::readMsg()
{
    while (tcpsocket->bytesAvailable())
    {
        if(isRecving == true) continue;
        MSG_header header;
        qint64 bytesRead = tcpsocket->read(reinterpret_cast<char*>(&header), sizeof(header));

        if (bytesRead == -1)
        {
            qDebug() << "Error reading from socket:" << tcpsocket->errorString();
        }
        switch(header.Type)
        {
        case INITIAL:
        {
            QByteArray userListData = tcpsocket->read(header.length);
            QString userList = QString::fromUtf8(userListData);

            QStringList users = userList.split(',', Qt::SkipEmptyParts);
            ui->user_list->clear();

            for (const QString &user : users)
            {
                ui->user_list->addItem(user.trimmed());
            }
            break;
        }
        case JOIN:
        {
            ui->textBrowser->append(QString("%1: %2 %3").arg("SERVER",header.sender_name,"加入聊天室."));
            ui->user_list->addItem(header.sender_name);
            break;
        }
        case EXIT:
        {
            ui->textBrowser->append(QString("%1: %2 %3").arg("SERVER",header.sender_name,"退出聊天室."));
            for (int i = 0; i < ui->user_list->count(); ++i)
            {
                QListWidgetItem *item = ui->user_list->item(i);
                if (item->text() == header.sender_name)
                {
                    delete ui->user_list->takeItem(i);
                    break;
                }
            }
            break;
        }
        case GROUP_MSG:
        {
            QByteArray msgData = tcpsocket->read(header.length);
            QString msg = QString::fromUtf8(msgData);
            ui->textBrowser->append(QString("%1: %2").arg(header.sender_name, msg));
            break;
        }
        case FILE_MSG:
        {
            File_handle(header);
            break;
        }

        }
    }
}

void client_widget::sendMsg()
{
    QString msg = ui->sender_edit->text();
    if (msg.isEmpty()) return;
    MSG_header header;
    std::strncpy(header.sender_name, user_name.toUtf8().constData(),sizeof(header.sender_name));
    header.Type = GROUP_MSG;
    header.length = msg.toUtf8().size();
    QByteArray data;
    data.append(reinterpret_cast<const char*>(&header), sizeof(header));
    data.append(msg.toUtf8());
    tcpsocket->write(data);
    ui->sender_edit->clear();
    ui->textBrowser->append(user_name + ": " + msg);
}

void client_widget::sendFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("所有文件 (*)"));
    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("错误"), tr("无法打开文件: %1").arg(file.errorString()));
        return;
    }

    MSG_header header;
    std::strncpy(header.sender_name, user_name.toUtf8().constData(), sizeof(header.sender_name));
    header.Type = FILE_MSG;
    header.length = file.size();

    QByteArray headerBuffer(reinterpret_cast<const char*>(&header), sizeof(header));
    tcpsocket->write(headerBuffer); // 告诉服务器开始文件传输模式

    // 初始化并显示进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(true);
    progressBar->show();

    // 启动定时器进行分块发送
    chunkedFileTransfer(file);
}

void client_widget::chunkedFileTransfer(QFile &file)
{
    const int chunkSize = 4096; // 分块大小，4KB
    static qint64 totalBytesSent = 0; // 记录已发送的字节数
    static qint64 totalFileSize = file.size(); // 文件总大小

    while(!file.atEnd())
    {
        QByteArray chunk = file.read(chunkSize);
        qint64 bytesWritten = tcpsocket->write(chunk);
        totalBytesSent += bytesWritten;

        if (bytesWritten == -1)
        {
            QMessageBox::critical(this, tr("错误"), tr("文件发送失败: %1").arg(tcpsocket->errorString()));
            file.close();
            progressBar->deleteLater(); // 删除进度条
            return;
        }

        // 更新进度条
        int progress = static_cast<int>((totalBytesSent * 100) / totalFileSize);
        progressBar->setValue(progress);
    }
        file.close(); // 关闭文件
        QMessageBox::information(this, tr("成功"), tr("文件发送成功!"));
        progressBar->deleteLater(); // 删除进度条
}

void client_widget::File_handle(MSG_header& header)
{

    isRecving = true;
    // 用户确认接收文件
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("文件接收"),
                                  tr("%1 向您发送了一个文件，请问您要接收吗？").arg(header.sender_name),
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
    {
        return;  // 用户选择不接收
    }
    if (reply == QMessageBox::Yes) {
        // 打开文件用于写入
        QString savePath = QFileDialog::getSaveFileName(this, tr("保存文件"), "", tr("所有文件 (*)"));
        if (savePath.isEmpty()) return; // 用户未选择保存路径

        // 打开文件以写入数据
        QFile file(savePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "无法打开文件:" ;
            isRecving = false;
            return;
        }

        // 设置接收缓冲区大小
        const int CHUNK_SIZE = 4096;
        char buffer[CHUNK_SIZE];
        qint64 bytesRead;
        qint64 totalBytesRead = 0;

        // 接收文件数据
        while (totalBytesRead < header.length) {
            bytesRead = tcpsocket->read(buffer, CHUNK_SIZE);
            if (bytesRead <= 0) {
                qWarning() << "接收数据错误或连接中断";
                file.close();
                isRecving = false;
                return;
            }
            totalBytesRead += bytesRead;
            file.write(buffer, bytesRead);
        }

        // 关闭文件
        file.close();

        qDebug() << "文件接收完成，共接收" << totalBytesRead << "字节";
    }

    isRecving = false;
}
