#include "baiduapi.h"
#include <QBuffer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextCodec>
#include <QFile>
#include <QMessageBox>
#include <QEventLoop>
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
            emit send_classific_result(jiankang_code,xingcheng_code,Others);
        }
    }}


//图片转码Base64
QByteArray BaiDuAPI::imageToBase64(QImage image)
{
    QByteArray imageData;
    QBuffer buff(&imageData);
    image.save(&buff,"PNG");
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
    int Detection_eye = 1;  //默认开启
    if(err != QNetworkReply::NoError){
        qDebug()<<"Failed:"<<replaystring;
    }else{
        //获取内容
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replaystring,&jsonErr);
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
            }
        }
    }

}

void BaiDuAPI::xingcheng_OCR(QString pic_url)
{
    QImage image(pic_url);
    QByteArray imageData;
    QByteArray base64;
    QBuffer buff(&imageData);
    buff.open(QIODevice::WriteOnly);
    image.save(&buff,"JPG");
    base64 = imageData.toBase64();
    //对图片进行urlencode
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QByteArray urlencode = codec->fromUnicode(base64).toPercentEncoding();
    //body
    QByteArray requestData = "image=" + urlencode;
    //答复的数据
    QByteArray replyData;
    QString m_url = "https://aip.baidubce.com/rest/2.0/ocr/v1/travel_card?access_token=24.5bdd9c5c9db0e87b96cc5aca0c9d00be.2592000.1663344632.282335-27060456";
    //头部信息
    QMap<QString,QString> header;
    //创建执行发送请求的对象
    QNetworkAccessManager manage;
    //请求的内容（包括url和头部信息）
    QNetworkRequest request;
    request.setUrl(m_url);
    //头部
    QMapIterator<QString,QString> iter(header);
    while(iter.hasNext()){
        iter.next();
        request.setRawHeader(iter.key().toLatin1(),iter.value().toLatin1());
    }
    //当reply收到数据的时候会产生结束信号，继而循环结束
    QNetworkReply* reply = manage.post(request,requestData);
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec(); //模拟阻塞状态，直到收到数据进行
    if(reply!=nullptr&&reply->error()==QNetworkReply::NoError){
        replyData = reply->readAll();
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData,&jsonErr);
        qDebug()<<jsonDoc.object();
        if(jsonDoc.isObject()){
            QJsonObject jsonObj = jsonDoc.object();
            QString phonenumber = jsonObj.value("result").toObject().value("手机号").toArray()[0].toObject().value("word").toArray()[0].toString();
            QString date = jsonObj.value("result").toObject().value("更新时间").toArray()[0].toObject().value("word").toArray()[0].toString().mid(0,10);
            QString place = jsonObj.value("result").toObject().value("途经地").toArray()[0].toObject().value("word").toArray()[0].toString();
            bool deg = jsonObj.value("result").toObject().value("风险性").toBool();
            emit send_tourOCR_result(phonenumber,date,place,deg);
        }
    }

}

void BaiDuAPI::jiankang_OCR(QString pic_url)
{
    //创建一个图片对象
    QImage image(pic_url);
    QByteArray imageData;
    QByteArray base64;
    QBuffer buff(&imageData);
    buff.open(QIODevice::WriteOnly);
    image.save(&buff,"JPG");
    base64 = imageData.toBase64();

    //对图片进行urlencode
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QByteArray urlencode = codec->fromUnicode(base64).toPercentEncoding();
    //body
    QByteArray requestData = "image=" + urlencode;
    //答复的数据
    QByteArray replyData;
    QString m_url = "https://aip.baidubce.com/rest/2.0/ocr/v1/general_basic?access_token=24.5bdd9c5c9db0e87b96cc5aca0c9d00be.2592000.1663344632.282335-27060456";
    //头部信息
    QMap<QString,QString> header;
    //创建执行发送请求的对象
    QNetworkAccessManager manage;
    //请求的内容（包括url和头部信息）
    QNetworkRequest request;
    request.setUrl(m_url);
    //头部
    QMapIterator<QString,QString> iter(header);
    while(iter.hasNext()){
        iter.next();
        request.setRawHeader(iter.key().toLatin1(),iter.value().toLatin1());
    }
    //当reply收到数据的时候会产生结束信号，继而循环结束
    QNetworkReply* reply = manage.post(request,requestData);
    QEventLoop loop;
    connect(reply,&QNetworkReply::finished,&loop,&QEventLoop::quit);
    loop.exec(); //模拟阻塞状态，直到收到数据进行

    QString name;
    QString number;
    QString date;
    QString status;

    if(reply!=nullptr&&reply->error()==QNetworkReply::NoError){
        replyData = reply->readAll();
        QJsonParseError jsonErr;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData,&jsonErr);
        if(jsonDoc.isObject()){
            QJsonObject jsonObj = jsonDoc.object();
//            qDebug()<<jsonObj;
            qDebug()<<jsonObj.value("words_result");

            for(int i =0;i<30;i++){
               if(jsonObj.value("words_result").toArray()[i].toObject().value("words").toString()=="成员管理"){
                   name = jsonObj.value("words_result").toArray()[i-1].toObject().value("words").toString();
                   number = jsonObj.value("words_result").toArray()[i+1].toObject().value("words").toString();
                   date = jsonObj.value("words_result").toArray()[i+2].toObject().value("words").toString();
                   i+=2;
               }
               if(jsonObj.value("words_result").toArray()[i].toObject().value("words").toString().contains("已接种",Qt::CaseSensitive)){
                   status = jsonObj.value("words_result").toArray()[i+1].toObject().value("words").toString();
                    break;
               }
            }
            qDebug()<<name<<number.mid(0,18)<<"2022."+date.mid(0,5).split("-")[0]+"."+date.mid(0,5).split("-")[1]<<status.mid(0,2);
            emit send_healthOCR_result(name,number.mid(0,18),"2022."+date.mid(0,5).split("-")[0]+"."+date.mid(0,5).split("-")[1],status.mid(0,2));
        }
    }
}

void BaiDuAPI::set_class_result(QString str)
{
    this->m_class_result = str;
}

QString BaiDuAPI::get_class_result()
{
    return m_class_result;
}
