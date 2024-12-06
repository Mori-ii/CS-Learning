#include "log_in.h"
#include "ui_log_in.h"

log_in::log_in(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::log_in)
    , isDragging(false)
    , tcpSocket(new QTcpSocket(this))
{
    ui->setupUi(this);
    ui->label_5->installEventFilter(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    connect(ui->LoginButton,&QPushButton::clicked,this,&log_in::HandleLoginButton_clicked);
    connect(tcpSocket, &QTcpSocket::connected, this, &log_in::onConnected);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &log_in::onError);
}

log_in::~log_in()
{
    delete ui;
}

void log_in:: mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        isDragging = true;
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}
void log_in:: mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging)
    {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}
void log_in:: mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        isDragging = false;
        event->accept();
    }
}

void log_in::HandleLoginButton_clicked()
{
    QString username = ui->user_name->text();
    QString ip = ui->server_ip->text();
    quint16 port = ui->server_port->text().toUShort();

    if (username.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Username cannot be empty!");
        return;
    }
    if(ip.isEmpty() || port == 0)
    {
        QMessageBox::warning(this, "Warning", "Please enter a valid IP and port!");
        return;
    }

    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        tcpSocket->disconnectFromHost();
    }
       tcpSocket->connectToHost(ip,port);
}

void log_in::onConnected()
{
    accept();
}

void log_in::onError(QAbstractSocket::SocketError error)
{
    (void)error;
    QMessageBox::critical(this, "Error", "Could not connect to server: " + tcpSocket->errorString());
}

QString log_in::getname()
{
    return ui->user_name->text();
}

QTcpSocket* log_in::getsocket()
{
    return tcpSocket;
}

bool log_in::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->label_5)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mousePressEvent(mouseEvent);
            return true;
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mouseMoveEvent(mouseEvent);
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            mouseReleaseEvent(mouseEvent);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
