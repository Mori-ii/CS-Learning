#ifndef LOG_IN_H
#define LOG_IN_H

#include <QDialog>
#include <QMouseEvent>
#include <QMessageBox>
#include <QTcpSocket>

namespace Ui
{
class log_in;
}

class log_in : public QDialog
{
    Q_OBJECT

public:
    log_in(QWidget *parent = nullptr);
    ~log_in();
    void onConnected();
    void onError(QAbstractSocket::SocketError error);
    QString getname();
    QTcpSocket* getsocket();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
private slots:
    void HandleLoginButton_clicked();
private:
    Ui::log_in *ui;
    bool isDragging;
    QPoint dragPosition;
    QTcpSocket *tcpSocket;
};

#endif // LOG_IN_H
