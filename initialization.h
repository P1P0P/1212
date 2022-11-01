#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <QDialog>
#include <QtSql>
#include <QTcpSocket>

namespace Ui {
class initialization;
}

class initialization : public QDialog
{
    Q_OBJECT

public:
    explicit initialization(QWidget *parent = nullptr);
    ~initialization();

public slots:
    void slotReadyRead();

private slots:

    void on_reg_button_clicked();

    void on_log_button_clicked();

    void on_login_box_currentIndexChanged(const QString &arg1);

    void on_reg2_button_clicked();

    void on_enter_button_clicked();

private:
    Ui::initialization *ui;
    QSqlDatabase local_db;
    QTcpSocket *socket;
    QString type;
};

#endif // INITIALIZATION_H
