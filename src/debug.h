#ifndef DEBUG_H
#define DEBUG_H

#include <QObject>
#include <iio.h>
#include <QDebug>
#include <QVector>

namespace adiscope {

    class Debug : public QObject
    {
        Q_OBJECT

    public:
        explicit Debug(QObject *parent = nullptr);
        QStringList getDeviceList(void);
        QStringList getChannelList(void);
        QStringList getAttributeList(void);
        QStringList getFileName(void);
        QVector<QString> getAttributeVector(void);
        struct iio_context * getIioContext(void);
        void setIioContext(struct iio_context *ctx);
        ~Debug();

        QString readAttribute(QString devName, QString channel, QString attribute);
        void writeAttribute(QString devName, QString channel, QString attribute, QString value);
        QStringList getAvailableValues(QString devName, QString channel, QString attribute);
        QString getAttributeValue(QString devName, QString channel, QString attribute);
        void setAttributeValue(QString devName, QString channel, QString attribute, QString value);

    Q_SIGNALS:
        void channelsChanged(const QStringList channelList);

    public Q_SLOTS:
        void scanDevices(void);
        void scanChannels(QString devName);
        void scanChannelAttributes(QString devName, QString channel);

    private:
        struct iio_context *ctx;
        QStringList deviceList;
        QStringList channelList;
        QStringList attributeList;
        QStringList filename;
        bool connected = false;
        QVector<QString> attributeAvailable;
    };
}

#endif // DEBUG_H
