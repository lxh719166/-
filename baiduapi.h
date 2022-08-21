#ifndef BAIDUAPI_H
#define BAIDUAPI_H

#include <QImage>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QMap>
#include <QTextCodec>
//#include "xlsxdocument.h"

class BaiDuAPI : public QObject
{
    Q_OBJECT
public:
    static int picindex;
    explicit BaiDuAPI(QObject *parent = nullptr);
    //图像分类
    void on_recognize(QString Base64data);
    //识别结果接收
    void recognizeResult(QNetworkReply * pReply);
    //图片转码
    static QByteArray imageToBase64(QImage image);
    //图像检测
    void Image_Detection(QString base64data);
    //图像检测结果接收
    void Detection_result(QNetworkReply * pReply);
    //行程码OCR识别
    void xingcheng_OCR(QString pic_url);
    //健康码OCR识别
    void jiankang_OCR(QString base64dat);
    //对class_result的处理
    void set_class_result(QString str);
    QString get_class_result();
signals:
    void send_classific_result(double value_1,double value_2,double value_3);
    void send_detection_result_eye(int result);
    void send_tourOCR_result(QString phonenumber,QString date,QString place,bool deg);
    void send_healthOCR_result(QString name,QString number,QString date,QString status);
    void next_pic();
    //网络调试
    void send_err(QString err);
private:
//    QString imageDatas;  //储存变量
    QString AssessToken = "24.a1ea61c2933ec95745d994e8b3d0bd40.2592000.1663118890.282335-26502137";
    QNetworkAccessManager *to_recoginze;  //图像分类信息变量
    QNetworkAccessManager *detection_info; //健康码眼睛物体检测信息变量
    QNetworkAccessManager *xingcheng_OCR_info;  //行程码OCR识别
    QString m_class_result;
};

#endif // BAIDUAPI_H
