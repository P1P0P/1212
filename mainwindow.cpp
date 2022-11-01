#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "initialization.h"
#include <QTimer>
#include <QScrollBar>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    socket->connectToHost("176.195.182.169", 53001);

    showMaximized();

    ui->emoji_list->hide();
    ui->emoji_list->setIconSize(QSize(80,80));
    int count = ui->emoji_list->count();
    for(int i = 0; i < count; i++)
    {
      QListWidgetItem *item = ui->emoji_list->item(i);
      item->setSizeHint(QSize(80, 80));
    }
    time=new QTimer(this);
    time->setInterval(1000);
    QObject::connect(time,SIGNAL(timeout()),this,SLOT(refresh()));
    time->start();


    ui->chat_->verticalScrollBar()->setStyleSheet(QString::fromUtf8("QScrollBar:vertical {"
                                                                                           "    border: 1px solid #999999;"
                                                                                           "    background:white;"
                                                                                           "    width:10px;    "
                                                                                           "    margin: 0px 0px 0x 0px;"
                                                                                           "}"
                                                                                           "QScrollBar::handle:vertical {"
                                                                                           "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                                                                                           "    stop: 0 rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130), stop:1 rgb(32, 47, 130));"
                                                                                           "    min-height: 0px;"
                                                                                           "}"
                                                                                           "QScrollBar::add-line:vertical {"
                                                                                           "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                                                                                           "    stop: 0 rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
                                                                                           "    height: 0px;"
                                                                                           "    subcontrol-position: bottom;"
                                                                                           "    subcontrol-origin: margin;"
                                                                                           "}"
                                                                                           "QScrollBar::sub-line:vertical {"
                                                                                           "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                                                                                           "    stop: 0  rgb(32, 47, 130), stop: 0.5 rgb(32, 47, 130),  stop:1 rgb(32, 47, 130));"
                                                                                           "    height: 0 px;"
                                                                                           "    subcontrol-position: top;"
                                                                                           "    subcontrol-origin: margin;"
                                                                                           "}"
                                                                                           ));

}

MainWindow::MainWindow(QString login) : MainWindow()
{
    current_login = login;
    update_chats_list();

    if (QDir(QDir::currentPath() + "/img").exists()){
        qDebug() << "exist";
    }
    else{

        qDebug() << "not exist";
        QDir().mkdir("img");
    }
    //стикеры
    on_checking_triggered();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update_chats_list()
{
    ui->chats_list->clear();
    QString query = "SELECT chat_name FROM pussy_chats JOIN pussy_chats_link ON pussy_chats.chat_id = pussy_chats_link.chat_id WHERE user_login = '" + current_login + "';";
    type = "update_chats_list";
    //query = "select * from pussy_messages;";
    //type = "0";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
}

void MainWindow::on_chats_list_itemClicked(QListWidgetItem *item)
{
    ui->kick->setEnabled(0);
    ui->input_line->clear();
    vector.clear();
    ui->name_label->setText(item->text());
    ui->members_list->clear();

    QStringList list;
    QString temp = "SELECT nickname FROM pussy_chats JOIN pussy_chats_link \
            ON pussy_chats.chat_id = pussy_chats_link.chat_id JOIN pussy_users\
            ON user_login = login WHERE chat_name = '" + item->text() + "' AND login != '" + current_login + "'";//люди группы кроме самого пользователя
    type = "show_members";
    socket->write(temp.toUtf8());
    socket->waitForReadyRead(1000);
    temp = "SELECT main_member FROM pussy_chats JOIN pussy_chats_link \
            ON pussy_chats.chat_id = pussy_chats_link.chat_id JOIN pussy_users\
            ON user_login = login WHERE chat_name = '" + item->text() + "' AND login = '" + current_login + "'";//возможность кикать
    type = "set_kickable";
    socket->write(temp.toUtf8());
    socket->waitForReadyRead(1000);
    if (item->text() != "Users"){
        mode = 1;
        show_chat(item);
        ui->input_line->setEnabled(1);
        ui->input_line->setFocus();
    }
    else{
        mode = 0;
        ui->input_line->setEnabled(0);
        ui->chat_->clear();
    }
    set_item_size();
}

void MainWindow::on_members_list_itemClicked(QListWidgetItem *item)
{
     QString query = "SELECT login FROM pussy_users WHERE nickname = '" + item->text() + "';";
     type = "0";
     socket->write(query.toUtf8());
     socket->waitForReadyRead(1000);
     QStringList list;
     list = QString(socket->readAll()).split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
     if (list[0] != current_login){
         mode = 2;
         ui->name_label->setText(item->text());
         update_chats_list();
         show_dialog(item);
         ui->input_line->setEnabled(1);
         vector.clear();
         ui->kick->setEnabled(0);
         ui->input_line->setFocus();
     }
     else
         on_reconnection_button_triggered();
}

void MainWindow::on_reconnection_button_triggered()
{
    close();
    initialization * init = new initialization;
    init->show();
}


void MainWindow::on_create_group_triggered()
{
    GroupDialog * gd = new GroupDialog(current_login, ui->create_group->text(), "");
    if (gd->exec() == QDialog::Accepted){
        QModelIndex index = ui->chats_list->currentIndex();
        update_chats_list();
        ui->chats_list->setCurrentIndex(index);
    }
}


void MainWindow::on_enter_group_triggered()
{
    GroupDialog * gd = new GroupDialog(current_login, ui->enter_group->text(), "");
    if (gd->exec() == QDialog::Accepted){
        QModelIndex index = ui->chats_list->currentIndex();
        update_chats_list();
        ui->chats_list->setCurrentIndex(index);
    }
}

void MainWindow::show_chat(QListWidgetItem *item)
{
    ui->chat_->clear();
    QFont font;
    font.setPointSize(12);
    ui->chat_->setFont(font);
    last_message_id = 0;
    last_sender = "";
    QString query = "SELECT nickname, message, date, sender_login, message_id FROM pussy_chats_link JOIN pussy_messages\
            ON pussy_chats_link.chat_id = pussy_messages.chat_id JOIN pussy_users\
            ON sender_login = login WHERE chat_name = '" + item->text() + "' ORDER BY 3";
    type = "show_chat";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
}

void MainWindow::show_dialog(QListWidgetItem *item)
{
    ui->chat_->clear();
    QFont font;
    font.setPointSize(12);
    ui->chat_->setFont(font);
    int dialog_id = 0;
    last_message_id = 0;
    last_sender = "";
    QString query = "SELECT login FROM pussy_users WHERE nickname = '" + item->text() + "'";
    type = "0";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
    QString ans = QString(socket->readAll());
    QString login = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts)[0];

    query = "SELECT dialog_id FROM pussy_dialog WHERE user1_login = '" + login + "' AND user2_login = '" + current_login + "'";
    type = "0";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
    ans = QString(socket->readAll());
    if (ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts).isEmpty()){
        query = "SELECT MAX(chat_id) FROM (\
                   SELECT MAX(chat_id) AS chat_id FROM pussy_chats\
                   union\
                   SELECT MAX(dialog_id) AS chat_id FROM pussy_dialog\
                   ) COMBINED";
        type = "0";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
        ans = QString(socket->readAll());
        dialog_id = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts)[0].toInt();
        query = "INSERT INTO pussy_dialog VALUES(" +  QString::number(dialog_id + 1) + ", '" + current_login + "', '" + login + "'), \
                    (" +  QString::number(dialog_id + 1) + ", '" + login + "', '" + current_login + "')";
        type = "0";
        socket->write(query.toUtf8());
        socket->waitForReadyRead(1000);
    }

    query = "SELECT nickname, message, date, sender_login, message_id FROM pussy_dialog JOIN pussy_messages\
                   ON pussy_dialog.dialog_id = pussy_messages.chat_id JOIN pussy_users\
                   ON sender_login = login WHERE user1_login = '" + login + "' AND user2_login = '" + current_login + "' ORDER BY 3";
    type = "show_chat";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
}

void MainWindow::show_message(QVariant val0, QVariant val1, QVariant val2, QVariant val3, QVariant val4)
{
    int prev_pos = 0;

    if (val3.toString() == current_login){
        ui->chat_->setAlignment(Qt::AlignRight);
        if (last_sender != val3.toString()){
            ui->chat_->insertHtml("<div align=\"right\"><font color=\"DeepSkyBlue\">" + val0.toString() + "</font></div>");
            ui->chat_->setTextColor("white");
        }
    }
    else{
        ui->chat_->setAlignment(Qt::AlignLeft);
        if (last_sender != val3.toString()){
            ui->chat_->insertHtml("<div align=\"left\"><font color=\"DeepSkyBlue\">" + val0.toString() + "</font></div>");
            ui->chat_->setTextColor("white");
        }
    }

    ui->chat_->append("");
    last_sender = val3.toString();

    QString query = "SELECT position, image_id FROM pussy_images WHERE message_id = " + QString::number(val4.toInt());
    type = "0";
    socket->write(query.toUtf8());
    socket->waitForReadyRead(1000);
    QString tmp = decrypt(val1.toString());

    QString ans = QString(socket->readAll());
    ans = ans.remove('"').remove('[').remove(']');
    QStringList list = ans.split("), (", QString::SkipEmptyParts);
    if (!list.isEmpty()){
        list[0] = list[0].remove('(');
        list[list.length()-1] = list[list.length()-1].remove(')');
        for (int i = 0; i < list.length(); ++i) {
            QStringList list2 = list[i].split(", ");
            if (!QDir(QDir::currentPath() + "/img/" + list2[1] + ".png").exists()){
                on_checking_triggered();
            }
            ui->chat_->insertPlainText(tmp.left(list2[0].toInt() - prev_pos));
            ui->chat_->insertHtml(QString("<img src=\"%1\" width=\"70\" height=\"70\">").arg((QDir::currentPath() + "/img/" + list2[1] + ".png")));
            tmp = tmp.remove(0, list2[0].toInt() - prev_pos);
            prev_pos = list2[0].toInt() + 1;
        }
    }
    ui->chat_->insertPlainText(tmp);
    ui->chat_->append(val2.toDateTime().toString("ddd hh:mm"));
    ui->chat_->append("");
    ui->chat_->verticalScrollBar()->setValue(ui->chat_->verticalScrollBar()->maximum());
    last_sender = val3.toString();
    last_message_id = val4.toInt();
}

void MainWindow::on_send_button_clicked()
{
    if (ui->input_line->toPlainText().length() != 0){
        int chat_id;
        QString temp = "SELECT MAX(message_id) FROM pussy_messages";
        type = "0";
        socket->write(temp.toUtf8());
        socket->waitForReadyRead(1000);
        QString ans = QString(socket->readAll());
        QString message_id;
        if (!ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts).isEmpty()){
            message_id = QString::number(ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts)[0].toInt() + 1);
        }
        else{
            message_id = QString::number(1);
        }
        if (mode == 2){
            temp = "SELECT login FROM pussy_users WHERE nickname = '" + ui->members_list->currentItem()->text() + "'";
            type = "0";
            socket->write(temp.toUtf8());
            socket->waitForReadyRead(1000);
            QString ans = QString(socket->readAll());
            QString login = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts)[0];

            temp = "SELECT dialog_id FROM pussy_dialog WHERE user1_login = '" + login
                    + "' AND user2_login = '" + current_login + "'";
            type = "0";
            socket->write(temp.toUtf8());
            socket->waitForReadyRead(1000);
            ans = QString(socket->readAll());
            chat_id = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts)[0].toInt();
            QDateTime cur_time = QDateTime::currentDateTime();
            QString current_time = cur_time.toString("yyyy-MM-dd hh:mm:ss");
            temp = "INSERT INTO pussy_messages VALUES(" + message_id + ", " + QString::number(chat_id) + ",'" + current_login + "','"
                    + encrypt(ui->input_line->toPlainText()) + "','" + current_time + "');";
            type = "0";
            socket->write(temp.toUtf8());
            socket->waitForReadyRead(1000);
            while (!vector.empty()){
                temp = "INSERT INTO pussy_images VALUES (" + message_id + ", " + QString::number(vector[0].first) + ", " + QString::number(vector[0].second) + ")";
                type = "0";
                socket->write(temp.toUtf8());
                socket->waitForReadyRead(1000);
                vector.erase(vector.begin());
            }
            show_dialog(ui->members_list->currentItem());
            ui->input_line->clear();
        }
        else if (mode == 1){
            temp = "SELECT chat_id FROM pussy_chats_link WHERE chat_name = '" + ui->chats_list->currentItem()->text()+ "';";
            type = "0";
            socket->write(temp.toUtf8());
            socket->waitForReadyRead(1000);
            ans = QString(socket->readAll());
            chat_id = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts)[0].toInt();
            QDateTime cur_time = QDateTime::currentDateTime();
            QString current_time = cur_time.toString("yyyy-MM-dd hh:mm:ss");
            temp = "INSERT INTO pussy_messages VALUES(" + message_id + ", " + QString::number(chat_id) + ",'" + current_login + "','"
                    + encrypt(ui->input_line->toPlainText()) + "','" + current_time + "');";

            type = "0";
            socket->write(temp.toUtf8());
            socket->waitForReadyRead(1000);
            while (!vector.empty()){
                temp = "INSERT INTO pussy_images VALUES (" + message_id + ", " + QString::number(vector[0].first) + ", " + QString::number(vector[0].second) + ")";
                type = "0";
                socket->write(temp.toUtf8());
                socket->waitForReadyRead(1000);
                vector.erase(vector.begin());
            }
            show_chat(ui->chats_list->currentItem());
            ui->input_line->clear();
        }
        ui->input_line->setFocus();
        ui->chat_->verticalScrollBar()->setValue(ui->chat_->verticalScrollBar()->maximum());
    }
}

void MainWindow::set_item_size()
{
    //размер
    int count = ui->members_list->count();
    for(int i = 0; i < count; i++)
    {
      QListWidgetItem *item = ui->members_list->item(i);
      item->setSizeHint(QSize(item->sizeHint().width(), 50));
    }
    count = ui->chats_list->count();
    for(int i = 0; i < count; i++)
    {
      QListWidgetItem *item = ui->chats_list->item(i);
      item->setSizeHint(QSize(item->sizeHint().width(), 50));
    }
}


void MainWindow::on_emoji_button_clicked(bool checked)
{
    if (checked){
        ui->emoji_list->show();
    }
    else{
        ui->emoji_list->hide();
    }
}

void MainWindow::refresh()
{
    int pos = ui->chat_->verticalScrollBar()->value();
    if (mode == 2){
        QString temp = "SELECT nickname, message, date, sender_login, message_id FROM pussy_dialog JOIN pussy_messages\
                   ON pussy_dialog.dialog_id = pussy_messages.chat_id JOIN pussy_users\
                   ON sender_login = login WHERE user1_login = (SELECT login FROM pussy_users WHERE nickname = '" + ui->members_list->currentItem()->text() + "') \
                   AND user2_login = '" + current_login + "' ORDER BY 3";
        type = "refresh_dialog";
        socket->write(temp.toUtf8());
        socket->waitForReadyRead(1000);
    }
    else if (mode == 1){
        QString temp = "SELECT nickname, message, date, sender_login, message_id  FROM pussy_messages JOIN pussy_chats_link\
                     ON pussy_messages.chat_id = pussy_chats_link.chat_id JOIN pussy_users\
                     ON login = sender_login WHERE chat_name = '" + ui->chats_list->currentItem()->text() + "' ORDER BY 3";
        type = "refresh_dialog";
        socket->write(temp.toUtf8());
        socket->waitForReadyRead(1000);

        /*if (last_message_id != query.value(4).toInt()){ // так и должно быть, тут ошибка если ошибка, повтор кусок
            show_message(query.value(0), query.value(1), query.value(2), query.value(3), query.value(4));
            last_sender = query.value(0).toString();
        }*/
    }

    ui->chat_->verticalScrollBar()->setValue(pos);
}

void MainWindow::on_exit_group_triggered()
{
    GroupDialog * gd = new GroupDialog(current_login, ui->exit_group->text(), "");
    if (gd->exec() == QDialog::Accepted){
        update_chats_list();
        ui->members_list->clear();
        ui->chat_->clear();
        ui->name_label->setText("Выберите группу");
        mode = 0;
        ui->input_line->setEnabled(0);
    }
}

void MainWindow::on_delete_account_triggered()
{
    GroupDialog * gd = new GroupDialog(current_login, ui->delete_account->text(), "");
    if (gd->exec() == QDialog::Accepted)
        on_reconnection_button_triggered();
}

void MainWindow::on_kick_triggered()
{
    GroupDialog * gd = new GroupDialog(current_login, ui->kick->text(), ui->chats_list->currentItem()->text());
    if (gd->exec() == QDialog::Accepted){
        on_chats_list_itemClicked(ui->chats_list->currentItem());
    }
}

void MainWindow::on_emoji_list_itemClicked(QListWidgetItem *item)
{
    if (mode != 0){
        vector.push_back(std::make_pair(ui->input_line->textCursor().position(), ui->emoji_list->currentRow() + 1));
        ui->input_line->insertHtml(QString("<img src=\"%1\" width=\"70\" height=\"70\">").arg((QDir::currentPath() + "/img/" + QString(std::to_string(ui->emoji_list->currentRow() + 1).c_str()) + ".png")));
    }
    ui->input_line->setFocus();
}

void MainWindow::on_input_line_textChanged()
{
    int pos = 0;
    int len = 0;
    if (before.size() > ui->input_line->toPlainText().size()){
        for (int i = 0; i < before.size(); i++){
            if (before[i] != ui->input_line->toPlainText()[i]){
                pos = i;
                len = before.size() - ui->input_line->toPlainText().size();
                break;
            }
        }
        for (int i = 0; i < vector.size(); i++){
            if (pos < vector[i].first){
                if (vector[i].first - pos >= len){
                    vector[i].first = vector[i].first - len;
                }
                else{
                    vector.erase(vector.begin() + i);
                    i--;
                }
            }
        }
    }
    else{
        for (int i = 0; i < before.size(); i++){
            if (before[i] != ui->input_line->toPlainText()[i]){
                pos = i;
                len = ui->input_line->toPlainText().size() - before.size();
                break;
            }
        }
        for (int i = 0; i < vector.size(); i++){
            if (pos <= vector[i].first){
                vector[i].first = vector[i].first + len;
            }
        }
    }

    before = ui->input_line->toPlainText();
}


QString MainWindow::encrypt(QString input)
{
    QString alphabet = "ЛрвДn9JiЖОжРзHTsЙПhjмЭАbyGпЯкmf2Ц Y0УЧВKоgcКIFСQM5l4ёUМщЫLИeЕNТRvЬНpБ71ГЮDэBФШEЩkЗZoсюVХъOеSXЪляqаaWCшrунйцдчtтPфwыхdиA63бёгuь8zx";
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] != "￼")
            input.replace(i, 1, alphabet[(alphabet.indexOf(input[i]) + 3) % 157]);
        else{
            input.remove(i, 1);
            i--;
        }
    }
    return input;
}

QString MainWindow::decrypt(QString input)
{
    QString alphabet = "ЛрвДn9JiЖОжРзHTsЙПhjмЭАbyGпЯкmf2Ц Y0УЧВKоgcКIFСQM5l4ёUМщЫLИeЕNТRvЬНpБ71ГЮDэBФШEЩkЗZoсюVХъOеSXЪляqаaWCшrунйцдчtтPфwыхdиA63бёгuь8zx";
    for (int i = 0; i < input.size(); i++){
        if (alphabet.contains(input[i]))
            input.replace(i, 1, alphabet[(alphabet.indexOf(input[i]) - 3 + 157) % 157]);
    }
    return input;
}

void MainWindow::on_add_sticker_triggered()
{
    QString a = "send_image";
    QString path = QFileDialog::getOpenFileName(this, "Выбрать картинку", "", "PNG Image (*.png);");
    if (!path.isEmpty() and !path.isNull()){
        sticker = new QFile(path);
        sticker->open(QIODevice::ReadOnly);
        type = "0";
        socket->write(a.toUtf8());
        socket->waitForReadyRead(1000);
        auto ans = sticker->read(4096);
        while(!ans.isEmpty()){
            socket->write(ans);
            socket->waitForReadyRead(1000);
            ans = sticker->read(4096);
        }
        socket->write(QString("end").toUtf8());
        sticker->close();
        delete sticker;
    }
    on_checking_triggered();
}

void MainWindow::on_checking_triggered()
{
    QDir dir;
    dir.setPath(QDir::currentPath() + "/img");
    int exist = dir.entryList().size() - 2;
    qDebug() << exist;
    socket->write(QString("get_image").toUtf8());
    socket->waitForReadyRead(1000);
    type = "0";
    socket->write(QString::number(exist).toUtf8());
    socket->waitForReadyRead(1000);
    auto loading = socket->readAll();
    qDebug() << loading;
    for(int i = 1; i <= loading.toInt(); ++i){
        socket->write(QString("start").toUtf8());
        socket->waitForReadyRead(1000);
        sticker = new QFile(QDir::currentPath() + "/img/" + QString::number(exist + i) + ".png");
        sticker->open(QIODevice::WriteOnly);
        auto ans = socket->read(4096);
        socket->waitForReadyRead(200);
        while(!ans.isEmpty()){
            sticker->write(ans);
            socket->waitForReadyRead(200);
            ans = socket->read(4096);
            socket->waitForReadyRead(200);
        }
        socket->write(QString("end").toUtf8());
        socket->waitForReadyRead(1000);
        sticker->close();
        delete sticker;
    }
    ui->emoji_list->clear();
    for (int i = 1; i <= exist + loading.toInt(); ++i) {
        ui->emoji_list->addItem(new QListWidgetItem(QIcon(QDir::currentPath() + "/img/" + QString::number(i) + ".png"), NULL));
    }

}

void MainWindow::on_exit_triggered()
{
    close();
    QApplication::quit();
}

void MainWindow::slotReadyRead()
{
    if (type == "update_chats_list"){
        QString ans = QString(socket->readAll());
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        ui->chats_list->addItems(list);
        set_item_size();
    }
    else if (type == "show_members"){
        QString ans = QString(socket->readAll());
        QStringList list;

        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        ui->members_list->addItems(list);
    }
    else if (type == "set_kickable"){
        QString ans = QString(socket->readAll());
        QStringList list;
        list = ans.split(QRegularExpression("[^a-zA-Zа-яА-Я0-9]"), QString::SkipEmptyParts);
        if (list[0] == "true"){
            ui->kick->setEnabled(1);
        }
    }
    else if (type == "refresh_dialog"){
        QString ans = QString(socket->readAll());
        ans = ans.remove('"').remove('[').remove(']');
        QStringList list = ans.split("), (", QString::SkipEmptyParts);
        if (!list.isEmpty()){
            ans = list[list.length()-1].remove(')');
            list = ans.split(", ");
            if (last_message_id != list[4].toInt()){
                    show_message(list[0], list[1], list[2], list[3], list[4]);
                    last_sender = list[0];
            }
        }
    }
    else if (type == "show_chat"){
        QString ans = QString(socket->readAll());
        ans = ans.remove('"').remove('[').remove(']');
        QStringList list = ans.split("), (", QString::SkipEmptyParts);
        if (!list.isEmpty()){
            list[0] = list[0].remove('(');
            list[list.length()-1] = list[list.length()-1].remove(')');
            for (int i = 0; i < list.length(); ++i) {
                QStringList list2 = list[i].split(", ");
                show_message(list2[0].remove("'"), list2[1].remove("'"), list2[2].remove("'"), list2[3].remove("'"), list2[4]);
            }
        }
    }
}
