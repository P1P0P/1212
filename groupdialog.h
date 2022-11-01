#ifndef GROUPDIALOG_H
#define GROUPDIALOG_H

#include <QDialog>
#include <QtSql>
#include <mainwindow.h>
#include <QTcpSocket>

namespace Ui {
class GroupDialog;
}

class GroupDialog : public QDialog
{
    Q_OBJECT

public:
     GroupDialog(QWidget *parent = nullptr);
     GroupDialog(QString, QString, QString);
    ~GroupDialog();

public slots:
     void slotReadyRead();

private slots:

    void on_buttonBox_clicked();

    void on_pushButton_clicked();

private:
    Ui::GroupDialog *ui;
    QTcpSocket *socket;
    QString type;
    QString m_login;
    QString m_choice;
    QString m_cur_chat;
};

#endif // GROUPDIALOG_H
