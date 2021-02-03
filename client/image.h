#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QUuid>
#include <QPixmap>

class Image : public QObject
{
    Q_OBJECT
public:
    Image(QUuid id, QObject* parent);
    ~Image();

    QImage image();
    QPixmap pixmap();
signals:
    void loaded();
private:
    QUuid m_id;
    QPixmap m_pixmap;
};

#endif // IMAGE_H
