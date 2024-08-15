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

    int HAR_MAG_1, HAR_MAG_2, HAR_MAG_3, HAR_MAG_8 ,HAR_PHASE_1 ,HAR_PHASE_2 ,HAR_PHASE_3 ,HAR_PHASE_8;

    vector<double> angle_errors_fft, angle_errors_fft_phase;

    enum Channel
    {
        ROTATION,
        ANGLE,
        COUNT,
        TEMPERATURE,
        CHANNEL_COUNT
    };

    enum Device
    {
        ADMT4000,
        TMC5240,
        DEVICE_COUNT
    };

    enum MotorAttribute
    {
        AMAX,
        ROTATE_VMAX,
        DMAX,
        DISABLE,
        TARGET_POS,
        CURRENT_POS,
        RAMP_MODE,
        MOTOR_ATTR_COUNT
    };

    enum MotorRampMode
    {
        POSITION,
        RAMP_MODE_1
    };

    const char* ChannelIds[CHANNEL_COUNT] = {"rot", "angl", "count", "temp"};
    const char* DeviceIds[DEVICE_COUNT] = {"admt4000", "tmc5240"};
    const char* MotorAttributes[MOTOR_ATTR_COUNT] = {"amax", "rotate_vmax", "dmax",
                                                     "disable", "target_pos", "current_pos",
                                                     "ramp_mode"};

    const char* getChannelId(Channel channel);
    const char* getDeviceId(Device device);
    const char* getMotorAttribute(MotorAttribute attribute);

    void connectADMT();
    void disconnectADMT();
    int getChannelIndex(const char *deviceName, const char *channelName);
    double getChannelValue(const char *deviceName, const char *channelName, int bufferSize = 1);
    int getDeviceAttributeValue(const char *deviceName, const char *attributeName, double *returnValue);
    int setDeviceAttributeValue(const char *deviceName, const char *attributeName, double writeValue);
    QString calibrate(vector<double> PANG, int cycles = 11, int samplesPerCycle = 256);
    int writeDeviceRegistry(const char *deviceName, uint32_t address, double value);
    int readDeviceRegistry(const char *deviceName, uint32_t address, double& readValue);
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