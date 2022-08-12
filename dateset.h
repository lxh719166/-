#ifndef DATESET_H
#define DATESET_H

#include <QWidget>
#include <QDebug>
#include <QDate>
namespace Ui {
class dateset;
}

class dateset : public QWidget
{
    Q_OBJECT

public:
    explicit dateset(QWidget *parent = nullptr);
    ~dateset();

private slots:
    void on_datesetfinish_clicked();
signals:
    void send_date(QDate start,QDate finish);
private:
    Ui::dateset *ui;
};

#endif // DATESET_H
