#include "registered.h"
#include "ui_registered.h"

#include <QSqlQuery>

registered::registered(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::registered)
{
    ui->setupUi(this);
    ui->res_username->setPlaceholderText("请输入用户名");
    ui->res_password->setPlaceholderText("请输入密码");
    ui->res_password_again->setPlaceholderText("请再次输入密码");
    //首先连接数据库
    linkdatabase();



}

registered::~registered()
{
    delete ui;
}

void registered::linkdatabase()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("lu719166");
    db.open();
}

void registered::on_pushButton_clicked()
{
    //获得输入框的文本
    QString username = ui->res_username->text();
    QString password = ui->res_password->text();
    QString psssword_again = ui->res_password_again->text();

    //判断两次密码输入是否正确
    if(password != psssword_again){
        QMessageBox::information(this,"警告","两次密码输入不一致");
        //清空再次输入密码的框
        ui->res_password_again->clear();
    }
    //判断用户名是否存在
    QSqlQuery result_user;
    result_user.prepare("SELECT count(*) FROM user where username=? limit 1;");
    result_user.addBindValue(username);
    result_user.exec();
    if(username!=NULL){
    if(result_user.first()){
        if(result_user.value(0).toInt()==1){
            QMessageBox::information(this,"警告","该用户名已存在");
        }else{
            result_user.prepare("insert into user(username,password) values(?,?);");
            result_user.addBindValue(username);
            result_user.addBindValue(password);
            result_user.exec();
            //创建该用户所用到的表
            db.exec("create table "+username+"_person(name varchar(20),phonenumber varchar(20));");
            db.exec("create table "+username+"_health(name varchar(20),number varchar(20),date varchar(20),status varchar(20));");
            db.exec("create table "+username+"_tour(phonenumber varchar(20),date varchar(20),place varchar(20),status varchar(20));");
            QMessageBox::information(this,"通知","注册成功！");
            emit sendusername(username);
            this->close();
        }
    }}

}
