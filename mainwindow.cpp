#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QBuffer>
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
#include <QSqlRecord>
#include <QTableWidget>
#include <QTableWidget>
#include <QTextCodec>                                //listresult
#include <xlsxformat.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setStyleSheet("#pushButton_2:pressed,#startrecon:pressed,#cleardata:pressed,#show_user:pressed,#show_result:pressed,#get_excel:pressed,#show_analyze:pressed{border-style:solid;border-radius: 5px;width: 100px;height: 40px;color:#EBEBD3;background-color:#38523d} "
                        "#pushButton_2,#startrecon,#cleardata,#show_user,#show_result,#get_excel,#show_analyze{border-style:solid;border-radius: 5px;width: 100px;height: 40px;color:#EBEBD3;background-color:#708B75}");

    this->setFixedSize(1126,750);
    //设置标题文字
    QPixmap pic_title;
    pic_title.load(":/res/主界面标题.png");
    pic_title.scaled(ui->labelTitle->size(),Qt::KeepAspectRatio);
    ui->labelTitle->setAlignment(Qt::AlignCenter);
    ui->labelTitle->setPixmap(pic_title);
    //设置时间默认为当前时间
    m_start = QDate::currentDate();
    m_finish = QDate::currentDate();
    //设置table_user
    ui->table_user->setRowCount(50);
    ui->table_user->setColumnCount(4);
    QStringList header_user;
    header_user<<"姓名"<<"手机号"<<""<<"";   //表头
    ui->table_user->setHorizontalHeaderLabels(header_user);
    //设置table_result
    ui->table_result->setRowCount(50);
    ui->table_result->setColumnCount(8);
    QStringList header_result;
    header_result<<"姓名"<<"身份证号"<<"手机号"<<"健康码状态"<<"行程码状态"<<"7日途径"<<"健康码提交时间"<<"行程码提交时间";
    ui->table_result->setHorizontalHeaderLabels(header_result);
    //设置table_analyze
    QStringList header_analyze;
    ui->table_analyze->setRowCount(50);
    ui->table_analyze->setColumnCount(7);
    header_analyze<<"健康码未交名单"<<"行程码未交名单"<<"健康码时间异常名单"<<"行程码时间异常名单"<<"健康码状态异常名单"<<"行程码状态异常名单"<<"健康码信息遮挡文件";
    ui->table_analyze->setHorizontalHeaderLabels(header_analyze);
    //设置初始显示的页面
    ui->stackwidget->setCurrentIndex(0);
    //设置大标题
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
    dateEdit = new dateset;
    connect(dateEdit,&dateset::send_date,this,&MainWindow::getdate);

    //得到切换的用户名
    changeusername = new ChangeUser;
    connect(changeusername,&ChangeUser::send_change_user,this,&MainWindow::getchange_username);

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
        BaiDuAPI::picindex = 0;
        //        status_label_1->setText("正在识别中"+QString::number(baidu->picindex)+"/"+QString::number(this->m_url.size()));
        status_label_1->setText("正在识别中");
        //        if(!QSslSocket::supportsSsl()){
        //            QMessageBox::information(this,"错误","系统不支持openssl");
        //            return;
        //        }
        progressDialog = new QProgressDialog(this);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setMinimumDuration(1);
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

void MainWindow::receiveLogin(QString username)
{
    this->show();
    m_username = username;
}

void MainWindow::getchange_username(QString username)
{
    this->m_username = username;
}


void MainWindow::getdate(QDate start, QDate finish)
{
    this->m_start = start;
    this->m_finish = finish;
}

void MainWindow::pictureclass(QUrl url)
{
    //连接数据库
    if(!db.isOpen())
        linkdatabase();
    QByteArray base64 =BaiDuAPI::imageToBase64(QImage(url.toString()));
    QByteArray test = "";
    BaiDuAPI *baidu = new BaiDuAPI;  //创建百度API对象
    qDebug()<<"当前任务："<<BaiDuAPI::picindex+1<<"/"<<m_url.size();

    //接收传过来的识别数据
    connect(baidu,&BaiDuAPI::send_healthOCR_result,[=](QString name,QString number,QString date,QString status){
        QString flag;
        if(status=="绿码") flag = "正常";
        else flag = "异常";
        QSqlQuery insert_temp;
        insert_temp.prepare("insert into "+m_username+"_health(name,number,date,status) VALUES(?,?,?,?);");
        insert_temp.addBindValue(name);
        insert_temp.addBindValue(number);
        insert_temp.addBindValue(date);
        insert_temp.addBindValue(flag);
        bool insertflag = insert_temp.exec();
        qDebug()<<"健康码插入状态："<<insertflag;
    });
    connect(baidu,&BaiDuAPI::send_classific_result,[=](double value_1,double value_2,double value_3){
        if(value_1 > value_2 && value_1>value_3){  //健康码的情况
            //显示分类结果
//            qDebug()<<jiankang;
            //对健康码上的小眼睛进行检测，如果没开，信息入库，如果开了，进行OCR
            health_eye_detection(base64,baidu);
            //显示小眼睛识别结果
            connect(baidu,&BaiDuAPI::send_detection_result_eye,[=](int result){
                if(result){
                    //健康码的OCR识别
                    baidu->jiankang_OCR(url.toString());
                    //处理返回的结果

                } else{  //眼睛关闭信息入库
                    QSqlQuery eye;  //行程码信息列表
                    eye.prepare("insert into "+m_username+"_eyeDetection(Filename,status) values(?,?);");
                    eye.addBindValue(url.fileName());
                    eye.addBindValue("眼睛关闭");
                    eye.exec();
                    qDebug()<<"眼睛关闭";
                }
                //判断识别识别完成
                if(baidu->picindex<m_url.size())
                    pictureclass(m_url.at(baidu->picindex));
                else{
                    progressDialog->setValue(m_url.size());
                    status_label_1->setText("识别完成");
                    QMessageBox::information(this,"提醒","识别完成");
                }
            });
        }
        else if(value_2>value_1&&value_2>value_3){ //行程码的情况
            //处理传过来的数据，信息入库
            connect(baidu,&BaiDuAPI::send_tourOCR_result,[=](QString phonenumber,QString date,QString place,bool deg){
                QString status;
                if(deg) status = "异常";
                else status = "正常";
                QSqlQuery insert_temp;
                insert_temp.prepare("insert into "+m_username+"_tour(phonenumber,date,place,status) VALUES(?,?,?,?);");
                insert_temp.addBindValue(phonenumber);
                insert_temp.addBindValue(date);
                insert_temp.addBindValue(place);
                insert_temp.addBindValue(status);
                bool flag = insert_temp.exec();
                qDebug()<<"行程码插入状态:"<<flag;
            });
            baidu->xingcheng_OCR(url.toString());
            //判断识别识别完成
            if(baidu->picindex<m_url.size())
                pictureclass(m_url.at(baidu->picindex));
            else{
                progressDialog->setValue(m_url.size());
                status_label_1->setText("识别完成");
                QMessageBox::information(this,"提醒","识别完成");
            }
        }
        else if(value_3>value_1&&value_3>value_2){ //其他的情况
            qDebug()<<"其他";
            qDebug()<<"当前图片index："<<&BaiDuAPI::picindex;
            if(baidu->picindex<m_url.size())
                pictureclass(m_url.at(baidu->picindex));
            else{
                progressDialog->setValue(m_url.size());
                status_label_1->setText("识别完成");
                QMessageBox::information(this,"提醒","识别完成");
            }
        }
    });
    //设置进度条进度
    progressDialog->setValue(BaiDuAPI::picindex);
    baidu->on_recognize(base64);  //开始识别

}

void MainWindow::health_eye_detection(QString base64,BaiDuAPI *baidu)
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
    db.open();
    if (db.isOpen()){
        status_label->setText("数据库已连接");
    }
    else {
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
    //添加工作表
    xlsx.addSheet("识别结果");
    xlsx.addSheet("总体情况");
    xlsx.addSheet("分析结果");
    //设置总体情况工作表 表头部分
    if(!xlsx.selectSheet("识别结果"))
    {
        xlsx.selectSheet("识别结果");
    }
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
    xlsx.setRowHeight(1,30);
    xlsx.setRowHeight(2,25);
    xlsx.setColumnWidth(1,9,20);
    xlsx.setColumnWidth(5,8);
    //添加数据

    //设置识别结果工作表 表头部分
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

    //设置总体情况工作表 表头部分
    if(!xlsx.selectSheet("总体情况"))
    {
        xlsx.selectSheet("总体情况");
    }
    xlsx.setColumnWidth(1,8,19);   //设置1-8列，列宽为19
    xlsx.setRowHeight(1,100,20);   //设置1-100行，行高为20
    //设置表头
    xlsx.write("A1","姓名",format_1);
    xlsx.write("B1","身份证号",format_1);
    xlsx.write("C1","手机号",format_1);
    xlsx.write("D1","健康码状态",format_1);
    xlsx.write("E1","行程码状态",format_1);
    xlsx.write("F1","7日内途径",format_1);
    xlsx.write("G1","健康码提交时间",format_1);
    xlsx.write("H1","行程码提交时间",format_1);
    //设置分析结果工作表 表头部分
    if(!xlsx.selectSheet("分析结果"))
    {
        xlsx.selectSheet("分析结果");
    }
    xlsx.setColumnWidth(1,7,20);   //设置1-7列，列宽为19
    xlsx.setRowHeight(1,100,20);   //设置1-100行，行高为20
    //设置表头
    xlsx.write("A1","健康码未交名单",format_1);
    xlsx.write("B1","行程码未交名单",format_1);
    xlsx.write("C1","健康码时间异常名单",format_1);
    xlsx.write("D1","行程码时间异常名单",format_1);
    xlsx.write("E1","健康码状态异常名单",format_1);
    xlsx.write("F1","行程码状态异常名单",format_1);
    xlsx.write("G1","健康码信息遮挡文件",format_1);
}

void MainWindow::write_data_tour(int row, QString phonenumber, QString place, QString tour_status, QDate tour_time, QXlsx::Format format)
{
    if(!xlsx.selectSheet("识别结果"))
    {
        xlsx.selectSheet("识别结果");
    }
    xlsx.write("F"+QString::number(row),phonenumber,format);  //手机号码
    xlsx.write("G"+QString::number(row),place,format);  //7日内途径
    xlsx.write("H"+QString::number(row),tour_status,format);  //行程码状态
    xlsx.write("I"+QString::number(row),tour_time.toString("yyyy.MM.dd"),format);  //行程码提交时间
}

void MainWindow::write_data_health(int row, QString name, QString number, QString health_status,QDate health_time,QXlsx::Format format)
{
    if(!xlsx.selectSheet("识别结果"))
    {
        xlsx.selectSheet("识别结果");
    }
    xlsx.write("A"+QString::number(row),name,format);  //姓名
    xlsx.write("B"+QString::number(row),number,format);  //身份证号
    xlsx.write("C"+QString::number(row),health_status,format);  //健康码状态
    xlsx.write("D"+QString::number(row),health_time.toString("yyyy-MM-dd"),format);  //健康码提交时间
}


void MainWindow::on_cleardata_clicked()    //清除数据按钮
{
    m_url.clear();
    BaiDuAPI::picindex = 0;
    while (ui->listWidget->count() > 0)
    {
        QListWidgetItem *item = ui->listWidget->takeItem(0);
        delete item;
    }
}

void MainWindow::on_actionaddperson_triggered()  //添加人员按钮
{
    //连接数据库
    if(!db.isOpen())
        linkdatabase();
    bool flag = 0;
    QSqlQuery table = db.exec("select * from person");
    QString Qurl = QFileDialog::getOpenFileName(this,"打开文件","./","*.xlsx x.xls");
    //读取文件
    QXlsx::Document xlsx(Qurl);
    QXlsx::CellRange range = xlsx.dimension();
    //插入数据
    qDebug()<<"行数"<<range.rowCount();
    qDebug()<<"列数"<<range.columnCount();
    for(int i=1;i <= range.rowCount();i++){
        table.prepare("insert into "+m_username+"_person(name,phonenumber) values(?,?);");
        if(xlsx.cellAt(i,1)!=NULL)
            table.addBindValue(xlsx.cellAt(i,1)->value().toString());
        else
            table.addBindValue(" ");
        if(xlsx.cellAt(i,2)!=NULL)
            table.addBindValue(xlsx.cellAt(i,2)->value().toString());
        else
            table.addBindValue("");
       flag = table.exec();
    }
    if(flag)
        QMessageBox::information(this,"通知","人员信息添加成功");
}

void MainWindow::on_pushButton_2_clicked()   //上传数据按钮
{
    QString dirpath = QFileDialog::getExistingDirectory(this, "选择目录", "./", QFileDialog::ShowDirsOnly);
    QString filepath = "";
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
        ui->listWidget->setIconSize(QSize(80,80));
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
        imageItem->setSizeHint(QSize(80,100));
        //将图片项添加到QListWidget中
        ui->listWidget->addItem(imageItem);
    }
}

void MainWindow::on_actionsetdate_triggered()   //设置日期按钮
{
    dateEdit->show();
}


void MainWindow::on_get_excel_clicked()  //导出Excel表
{
    //填写识别结果工作表
    int row_init = 3; //数据部分从第3行开始写
    if(!db.isOpen()){
        QMessageBox::information(this,"错误","请先连接数据库");
        return;
    }
    open_Excel();
    QSqlQuery query_health;  //健康码信息列表
    QSqlQuery query_tour;  //行程码信息列表
    query_health.exec("select * from "+m_username+"_health;");//从health_code中得到数据
    query_tour.exec("select * from "+m_username+"_tour;");  //从tour_code中得到数据
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
    //填写总体情况工作表
    if(!xlsx.selectSheet("总体情况"))
    {
        xlsx.selectSheet("总体情况");
    }
    QSqlQuery query_all;  //总体情况列表
    query_all.exec("Select a.name as 姓名,b.number as 身份证号,c.phonenumber as 手机号,b.status as 健康码状态,c.status as 行程码状态,c.place as 7日途径,b.date as 健康码提交时间,c.date as 行程码提交时间 from "+m_username+"_person a left join "+m_username+"_health b on a.name=b.name left join "+m_username+"_tour c on RIGHT(a.phonenumber,4)=RIGHT(c.phonenumber,4);");
    int row = 2;
    while(query_all.next()){ //一行一行遍历
       xlsx.write("A"+QString::number(row),query_all.value(0).toString(),format_1);  //姓名
       xlsx.write("B"+QString::number(row),query_all.value(1).toString(),format_1);  //身份证号
       xlsx.write("C"+QString::number(row),query_all.value(2).toString(),format_1);  //手机号
       xlsx.write("D"+QString::number(row),query_all.value(3).toString(),format_1);  //健康码状态
       xlsx.write("E"+QString::number(row),query_all.value(4).toString(),format_1);  //行程码状态
       xlsx.write("F"+QString::number(row),query_all.value(5).toString(),format_1);  //7日内途径
       xlsx.write("G"+QString::number(row),query_all.value(6).toString(),format_1);  //健康码提交时间
       xlsx.write("H"+QString::number(row),query_all.value(7).toString(),format_1);  //行程码提交时间
       row++;
    }


    //填写分析结果工作表
    if(!xlsx.selectSheet("分析结果"))
    {
        xlsx.selectSheet("分析结果");
    }
    //填行程码未交名单
    QSqlQuery query_temp;
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join " + m_username + "_tour as t on RIGHT(p.phonenumber,4)=RIGHT(t.phonenumber,4) where t.phonenumber is null;");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("B"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;
    //填健康码未交名单
    query_temp.exec("SELECT p.name as 健康码未交名单 FROM "+m_username+"_person as p left join "+m_username+"_health as h on p.name=h.name where h.name is null;");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("A"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;

    //填健康码时间异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_health as h on p.name=h.name where h.date != \""+m_start.toString("yyyy.MM.dd")+"\";");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("C"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;

    //填行程码时间异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_tour as h on RIGHT(p.phonenumber,4)=RIGHT(h.phonenumber,4) where h.date != \""+m_start.toString("yyyy.MM.dd")+"\";");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("D"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;

    //填健康码状态异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_health as h on p.name=h.name where h.status = \"异常\";");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("E"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;

    //填行程码状态异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_tour as h on RIGHT(p.phonenumber,4)=RIGHT(h.phonenumber,4) where h.status = \"异常\";");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("F"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;

    //填健康码信息遮挡
    query_temp.exec("select Filename from "+m_username+"_eyedetection where `status`=\"眼睛关闭\";");
    row = 2;
    while(query_temp.next()){ //一行一行遍历
       xlsx.write("G"+QString::number(row),query_temp.value(0).toString(),format_1);  //姓名
       row++;
    }
    row=2;

    xlsx.saveAs(filename);
    if(filename!="")
        QMessageBox::information(this,"通知","导出成功");
}

void MainWindow::on_show_user_clicked()  //展示人员表
{
    if(ui->stackwidget->currentIndex()!=0)
        ui->stackwidget->setCurrentIndex(0);
    ui->table_user->clearContents();
    if(!db.isOpen()){                      //未连接数据库情况
        linkdatabase();
    }
    //从数据库读入数据
    QSqlQuery query_person;  //行程码信息列表
    query_person.exec("select * from "+m_username+"_person;");//从health_code中得到数据
    int row = 0;
    while(query_person.next()){ //一行一行遍历
        ui->table_user->setItem(row,0,new QTableWidgetItem(query_person.value(0).toString()));
        ui->table_user->setItem(row,1,new QTableWidgetItem(query_person.value(1).toString()));
        row++;
    }
}

void MainWindow::on_show_result_clicked()  //展示结果表
{
    if(ui->stackwidget->currentIndex()!=1)
        ui->stackwidget->setCurrentIndex(1);
    ui->table_result->clearContents();
    if(!db.isOpen()){                      //未连接数据库情况
        linkdatabase();
    }
    //展示结果
    QSqlQuery query_all;  //行程码信息列表
    query_all.exec("Select a.name as 姓名,b.number as 身份证号,c.phonenumber as 手机号,b.status as 健康码状态,c.status as 行程码状态,c.place as 7日途径,b.date as 健康码提交时间,c.date as 行程码提交时间 from "+m_username+"_person a left join "+m_username+"_health b on a.name=b.name left join "+m_username+"_tour c on RIGHT(a.phonenumber,4)=RIGHT(c.phonenumber,4);");
    int row = 0;
    while(query_all.next()){    //
        ui->table_result->setItem(row,0,new QTableWidgetItem(query_all.value(0).toString()));
        ui->table_result->setItem(row,1,new QTableWidgetItem(query_all.value(1).toString()));
        ui->table_result->setItem(row,2,new QTableWidgetItem(query_all.value(2).toString()));
        ui->table_result->setItem(row,3,new QTableWidgetItem(query_all.value(3).toString()));
        ui->table_result->setItem(row,4,new QTableWidgetItem(query_all.value(4).toString()));
        ui->table_result->setItem(row,5,new QTableWidgetItem(query_all.value(5).toString()));
        ui->table_result->setItem(row,6,new QTableWidgetItem(query_all.value(6).toString()));
        ui->table_result->setItem(row,7,new QTableWidgetItem(query_all.value(7).toString()));
        row++;
    }
}

void MainWindow::on_actionchangeuser_triggered()
{
    changeusername->show();
}

void MainWindow::on_show_analyze_clicked()
{
    if(ui->stackwidget->currentIndex()!=2)
        ui->stackwidget->setCurrentIndex(2);
    ui->table_analyze->clearContents();
    //填数据
    //填行程码未交名单
    int row=0;
    QSqlQuery query_temp;
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join " + m_username + "_tour as t on RIGHT(p.phonenumber,4)=RIGHT(t.phonenumber,4) where t.phonenumber is null;");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,1,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row=0;
    //填健康码未交名单
    query_temp.exec("SELECT p.name as 健康码未交名单 FROM "+m_username+"_person as p left join "+m_username+"_health as h on p.name=h.name where h.name is null;");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,0,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row=0;

    //填健康码时间异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_health as h on p.name=h.name where h.date != \""+m_start.toString("yyyy.MM.dd")+"\";");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,2,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row=0;

    //填行程码时间异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_tour as h on RIGHT(p.phonenumber,4)=RIGHT(h.phonenumber,4) where h.date != \""+m_start.toString("yyyy.MM.dd")+"\";");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,3,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row=0;

    //填健康码状态异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_health as h on p.name=h.name where h.status = \"异常\";");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,4,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row = 0;

    //填行程码状态异常名单
    query_temp.exec("SELECT p.name FROM "+m_username+"_person as p left join "+m_username+"_tour as h on RIGHT(p.phonenumber,4)=RIGHT(h.phonenumber,4) where h.status = \"异常\";");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,5,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row = 0;

    //填健康码信息遮挡文件名
    query_temp.exec("select Filename from "+m_username+"_eyedetection where `status`=\"眼睛关闭\";");
    row = 0;
    while(query_temp.next()){ //一行一行遍历
       ui->table_analyze->setItem(row,6,new QTableWidgetItem(query_temp.value(0).toString())); //姓名
       row++;
    }
    row = 0;
}

void MainWindow::on_actiondele_person_triggered()
{
    if(!db.isOpen())
        linkdatabase();
    QMessageBox::StandardButton result;
    result = QMessageBox::question(this,"消息","是否确定清空全部人员？",QMessageBox::Yes|QMessageBox::No);
    if(result == QMessageBox::Yes){
        db.exec("DELETE from "+m_username+"_person;");
        QMessageBox::information(this,"通知","用户："+m_username+" 的人员信息已经全部清除！");
    }
    else
        return;
}

void MainWindow::on_actionclear_data_triggered()
{
    if(!db.isOpen())
        linkdatabase();
    QMessageBox::StandardButton result;
    result = QMessageBox::question(this,"消息","是否确定清空上一次识别结果？",QMessageBox::Yes|QMessageBox::No);
    if(result == QMessageBox::Yes){
        db.exec("DELETE from "+m_username+"_health;");
        db.exec("DELETE from "+m_username+"_tour;");
        db.exec("DELETE from "+m_username+"_eyedetection;");
        QMessageBox::information(this,"通知","上一次识别结果已经全部清除！");
    }
    else
        return;
}
