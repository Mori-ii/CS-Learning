/********************************************************************************
** Form generated from reading UI file 'log_in.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOG_IN_H
#define UI_LOG_IN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_log_in
{
public:
    QPushButton *LoginButton;
    QPushButton *ExitButton;
    QLabel *label_2;
    QLineEdit *user_name;
    QLineEdit *server_port;
    QLineEdit *server_ip;
    QLabel *label_5;
    QLabel *label_3;
    QLabel *label_4;

    void setupUi(QDialog *log_in)
    {
        if (log_in->objectName().isEmpty())
            log_in->setObjectName("log_in");
        log_in->resize(400, 300);
        log_in->setMinimumSize(QSize(400, 300));
        log_in->setMaximumSize(QSize(400, 300));
        log_in->setStyleSheet(QString::fromUtf8("#log_in\n"
"{\n"
"	image: url(:/backgroud/ATRI.jpg);\n"
"}"));
        LoginButton = new QPushButton(log_in);
        LoginButton->setObjectName("LoginButton");
        LoginButton->setGeometry(QRect(100, 200, 61, 31));
        LoginButton->setAutoFillBackground(false);
        LoginButton->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 50);\n"
"font: 700 9pt \"Microsoft YaHei UI\";\n"
"text-decoration: underline;\n"
""));
        LoginButton->setFlat(true);
        ExitButton = new QPushButton(log_in);
        ExitButton->setObjectName("ExitButton");
        ExitButton->setGeometry(QRect(240, 200, 61, 31));
        ExitButton->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 50);\n"
"font: 700 9pt \"Microsoft YaHei UI\";\n"
"text-decoration: underline;"));
        ExitButton->setFlat(true);
        label_2 = new QLabel(log_in);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(99, 100, 71, 20));
        label_2->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        user_name = new QLineEdit(log_in);
        user_name->setObjectName("user_name");
        user_name->setGeometry(QRect(170, 100, 131, 20));
        user_name->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 100);\n"
"color: rgb(0, 0, 0);"));
        user_name->setFrame(false);
        server_port = new QLineEdit(log_in);
        server_port->setObjectName("server_port");
        server_port->setGeometry(QRect(170, 160, 131, 20));
        server_port->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 100);\n"
"color: rgb(0, 0, 0);"));
        server_port->setFrame(false);
        server_ip = new QLineEdit(log_in);
        server_ip->setObjectName("server_ip");
        server_ip->setGeometry(QRect(170, 130, 131, 20));
        server_ip->setStyleSheet(QString::fromUtf8("background-color: rgba(255, 255, 255, 100);\n"
"color: rgb(0, 0, 0);"));
        server_ip->setFrame(false);
        label_5 = new QLabel(log_in);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(80, 0, 251, 91));
        label_3 = new QLabel(log_in);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(100, 130, 71, 20));
        label_3->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        label_4 = new QLabel(log_in);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(99, 160, 71, 20));
        label_4->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);"));
        label_5->raise();
        LoginButton->raise();
        ExitButton->raise();
        label_2->raise();
        user_name->raise();
        server_port->raise();
        server_ip->raise();
        label_3->raise();
        label_4->raise();

        retranslateUi(log_in);
        QObject::connect(ExitButton, &QPushButton::clicked, log_in, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(log_in);
    } // setupUi

    void retranslateUi(QDialog *log_in)
    {
        log_in->setWindowTitle(QCoreApplication::translate("log_in", "Log in", nullptr));
        LoginButton->setText(QCoreApplication::translate("log_in", "\347\231\273\345\275\225", nullptr));
        ExitButton->setText(QCoreApplication::translate("log_in", "\351\200\200\345\207\272", nullptr));
        label_2->setText(QCoreApplication::translate("log_in", "<html><head/><body><p><span style=\" font-weight:700; font-style:italic;\">\346\234\215\345\212\241\345\231\250IP</span></p></body></html>", nullptr));
        label_5->setText(QCoreApplication::translate("log_in", "<html><head/><body><p align=\"center\"><span style=\" font-size:14pt; font-weight:700; font-style:italic;\">\346\254\242\350\277\216\346\235\245\345\210\260Online ChatRoom</span></p></body></html>", nullptr));
        label_3->setText(QCoreApplication::translate("log_in", "<html><head/><body><p><span style=\" font-weight:700; font-style:italic;\">\346\234\215\345\212\241\345\231\250\347\253\257\345\217\243</span></p></body></html>", nullptr));
        label_4->setText(QCoreApplication::translate("log_in", "<html><head/><body><p><span style=\" font-weight:700; font-style:italic;\">\347\224\250\346\210\267\345\220\215</span></p></body></html>", nullptr));
    } // retranslateUi

};

namespace Ui {
    class log_in: public Ui_log_in {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOG_IN_H
