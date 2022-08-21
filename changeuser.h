#ifndef CHANGEUSER_H
#define CHANGEUSER_H

#include <QSqlDatabase>
#include <QWidget>
#include <QtSql/QSqlDatabase>
namespace Ui {
class ChangeUser;
}

class ChangeUser : public QWidget
{
    Q_OBJECT

public:
    explicit ChangeUser(QWidget *parent = nullptr);
    ~ChangeUser();
    void linkdatabase();

private slots:
    void on_btn_ChUser_clicked();
signals:
    void send_change_user(QString username);
private:
    Ui::ChangeUser *ui;
    QSqlDatabase db;
};

#endif // CHANGEUSER_H
