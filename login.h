#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QtSql/QSqlDatabase>
#include "registered.h"
#include <QDebug>
namespace Ui {
class login;
}

class login : public QWidget
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login();
    //连接数据库
    void linkdatabase();

signals:
     void showmain();
private slots:
    void on_btn_regi_clicked();

    void on_btn_login_clicked();

private:
    Ui::login *ui;
    QSqlDatabase db;
    registered *regis;
};

#endif // LOGIN_H
