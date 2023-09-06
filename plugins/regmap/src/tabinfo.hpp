#ifndef TABINFO_HPP
#define TABINFO_HPP

#include <QObject>
#include <QString>
#include <iio.h>

class TabInfo : public QObject
{
    Q_OBJECT
public:
    explicit TabInfo(struct iio_device *dev, QString deviceName, QString xmlPath,QObject *parent = nullptr);

    iio_device *getDev() const;

    QString getDeviceName() const;

    QString getXmlPath() const;

signals:

private:
    struct iio_device *dev;
    QString deviceName;
    QString xmlPath;
};

#endif // TABINFO_HPP
