#ifndef ADMTCONTROLLER_H
#define ADMTCONTROLLER_H

#include "scopy-admt_export.h"

#include <iio.h>
#include <bitset>

#include <QObject>
#include <QString>

#include <iioutil/connectionprovider.h>

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <math.h>
#include <numeric>
#include <complex>
#include <iterator>

using namespace std;

namespace scopy::admt {    
class SCOPY_ADMT_EXPORT ADMTController : public QObject
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
    QString calibrate(vector<double> PANG);
private:
    iio_context *m_iioCtx;
    iio_buffer *m_iioBuffer;
    Connection *m_conn;
    QString uri;

    unsigned int bitReverse(unsigned int x, int log2n);
    template <typename Iter_T>
    void fft(Iter_T a, Iter_T b, int log2n);
    int linear_fit(vector<double> x, vector<double> y, double* slope, double* intercept);
    int calculate_angle_error(vector<double> angle_meas, vector<double>& angle_error_ret, double* max_angle_err);
};
} // namespace scopy::admt

#endif // ADMTCONTROLLER_H