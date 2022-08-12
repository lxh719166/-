#include "database.h"

DataBase::DataBase()
{

}

void DataBase::linkdatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("223.90.48.208");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setUserName("root");
    db.setPassword("lu719166");
    bool ok = db.open();
    if (ok){
//        qDebug()<<"数据库连接成功";
    }
    else {
//        qDebug()<<"数据库连接失败";
    }
}
