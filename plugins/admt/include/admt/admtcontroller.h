#ifndef ADMTCONTROLLER_H
#define ADMTCONTROLLER_H

#include <iio.h>
#include <bitset>

#include <QObject>
#include <QString>

#include <iioutil/connectionprovider.h>

namespace scopy::admt {    
class ADMTController : public QObject
{
    Q_OBJECT
public:
    ADMTController(QString uri, QObject *parent = nullptr);
    ~ADMTController();

    enum Channel
    {
        ROTATION,
        ANGLE,
        COUNT,
        TEMPERATURE,
        CHANNEL_COUNT
    };

    const char* ChannelIds[CHANNEL_COUNT] = {"rot", "angl", "count", "temp"};

    const char* getChannelId(Channel channel);

    void connectADMT();
    void disconnectADMT();
    QString getChannelValue();
    int getChannelIndex(const char *channelName);
    double getChannelValue(const char *channelName, int bufferSize);
private:
    iio_context *m_iioCtx;
    iio_buffer *m_iioBuffer;
    Connection *m_conn;
    QString uri;
};
} // namespace scopy::admt

#endif // ADMTCONTROLLER_H