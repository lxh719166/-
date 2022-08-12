#include "baiduapi.h"
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "baiduapi.h"

int BaiDuAPI::picindex = 0;

BaiDuAPI::BaiDuAPI(QObject *parent) : QObject(parent)
{

}

//向API接口发送请求
void BaiDuAPI::on_recognize(QString Base64data)
{
    if(Base64data.isEmpty()){  //如果图片转码结果为空，则不能识别
        qDebug() << "image base64 is empty";
        return;
    }
    QUrl url;
    //设置API接口地址
    url.setUrl("https://aip.baidubce.com/rpc/2.0/ai_custom/v1/classification/codeclassific?access_token="+AssessToken);
    QNetworkRequest request;
    request.setUrl(url);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("Content-Type:application/json;charset=UTF-8"));
    //制作json消息
    QJsonObject post_data;
    QJsonDocument document;
    post_data.insert("image",Base64data);
    post_data.insert("image_type","BASE64");
    document.setObject(post_data);
    QByteArray post_param = document.toJson(QJsonDocument::Compact);
    //发送消息
    to_recoginze = new QNetworkAccessManager(this);
    connect(to_recoginze,&QNetworkAccessManager::finished, this, &BaiDuAPI::recognizeResult);
    to_recoginze->post(request,post_param);
}


//处理返回的结果
void BaiDuAPI::recognizeResult(QNetworkReply *pReply)
{
    this->picindex++;
    QByteArray replaystring = pReply->readAll();
//  qDebug()<<"reganizeResult()"+replaystring;
    QNetworkReply::NetworkError err = pReply->error();
//    qDebug()<<"网络错误："<<pReply->errorString();
//    emit send_err(pReply->errorString());
    if(err != QNetworkReply::NoError){
        qDebug()<<"Failed:"<<replaystring;
    }else{
        //获取内容
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replaystring,&jsonErr);
        qDebug()<<jsonDoc.object();
        double xingcheng_code = 0,jiankang_code = 0,Others = 0;
        if(jsonDoc.isObject()){
            QJsonObject jsonObj = jsonDoc.object();
            QJsonValue result = jsonObj.value("results");
            if(result.isArray()){
                QJsonArray resultArray = result.toArray();//得到整个结果json串，再分成数组
                for(int i = 0;i<resultArray.count();i++){
                    QJsonObject temp = resultArray[i].toObject();
                   // qDebug()<<temp;    //显示现在处理的是哪个json串
                    if(temp.value("name") == "健康码"){
                        jiankang_code = temp.value("score").toDouble();
                    }
                    else if(temp.value("name") == "[default]"){
                        Others = temp.value("score").toDouble();
                    }
                    else{
                        xingcheng_code =temp.value("score").toDouble();
                    }
            }
        }
            emit next_pic();
            emit send_classific_result("健康码",jiankang_code,"行程码",xingcheng_code,"其他",Others);
    }
}}


//图片转码Base64
QString BaiDuAPI::imageToBase64(QImage image)
{
    QByteArray imageData;
    QBuffer buff(&imageData);
    image.save(&buff,"jpg");
    return imageData.toBase64();
}


void BaiDuAPI::Image_Detection(QString base64data)
{

    if(base64data.isEmpty()){  //如果图片转码结果为空，则不能识别
        qDebug() << "image base64 is empty";
        return;
    }
    QUrl url;
    //设置API接口地址
//    qDebug() << "准备数据";
    url.setUrl("https://aip.baidubce.com/rpc/2.0/ai_custom/v1/detection/eyedec?access_token="+AssessToken);
    QNetworkRequest request;
    request.setUrl(url);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("Content-Type:application/json;charset=UTF-8"));

    //制作json消息
    QJsonObject post_data;
    QJsonDocument document;
    post_data.insert("image",base64data);
    document.setObject(post_data);
    QByteArray post_param = document.toJson(QJsonDocument::Compact);

    //发送消息
    detection_info = new QNetworkAccessManager(this);
    connect(detection_info,&QNetworkAccessManager::finished, this, &BaiDuAPI::Detection_result);
    detection_info->post(request,post_param);
}

void BaiDuAPI::Detection_result(QNetworkReply *pReply)
{
    QByteArray replaystring = pReply->readAll();
    QNetworkReply::NetworkError err = pReply->error();
    int Detection_eye;
    if(err != QNetworkReply::NoError){
        qDebug()<<"Failed:"<<replaystring;
    }else{
        //获取内容
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replaystring,&jsonErr);
        //qDebug()<<jsonDoc.object();
        if(jsonDoc.isObject()){
            QJsonObject jsonObj = jsonDoc.object();
            QJsonValue result = jsonObj.value("results");
            if(result.isArray()){
                QJsonArray resultArray = result.toArray();//得到整个结果json串，再分成数组
                QJsonObject temp = resultArray[0].toObject();  //得到,"name":"关"
//                qDebug()<<temp.value("name");
                if(temp.value("name")=="关") Detection_eye=0;
                else if(temp.value("name")=="开") Detection_eye=1;
                emit this->send_detection_result_eye(Detection_eye);
;            }
        }
    }

}

void BaiDuAPI::xingcheng_OCR(QString base64data)
{
    if(base64data.isEmpty()){  //如果图片转码结果为空，则不能识别
//        qDebug() << "image base64 is empty";
        return;
    }
    QUrl url;
    //设置API接口地址
    url.setUrl("https://aip.baidubce.com/rpc/2.0/ai_custom/v1/detection/eyedec?access_token="+AssessToken);
    QNetworkRequest request;
    request.setUrl(url);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("Content-Type:application/json;charset=UTF-8"));

    //制作json消息
    QJsonObject post_data;
    QJsonDocument document;
    post_data.insert("image",base64data);
    post_data.insert("modelld","smgr0");
    post_data.insert("detect_direction",true);
    document.setObject(post_data);
    QByteArray post_param = document.toJson(QJsonDocument::Compact);

    //发送消息
    xingcheng_OCR_info = new QNetworkAccessManager(this);
    connect(xingcheng_OCR_info,&QNetworkAccessManager::finished, this, &BaiDuAPI::xingcheng_OCR_result);
    xingcheng_OCR_info->post(request,post_param);
}

void BaiDuAPI::xingcheng_OCR_result(QNetworkReply *pReply)
{
    QByteArray replaystring = pReply->readAll();
    QNetworkReply::NetworkError err = pReply->error();
    if(err != QNetworkReply::NoError){
//        qDebug()<<"Failed:"<<replaystring;
    }else{
        //获取内容
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replaystring,&jsonErr);
//        qDebug()<<jsonDoc.object();
    }
}

void BaiDuAPI::jiankang_OCR(QString base64dat)
{

}

void BaiDuAPI::jiankang_OCR_result(QNetworkReply *pReply)
{

}

void BaiDuAPI::set_class_result(QString str)
{
    this->m_class_result = str;
}

QString BaiDuAPI::get_class_result()
{
    return m_class_result;
}
