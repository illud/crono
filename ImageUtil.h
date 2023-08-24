#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <QObject>
#include <QImage>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

class ImageUtil : public QObject
{
    Q_OBJECT
public:
    ImageUtil(QObject *parent = nullptr) : QObject(parent),
                                           m_Manager(new QNetworkAccessManager(this))
    {
        connect(m_Manager, &QNetworkAccessManager::finished, this, &ImageUtil::onFinished);
    }

    const QImage &image() const { return m_Image; }
    Q_INVOKABLE void loadFromUrl(const QUrl &url)
    {
        QNetworkRequest request(url);
        m_Manager->get(request);
    }

signals:
    void loaded();

protected slots:
    void onFinished(QNetworkReply *reply)
    {
        QByteArray data = reply->readAll();
        m_Image.loadFromData(data);
        emit loaded();
    }

protected:
    QImage m_Image;
    QNetworkAccessManager *m_Manager;
};

#endif // IMAGEUTIL_H
