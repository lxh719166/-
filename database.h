#ifndef DATABASE_H
#define DATABASE_H
#include <QtSql/QSqlDatabase>
#include <QDebug>
class DataBase
{
public:
    DataBase();
    //连接数据库
    void linkdatabase();
};

#endif // DATABASE_H
