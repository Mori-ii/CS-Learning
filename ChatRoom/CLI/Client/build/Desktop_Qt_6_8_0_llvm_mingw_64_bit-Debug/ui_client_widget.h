/********************************************************************************
** Form generated from reading UI file 'client_widget.ui'
**
** Created by: Qt User Interface Compiler version 6.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENT_WIDGET_H
#define UI_CLIENT_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_client_widget
{
public:
    QLineEdit *lineEdit;

    void setupUi(QWidget *client_widget)
    {
        if (client_widget->objectName().isEmpty())
            client_widget->setObjectName("client_widget");
        client_widget->resize(564, 367);
        client_widget->setStyleSheet(QString::fromUtf8("image: url(:/backgroud/ATRI2.jpg);"));
        lineEdit = new QLineEdit(client_widget);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setGeometry(QRect(40, 270, 381, 41));

        retranslateUi(client_widget);

        QMetaObject::connectSlotsByName(client_widget);
    } // setupUi

    void retranslateUi(QWidget *client_widget)
    {
        client_widget->setWindowTitle(QCoreApplication::translate("client_widget", "Client", nullptr));
    } // retranslateUi

};

namespace Ui {
    class client_widget: public Ui_client_widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENT_WIDGET_H
