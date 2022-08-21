#include "dateset.h"
#include "ui_dateset.h"

#include <QMessageBox>

dateset::dateset(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dateset)
{
    ui->setupUi(this);
    this->setWindowTitle("设置日期");
    this->setFixedSize(300,200);
    ui->dateEdit_start->setDate(QDate::currentDate());
    ui->dateEdit_finish->setDate(QDate::currentDate());
}

dateset::~dateset()
{
    delete ui;
}

void dateset::on_datesetfinish_clicked()
{
    if(ui->dateEdit_start->date()>ui->dateEdit_finish->date()){
         QMessageBox::information(this,"错误","开始时间晚于结束时间");
         return;
    }
    emit send_date(ui->dateEdit_start->date(),ui->dateEdit_finish->date());
    QMessageBox::information(this,"通知","设置成功");
}
