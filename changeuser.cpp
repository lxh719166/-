#include "changeuser.h"
#include "ui_changeuser.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>

ChangeUser::ChangeUser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChangeUser)
{
    ui->setupUi(this);
    this->setStyleSheet("QPushButton:pressed,#show_analyze:pressed{border-style:solid;border-radius: 5px;width: 100px;height: 40px;color:#EBEBD3;background-color:#38523d} "
                        "QPushButton{border-style:solid;border-radius: 5px;width: 100px;height: 40px;color:#EBEBD3;background-color:#708B75}");
    this->setWindowTitle("切换用户");
    linkdatabase();  //连接数据库
}

ChangeUser::~ChangeUser()
{
    delete ui;
}

void ChangeUser::linkdatabase()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("lu719166");
    db.open();
}

void ChangeUser::on_btn_ChUser_clicked()
{
    //得到用户名框中的信息
    QString username = ui->username->text();
    //得到用户名框中的信息
    QString password = ui->password->text();
    QString password_Database;
    //与数据库中的信息进行对比
    //1.判断密码是否正确
    QSqlQuery password_list;
    password_list.prepare("select password from user where username=?;");
    password_list.addBindValue(username);
    password_list.exec();
    if(password_list.first())
        password_Database = password_list.value(password_list.record().indexOf("password")).toString();
    if(password!=NULL){
        if(password == password_Database){   //密码正确的情况
            emit send_change_user(username);
            db.close();
            this->close();
            QMessageBox::information(this,"通知","用户切换成功");
        }else if(password != password_Database){
            QMessageBox::information(this,"警告","密码错误，请重新输入");
        }}
}
