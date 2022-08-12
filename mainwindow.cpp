#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateEdit>
#include <QFileDialog>
#include <QImage>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QProgressDialog>
#include <QSqlQuery>
#include <QTextCodec>
#include <xlsxformat.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    BaiDuAPI *baidu = new BaiDuAPI;  //创建百度API对象
    setWindowTitle("双码智能识别系统");
    setWindowIcon(QIcon(":/res/icon.png"));
    //设置状态栏显示
    ui->statusBar->addWidget(new QLabel("当前状态："));
    status_label = new QLabel;
    status_label->setText("未连接数据库");
    status_label_1 = new QLabel;
    status_label_1->setText("未开始识别");
    ui->statusBar->addWidget(status_label);
    ui->statusBar->addWidget(status_label_1);

    //得到设置的时间
    dateset* date = new dateset;
    connect(date,&dateset::send_date,this,&MainWindow::getdate);
    //得到拖入文件的文件路径
    connect(ui->listWidget,&DragFile::sendUrlList,[=](QList<QUrl> url){
        for(int i=0;i<url.size();i++){
            m_url.push_back(url.at(i).toLocalFile());
        }
    });

    //点击开始识别之后的处理
    connect(ui->startrecon,&QPushButton::clicked,[=](){
        if(m_url.isEmpty()){
            QMessageBox::information(this,"错误","未上传数据");
            return;
        }
        //        status_label_1->setText("正在识别中"+QString::number(baidu->picindex)+"/"+QString::number(this->m_url.size()));
        status_label_1->setText("正在识别中");
        //        if(!QSslSocket::supportsSsl()){
        //            QMessageBox::information(this,"错误","系统不支持openssl");
        //            return;
        //        }
        progressDialog = new QProgressDialog(this);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setMinimumDuration(5);
        progressDialog->setWindowTitle("请等待");
        progressDialog->setLabelText("识别中....");
        progressDialog->setCancelButtonText("取消");
        progressDialog->setRange(0,m_url.size());
        pictureclass(m_url.at(0));
    });
}

MainWindow::~MainWindow()

{
    delete ui;
}

void MainWindow::getdate(QDate start, QDate finish)
{
    this->m_start = start;
    this->m_finish = finish;
}

void MainWindow::pictureclass(QUrl url)
{
    QString base64 =BaiDuAPI::imageToBase64(QImage(url.toString()));
    BaiDuAPI *baidu = new BaiDuAPI;  //创建百度API对象
    qDebug()<<"当前任务："<<BaiDuAPI::picindex+1<<"/"<<m_url.size();
    //设置进度条进度
    progressDialog->setValue(BaiDuAPI::picindex+1);
    //接收传过来的识别数据
    connect(baidu,&BaiDuAPI::send_classific_result,[=](QString jiankang,double value_1,QString xingcheng,double value_2,QString qita,double value_3){
        if(value_1 > value_2 && value_1>value_3){  //健康码的情况
            //显示分类结果
            ui->listresult->addItem(jiankang);
            //对健康码上的小眼睛进行检测，如果没开，信息入库，如果开了，进行OCR
            health_eye_detection(base64,baidu);
            //显示小眼睛识别结果
            connect(baidu,&BaiDuAPI::send_detection_result_eye,[=](int result){
                //                qDebug()<<"眼睛识别结果:"<<result;
                if(result){
                    ui->listresult->addItem("眼睛开启");
                    //健康码的OCR识别
                } else{  //眼睛关闭信息入库
                    QSqlQuery table = db.exec("select * from eye_detection");
                    //插入数据
                    table.prepare("insert into eye_detection(filename,eyestatus) values(?,?);");
                    table.addBindValue(url.fileName());
                    table.addBindValue(result);
                    table.exec();
                }
                //判断识别识别完成
                if(baidu->picindex<m_url.size())
                    pictureclass(m_url.at(baidu->picindex));
                else{
                    status_label_1->setText("识别完成");
                    QMessageBox::information(this,"提醒","识别完成");
                }

            });
            //对健康码进行OCR识别，信息入库
        }
        else if(value_2>value_1&&value_2>value_3){ //行程码的情况
            //显示分类结果
            //            qDebug()<<"行程码";
            ui->listresult->addItem(xingcheng);
            //对行程码上的码的状态进行检测，信息入库
            //对行程码进行OCR识别

            //判断识别识别完成
            if(baidu->picindex<m_url.size())
                pictureclass(m_url.at(baidu->picindex));
            else{
                status_label_1->setText("识别完成");
                QMessageBox::information(this,"提醒","识别完成");
            }
        }
        else if(value_3>value_1&&value_3>value_2){ //其他的情况
            //qDebug()<<"其他";
            ui->listresult->addItem(qita);
            //qDebug()<<"当前图片index："<<baidu->picindex;
            if(baidu->picindex<m_url.size())
                pictureclass(m_url.at(baidu->picindex));
            else{
                status_label_1->setText("识别完成");
                QMessageBox::information(this,"提醒","识别完成");
            }
        }
    });

    baidu->on_recognize(base64);  //开始识别

}

int MainWindow::health_eye_detection(QString base64,BaiDuAPI *baidu)
{
    baidu->Image_Detection(base64);
}

void MainWindow::linkdatabase()
{
    db.setUserName("root");
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("223.90.49.208");
    db.setPort(3306);
    db.setDatabaseName("mysql");
    db.setPassword("lu719166");
    bool ok = db.open();
    if (db.isOpen()){
        QMessageBox::information(this,"通知","数据库已成功连接");
        status_label->setText("数据库已连接");
    }
    else {
        QMessageBox::information(this,"通知","数据库连接失败");
        status_label->setText("数据库连接失败");
    }
}

void MainWindow::eye_detection(int num)
{
    eye_result = num;
}

void MainWindow::open_Excel()  //打开Excel
{
    filename = QFileDialog::getSaveFileName(this, "保存文件",
                                            "./识别结果.xlsx",
                                            "Microsoft Excel文件(*.xlsx)");
    //标题格式
    QXlsx::Format format;
    format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format.setFontSize(16);
    //正文格式
    format_1.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
    format_1.setVerticalAlignment(QXlsx::Format::AlignVCenter);
    format_1.setFontSize(11);
    //表格设置
    xlsx.setRowHeight(1,100,20);
    xlsx.setRowHeight(1,40);
    xlsx.setRowHeight(2,25);
    xlsx.setColumnWidth(1,9,20);
    xlsx.setColumnWidth(5,8);
    //添加数据
    //1.设置标题
    xlsx.mergeCells("A1:D1",format);
    xlsx.write("A1", "健康码",format);
    xlsx.mergeCells("F1:I1",format);
    xlsx.write("F1", "行程码",format);
    //2.设置表头
    xlsx.write("A2","姓名",format_1);
    xlsx.write("B2","身份证号",format_1);
    xlsx.write("C2","健康码状态",format_1);
    xlsx.write("D2","健康码提交时间",format_1);
    xlsx.write("F2","手机号码",format_1);
    xlsx.write("G2","7日内途径城市",format_1);
    xlsx.write("H2","行程码状态",format_1);
    xlsx.write("I2","行程码提交时间",format_1);
}

void MainWindow::write_data_tour(int row, QString phonenumber, QString place, QString tour_status, QDate tour_time, QXlsx::Format format)
{
    xlsx.write("F"+QString::number(row),phonenumber,format);  //手机号码
    xlsx.write("G"+QString::number(row),place,format);  //7日内途径
    xlsx.write("H"+QString::number(row),tour_status,format);  //行程码状态
    xlsx.write("I"+QString::number(row),tour_time.toString("yyyy-MM-dd"),format);  //行程码提交时间
}

void MainWindow::write_data_health(int row, QString name, QString number, QString health_status,QDate health_time,QXlsx::Format format)
{
    xlsx.write("A"+QString::number(row),name,format);  //姓名
    xlsx.write("B"+QString::number(row),number,format);  //身份证号
    xlsx.write("C"+QString::number(row),health_status,format);  //健康码状态
    xlsx.write("D"+QString::number(row),health_time.toString("yyyy-MM-dd"),format);  //健康码提交时间
}


void MainWindow::on_cleardata_clicked()    //清除数据按钮
{
    m_url.clear();
    while (ui->listWidget->count() > 0)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item;
    }
}

void MainWindow::on_actionaddperson_triggered()  //添加人员按钮
{
    if(!db.isOpen()){
        QMessageBox::information(this,"警告","数据库未连接");
        return;
    }
    QSqlQuery table = db.exec("select * from person");
    QString Qurl = QFileDialog::getOpenFileName(this,"打开文件","./","*.txt");
    //读取文件
    QFile file(Qurl);
    //打开文件，只读
    if(file.open(QIODevice::ReadOnly|QIODevice::Text)){
        QString line;
        while (!file.atEnd()) {
            line = file.readLine();
            //插入数据
            table.prepare("insert into person(name,phonenumber) values(?,?);");
            table.addBindValue(line);
            table.addBindValue(" ");
            table.exec();
            //            qDebug()<<line;
        }
    }
    file.close();
}

void MainWindow::on_pushButton_2_clicked()   //上传数据按钮
{
    QString dirpath = QFileDialog::getExistingDirectory(this, "选择目录", "./", QFileDialog::ShowDirsOnly);
    QString filepath;
    dirpath += '/';
    //    qDebug()<<dirpath;
    QDir dir(dirpath);
    QStringList nameFilters;
    nameFilters << "*.jpg" << "*.png";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable,QDir::NoSort);
    for(int i =0;i < files.size();i++){
        filepath =dirpath+files[i];
        //        qDebug()<<filepath;
        m_url.push_back(filepath);
        //设置QListWidget的显示模式
        ui->listWidget->setViewMode(QListView::IconMode);
        //设置QListWidget中单元格的图片大小
        ui->listWidget->setIconSize(QSize(100,100));
        //设置QListWidget中单元项的间距
        ui->listWidget->setSpacing(10);
        //设置自动适应布局调整（Adject适应，Fixed不适应，默认不适应）
        ui->listWidget->setResizeMode(QListWidget::Adjust);
        //设置不能移动
        //      ui->listWidget->setMovement(QListWidget::Static);
        //定义QListWidgetItem对象
        QListWidgetItem *imageItem = new QListWidgetItem;
        //为单元项设置属性
        imageItem->setIcon(QIcon(filepath));
        QStringList namelist = filepath.split('/');
        //设置每个图片的名字
        imageItem->setText(namelist[namelist.size()-1]);
        //重新设置单元项图片的宽度和高度
        imageItem->setSizeHint(QSize(100,120));
        //将图片项添加到QListWidget中
        ui->listWidget->addItem(imageItem);
    }
}

void MainWindow::on_actionsetdate_triggered()   //设置日期按钮
{
    dateset *dateEdit = new dateset;
    dateEdit->show();
}

void MainWindow::on_actionconnectdatabase_triggered()
{
    //连接数据库
    linkdatabase();
}

void MainWindow::on_get_excel_clicked()
{
    int row_init = 3; //数据部分从第3行开始写
    if(!db.isOpen()){
        QMessageBox::information(this,"错误","请先连接数据库");
        return;
    }
    open_Excel();
    //
    QSqlQuery query_health;  //健康码信息列表
    QSqlQuery query_tour;  //行程码信息列表
    query_health.exec("select * from health_code;");//从health_code中得到数据
    query_tour.exec("select * from tour_code;");  //从tour_code中得到数据
    while(query_health.next()){ //一行一行遍历
    write_data_health(row_init,query_health.value(0).toString(),    // 姓名
               query_health.value(1).toString(),    //身份证号
               query_health.value(3).toString(),    //健康码状态
               QDate(query_health.value(2).toString().split('.').at(0).toInt(),query_health.value(2).toString().split('.').at(1).toInt(),query_health.value(2).toString().split('.').at(2).toInt()),    //健康码日期
               format_1
               );
    row_init++;
    }
    row_init = 3;
    while(query_tour.next()){//一行一行遍历
    write_data_tour(row_init,query_tour.value(0).toString(),
                    query_tour.value(2).toString(),
                    query_tour.value(3).toString(),
                    QDate(query_tour.value(1).toString().split('.').at(0).toInt(),query_tour.value(1).toString().split('.').at(1).toInt(),query_tour.value(1).toString().split('.').at(2).toInt()),    //健康码日期
                    format_1
                    );
    row_init++;
    }
//    write_data(3,"卢校虎","410883199809023058","绿码","绿码",QDate(2022,3,10),QDate(2022,3,11),"焦作市",format_1);
    xlsx.saveAs(filename);
    if(filename!="")
        QMessageBox::information(this,"通知","导出成功");
}
