#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dragfile.h"
#include <QList>
#include <QDate>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QProgressDialog>
#include "baiduapi.h"
#include <QtSql/QSqlDatabase>
#include <QMessageBox>
#include <QFileDialog>
#include "dateset.h"
#include "xlsxdocument.h"
#include "xlsxformat.h"
#include "changeuser.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void receiveLogin(QString username);   //接收登录成功的通知
    //获得切换用户之后的用户名
    void getchange_username(QString username);
    int Detection_eye;
    void get_result_dection_eye(int num);
    //获得开始日期和结束日期
    void getdate(QDate start,QDate finish);
    //图像识别
    void pictureclass(QUrl url);
    //健康码眼睛检测
    void health_eye_detection(QString base64,BaiDuAPI *baidu);
    //连接数据库
    void linkdatabase();
    void eye_detection(int num);
    //打开excel表
    void open_Excel();
    //添加数据
    void write_data_health(int row,QString name,QString number,QString health_status,QDate health_time,QXlsx::Format format);
    void write_data_tour(int row,QString phonenumber,QString place,QString tour_status,QDate tour_time,QXlsx::Format format);
private slots:
    void on_cleardata_clicked();
    void on_actionaddperson_triggered();
    void on_pushButton_2_clicked();
    void on_actionsetdate_triggered();
    void on_get_excel_clicked();
    void on_show_user_clicked();
    void on_show_result_clicked();
    void on_actionchangeuser_triggered();
    void on_show_analyze_clicked();
    void on_actiondele_person_triggered();
    void on_actionclear_data_triggered();

signals:
    void class_finish();
private:
    dateset *dateEdit; //日期对象
    ChangeUser *changeusername;  //切换用户对象
    QString m_username;  //保存用户的用户名，用于查询不同的表
    QXlsx::Format format_1;  //excel文字格式
    QProgressDialog *progressDialog;  //进度条
    Ui::MainWindow *ui;
    QList<QUrl> m_url;  //文件的路径
//    QList<QString> m_base64data;  //图片文件转码后的数据
    QNetworkAccessManager *get_accessToken;
    QSqlDatabase db;
    int eye_result;
    QDate m_start,m_finish;  //开始日期和结束日期
    QLabel* status_label;  //显示状态栏状态 数据库状态
    QLabel* status_label_1;  //显示状态栏状态 识别状态
    QXlsx::Document xlsx; //excel表
    QString filename;  //excel文件名称
};
#endif // MAINWINDOW_H
