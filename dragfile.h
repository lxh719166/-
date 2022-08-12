#ifndef DRAGFILE_H
#define DRAGFILE_H

#include <QWidget>
#include <QDropEvent>
#include <QLabel>
#include <QMimeData>
#include <QListWidget>
#include <QString>
class DragFile : public QListWidget
{
    Q_OBJECT
public:
    explicit DragFile(QWidget *parent = nullptr);
    //鼠标拖拽进入事件
    void dragEnterEvent(QDragEnterEvent*event) Q_DECL_OVERRIDE;
    //鼠标拖拽移动事件
    void dragMoveEvent(QDragMoveEvent *event) Q_DECL_OVERRIDE;
    //拖拽文件到窗口释放事件
    void dropEvent(QDropEvent *event) Q_DECL_OVERRIDE;
    //m_urls数据处理
    QList<QUrl> get_m_urls();
    void set_m_urls(QList<QUrl> urls);
private:
    QList<QUrl> m_urls;

signals:
    void sendUrlList(QList<QUrl> urls);
};

#endif // DRAGFILE_H
