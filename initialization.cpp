#include "initialization.h"
#include "ui_initialization.h"
#include "mainwindow.h"

initialization::initialization(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::initialization)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &initialization::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    socket->connectToHost("176.195.182.169", 53001);

    ui->sign_up_widget->hide();
    setFixedSize(500, 300);

    //логины
    local_db = QSqlDatabase::addDatabase("QSQLITE", "local_db");
    local_db.setDatabaseName("db_password");
    local_db.open();

    QSqlQuery query(local_db);
    query.exec("CREATE TABLE passwords(login TEXT PRIMARY KEY, password TEXT)");

    QStringList list;
    query.exec("SELECT login FROM passwords");
    while (query.next()){
        list << query.value(0).toString();
    }
    ui->login_box->addItems(list);
}

initialization::~initialization()
{
    delete ui;
}


void initialization::on_reg_button_clicked()
{
    ui->sign_in_widget->hide();
    ui->sign_up_widget->show();
    ui->label_4->setText("Уже есть аккаунт?");
}


void initialization::on_log_button_clicked()
{
    ui->sign_up_widget->hide();
    ui->sign_in_widget->show();
}


void initialization::on_login_box_currentIndexChanged(const QString &arg1)
{
    QSqlQuery query(local_db);
    query.exec("SELECT password FROM passwords WHERE login = '" + arg1 + "'");
    query.next();
    ui->password_line->setText(query.value(0).toString());
}


void initialization::on_enter_button_clicked() //вход
{
    QString query = "SELECT login,password FROM pussy_users WHERE login ='"
            + ui->login_box->currentText() + "' and password ='"
            + ui->password_line->text() + "';";
    type = "enter";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
}


void initialization::on_reg2_button_clicked() //регистрация
{
    ui->label_4->setText("");
    QString query = "SELECT login FROM pussy_users WHERE login ='" + ui->login_line->text() + "';";
    type = "reg";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
}

void initialization::slotReadyRead()
{
    QString ans = QString(socket->readAll());
    QString query;
    if (type == "enter"){
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        if (!list.isEmpty()){
            if (ui->remember_button->isChecked()){
                QSqlQuery query2(local_db);
                query = "INSERT INTO passwords VALUES('" + ui->login_box->currentText() + "','" + ui->password_line->text() + "')";
                query2.exec(query);
            }
            close();
            MainWindow *mainWindow;
            mainWindow = new MainWindow(ui->login_box->currentText());
            mainWindow->show();
        }
        else{
            ui->error_label->setText("Ошибка");
        }
    }
    else if (type == "reg"){
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        if (list.isEmpty()){
            ui->label_4->setText("Регистрация прошла успешна");
            query = "INSERT INTO pussy_users VALUES('" + ui->login_line->text()
                    + "','" + ui->password_line_3->text()
                    + "','" + ui->nickname_line->text() + "');";
            type = "0";
            socket->write(query.toUtf8());
            socket->waitForReadyRead(1000);
            query = "INSERT INTO pussy_chats VALUES(1,'" + ui->login_line->text() + "','false');";
            socket->write(query.toUtf8());
            socket->waitForReadyRead(1000);
        }
        else{
            ui->label_4->setText("Такой логин уже существует");
        }
    }
}


