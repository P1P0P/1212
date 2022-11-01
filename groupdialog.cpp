#include "groupdialog.h"
#include "ui_groupdialog.h"


GroupDialog::GroupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GroupDialog)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &GroupDialog::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    socket->connectToHost("176.195.182.169", 53001);
}

GroupDialog::GroupDialog(QString login, QString choice, QString cur_chat): GroupDialog(){
    m_login = login;
    m_choice = choice;
    m_cur_chat = cur_chat;
    if(m_choice == "Войти в группу"){
        ui->buttonBox->setText("Войти в группу");
    }
    else if(m_choice == "Выйти из группы"){
        ui->buttonBox->setText("Выйти из группы");
    }
    else if (m_choice == "Удалить аккаунт"){
        ui->group_edit->hide();
        ui->label->setText("Вы уверены, что хотите удалить аккаунт?");
        ui->label->setGeometry(80,100,300,30);
        ui->buttonBox->setText("Да, удалить");
    }
    else if (m_choice == "Выгнать из группы"){
        ui->label->setText("Кого Вы хотите выгнать?");
        ui->buttonBox->setText("Выгнать");
    }
}

GroupDialog::~GroupDialog()
{
    delete ui;
}


void GroupDialog::on_buttonBox_clicked()
{
    QString query;
    if(m_choice == "Создать группу"){
        query = "SELECT MAX(chat_id) FROM (\
                SELECT MAX(chat_id) AS chat_id FROM pussy_chats_link\
                union\
                SELECT MAX(dialog_id) AS chat_id FROM pussy_dialog\
                ) COMBINED;";
        type = "create_group";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
    }
    else if (m_choice == "Выйти из группы"){
        query = "SELECT chat_id FROM pussy_chats_link WHERE chat_name = '" + ui->group_edit->toPlainText() + "';";
        type = "exit_group";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
    }
    else if (m_choice == "Удалить аккаунт"){
        query = "UPDATE pussy_users SET nickname = 'DELETED', password = '3YHwrB3Y434f874b' WHERE login = '" + m_login +"'";
        type = "0";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
        accept();
    }
    else if (m_choice == "Войти в группу"){
        query = "SELECT chat_id FROM pussy_chats_link WHERE chat_name = '" + ui->group_edit->toPlainText() + "';";
        type = "enter_group";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
    }
    else{ // kick
        query = "SELECT chat_id FROM pussy_chats_link WHERE chat_name = '" + m_cur_chat + "'";
        type = "kick";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
    }
}

void GroupDialog::on_pushButton_clicked()
{
    close();
}

void GroupDialog::slotReadyRead()
{
    QString ans = QString(socket->readAll());
    QString query;
    if (type == "create_group"){
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);

        query = "INSERT INTO pussy_chats_link VALUES (" + QString::number(list[0].toInt()+1)+ ",'" + ui->group_edit->toPlainText() + "');";
        type = "check_exist";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
        QStringList list2;
        list2 = QString(socket->readAll()).split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        if (!list2.isEmpty()){
            ui->error_label->setText("Группа с таким названием уже существует");
        }
        else{
            query = "INSERT INTO pussy_chats VALUES(" + QString::number(list[0].toInt()+1) + ",'" + m_login + "','true');";
            type = "accept";
            socket->write(query.toUtf8());
            socket->waitForReadyRead(1000);
            accept();
        }
    }
    else if (type == "exit_group"){
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        if (!list.isEmpty()){
            if (list[0] == "1"){
                ui->error_label->setText("Из этой группы нельзя выйти");
            }
            else{
                query = "DELETE FROM pussy_chats WHERE chat_id =" + QString::number(list[0].toInt())+ " and user_login = '" + m_login + "';";
                type = "accept";
                socket->write(query.toUtf8());
                socket->waitForReadyRead(1000);
                accept();
            }
        }
        else{
            ui->error_label->setText("Вы не состоите в этой группе");
        }
    }
    else if (type == "enter_group"){
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        if (list.isEmpty()){
            ui->error_label->setText("Такой группы не существует");
        }
        else{
            if (list[0] == "1"){
                ui->error_label->setText("Вы уже состоите в этой группе");
            }
            else{
                query = "INSERT INTO pussy_chats VALUES(" + list[0] + ",'" + m_login + "','false');";
                type = "accept";
                socket->write(query.toUtf8());
                socket->waitForReadyRead(1000);
                accept();
            }
        }
    }
    else if (type == "kick"){
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        query = "DELETE FROM pussy_chats WHERE chat_id = " + list[0] + " AND user_login = '"
                   + ui->group_edit->toPlainText() + "'";
        type = "accept";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
        accept();
    }
}

