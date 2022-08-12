#ifndef REGISTERED_H
#define REGISTERED_H

#include <QWidget>
#include <QtSql/QSqlDatabase>
#include <QMessageBox>
namespace Ui {
class registered;
}

class registered : public QWidget
{
    Q_OBJECT

public:
    explicit registered(QWidget *parent = nullptr);
    ~registered();
    //连接数据库
    void linkdatabase();

signals:
    void sendusername(QString un);
private slots:
    void on_pushButton_clicked();

private:
    Ui::registered *ui;
    QSqlDatabase db;
};

#endif // REGISTERED_H
