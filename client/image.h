#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QUuid>
#include <QPixmap>

namespace ThorQ {
class Image : public QObject
{
    Q_OBJECT
public:
    Image(QUuid id, QObject* parent);
    ~Image();

    QUuid imageID();
    QPixmap pixmap();
signals:
    void loaded();
public slots:
    void load();
private:
    QUuid m_id;
    QPixmap m_pixmap;
};
}

#endif // IMAGE_H
