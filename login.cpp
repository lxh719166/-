#include "login.h"
#include "ui_login.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlRecord>

login::login(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    ui->username->setPlaceholderText("请输入用户名");
    ui->password->setPlaceholderText("请输入密码");
    this->setWindowTitle("登录界面");
    //连接数据库
    linkdatabase();
    connect(regis,&registered::sendusername,[=](QString usn){
        ui->username->setText(usn);
    });
}

login::~login()
{
    delete ui;
}

void login::linkdatabase()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("lu719166");
    db.open();
}

void login::on_btn_regi_clicked()
{
    regis = new registered;
    regis->show();
}

void login::on_btn_login_clicked()
{
    //得到用户名框中的信息
    QString username = ui->username->text();
    //得到用户名框中的信息
    QString password = ui->password->text();
    QString password_Database;
    //与数据库中的信息进行对比
    //1、对比数据库中是否有该用户名
    QSqlQuery username_exist;
    username_exist.prepare("SELECT count(*) FROM user where username=? limit 1;");
    username_exist.addBindValue(username);
    username_exist.exec();
    if(username!=NULL){
    if(username_exist.first()){
        if(username_exist.value(0).toInt()==0){
        QMessageBox::StandardButton result;
        result= QMessageBox::question(this,"消息","用户名不存在，是否进行注册？",QMessageBox::Yes|QMessageBox::No);
        if(result == QMessageBox::Yes){
            emit ui->btn_regi->clicked();
            return;}
        else{
            return;
        }
    }}}
    //2.判断密码是否正确
    QSqlQuery password_list;
    password_list.prepare("select password from user where username=?;");
    password_list.addBindValue(username);
    password_list.exec();
    if(password_list.first())
        password_Database = password_list.value(password_list.record().indexOf("password")).toString();
    if(password!=NULL){
        if(password == password_Database){
            emit showmain();
            db.close();
            this->close();
        }else{
            QMessageBox::information(this,"警告","密码错误，请重新输入");
        }}
}
