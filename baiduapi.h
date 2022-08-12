#ifndef BAIDUAPI_H
#define BAIDUAPI_H

#include <QImage>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QDebug>
#include <QMap>
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
    static QString imageToBase64(QImage image);
    //图像检测
    void Image_Detection(QString base64data);
    //图像检测结果接收
    void Detection_result(QNetworkReply * pReply);
    //行程码OCR识别
    void xingcheng_OCR(QString base64data);
    //行程码OCR结果接收
    void xingcheng_OCR_result(QNetworkReply * pReply);
    //健康码OCR识别
    void jiankang_OCR(QString base64dat);
    //健康码OCR识别结果接收
    void jiankang_OCR_result(QNetworkReply * pReply);
    //对class_result的处理
    void set_class_result(QString str);
    QString get_class_result();


signals:
    void send_classific_result(QString jiankang,double value_1,QString xingcheng,double value_2,QString qita,double value_3);
    void send_detection_result_eye(int result);
    void next_pic();
    //网络调试
    void send_err(QString err);
private:
//    QString imageDatas;  //储存变量
    QString AssessToken = "24.684b9d991f45b799a536ff44daf63e84.2592000.1660444048.282335-26502137";
    QNetworkAccessManager *to_recoginze;  //图像分类信息变量
    QNetworkAccessManager *detection_info; //健康码眼睛物体检测信息变量
    QNetworkAccessManager *xingcheng_OCR_info;  //行程码OCR识别
    QString m_class_result;
};

#endif // BAIDUAPI_H
