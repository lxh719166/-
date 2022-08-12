#include "dragfile.h"
#include <QDebug>

DragFile::DragFile(QWidget *parent) : QListWidget(parent)
{
    //设置可以接收拖拽事件
    this->setAcceptDrops(1);
}

void DragFile::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug()<<"鼠标拖拽进入了";
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction(); //事件数据中存在路径，方向事件
    }
    else
    {
        event->ignore();
    }
}

void DragFile::dragMoveEvent(QDragMoveEvent *event)
{
    //qDebug()<<"鼠标拖拽移动了";
}

//拖动文件到窗口释放
void DragFile::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        //得到拖拽的文件的url列表
        QList<QUrl> urls = mimeData->urls();
        emit sendUrlList(urls);
        //设置QListWidget的显示模式
        this->setViewMode(QListView::IconMode);
        //设置QListWidget中单元格的图片大小
        this->setIconSize(QSize(100,100));
        //设置QListWidget中单元项的间距
        this->setSpacing(10);
        //设置自动适应布局调整（Adject适应，Fixed不适应，默认不适应）
        this->setResizeMode(QListWidget::Adjust);
        //设置不能移动
//        this->setMovement(QListWidget::Static);
        for(int i = 0;i<urls.size();i++){
            //定义QListWidgetItem对象
            QListWidgetItem *imageItem = new QListWidgetItem;
            //为单元项设置属性
            imageItem->setIcon(QIcon(urls.at(i).toLocalFile()));
            QStringList namelist = urls.at(i).toLocalFile().split('/');
            //设置每个图片的名字
            imageItem->setText(namelist[namelist.size()-1]);
            //重新设置单元项图片的宽度和高度
            imageItem->setSizeHint(QSize(100,120));
            //将图片项添加到QListWidget中
            this->addItem(imageItem);
        }
    }
}
