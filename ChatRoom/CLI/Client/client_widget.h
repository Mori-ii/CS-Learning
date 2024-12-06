#ifndef CLIENT_WIDGET_H
#define CLIENT_WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QStyleOption>
#include <QDateTime>
#include <QTimer>
#include <QTcpSocket>
#include <QListView>
#include <QStringListModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>

#define MAX_NAME 64

QT_BEGIN_NAMESPACE
namespace Ui
{
class client_widget;
}
QT_END_NAMESPACE

enum MSG_type
{
    INITIAL,
    JOIN,
    EXIT,
    GROUP_MSG,
    FILE_MSG,
    FAILED,
};
struct MSG_header
{
    char sender_name[MAX_NAME];
    MSG_type Type;
    size_t length;
};



class client_widget : public QWidget
{
    Q_OBJECT

public:
    client_widget(QWidget *parent = nullptr,QTcpSocket* Tcpsocket = nullptr);
    ~client_widget();
    void paintEvent(QPaintEvent *) override;
    void initialize(const QString& username);
    void File_handle(MSG_header& header);
    void chunkedFileTransfer(QFile &file);
    void readFileChunks(size_t totalBytes, QFile &file,MSG_header& header);
private slots:
    void onReadyRead();
    void updateLoginTime();
    void sendMsg();
    void readMsg();
    void sendFile();
private:
    bool isRecving = false;
    QString user_name;
    QTcpSocket* tcpsocket;
    Ui::client_widget *ui;
    QDateTime logintime;
    QTimer* timer;
    QProgressBar* progressBar;
};
#endif // CLIENT_WIDGET_H
