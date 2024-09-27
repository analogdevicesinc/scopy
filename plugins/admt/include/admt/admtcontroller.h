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
#include <cstdint>

using namespace std;

namespace scopy::admt {    
class SCOPY_ADMT_EXPORT ADMTController : public QObject
{
    Q_OBJECT
public:
    ADMTController(QString uri, QObject *parent = nullptr);
    ~ADMTController();

    int HAR_MAG_1, HAR_MAG_2, HAR_MAG_3, HAR_MAG_8 ,HAR_PHASE_1 ,HAR_PHASE_2 ,HAR_PHASE_3 ,HAR_PHASE_8;

    vector<double> angle_errors_fft_pre, 
                   angle_errors_fft_phase_pre,
                   angle_errors_fft_post,
                   angle_errors_fft_phase_post,
                   calibration_samples_sine, 
                   calibration_samples_cosine, 
                   calibration_samples_sine_scaled, 
                   calibration_samples_cosine_scaled;

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

    enum DeviceAttribute
    {
        PAGE,
        SEQUENCER_MODE,
        ANGLE_FILT,
        CONVERSION_MODE,
        H8_CTRL,
        SDP_GPIO_CTRL,
        SDP_GPIO0_BUSY,
        SDP_COIL_RS,
        DEVICE_ATTR_COUNT
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

    enum HarmonicRegister
    {
        H1MAG,
        H1PH,
        H2MAG,
        H2PH,
        H3MAG,
        H3PH,
        H8MAG,
        H8PH,
        HARMONIC_REGISTER_COUNT
    };

    enum ConfigurationRegister
    {
        CNVPAGE,
        DIGIO,
        FAULT,
        GENERAL,
        DIGIOEN,
        ANGLECK,
        ECCDCDE,
        ECCDIS,
        CONFIGURATION_REGISTER_COUNT
    };

    enum SensorRegister
    {
        ABSANGLE,
        ANGLEREG,
        ANGLESEC,
        SINE,
        COSINE,
        SECANGLI,
        SECANGLQ,
        RADIUS,
        DIAG1,
        DIAG2,
        TMP0,
        TMP1,
        CNVCNT,
        SENSOR_REGISTER_COUNT
    };

    const char* ChannelIds[CHANNEL_COUNT] = { "rot", "angl", "count", "temp" };
    const char* DeviceIds[DEVICE_COUNT] = { "admt4000", "tmc5240" };
    const char* DeviceAttributes[DEVICE_ATTR_COUNT] = { "page", "sequencer_mode", "angle_filt", "conversion_mode", "h8_ctrl", "sdp_gpio_ctrl", "sdp_gpio0_busy", "sdp_coil_rs" };
    const char* MotorAttributes[MOTOR_ATTR_COUNT] = { "amax", "rotate_vmax", "dmax",
                                                     "disable", "target_pos", "current_pos",
                                                     "ramp_mode" };
    const uint32_t HarmonicRegisters[HARMONIC_REGISTER_COUNT] = { 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C };
    const uint32_t ConfigurationRegisters[CONFIGURATION_REGISTER_COUNT] = { 0x01, 0x04, 0x06, 0x10, 0x12, 0x13, 0x1D, 0x23 };
    const uint32_t ConfigurationPages[CONFIGURATION_REGISTER_COUNT] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 0x02, 0x02, 0x02, 0x02, 0x02 };
    const uint32_t SensorRegisters[SENSOR_REGISTER_COUNT] = { 0x03, 0x05, 0x08, 0x10, 0x11, 0x12, 0x13, 0x18, 0x1D, 0x1E, 0x20, 0x23, 0x14 };
    const uint32_t SensorPages[SENSOR_REGISTER_COUNT] = { UINT32_MAX, UINT32_MAX, UINT32_MAX, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 };

    const char* getChannelId(Channel channel);
    const char* getDeviceId(Device device);
    const char* getDeviceAttribute(DeviceAttribute attribute);
    const char* getMotorAttribute(MotorAttribute attribute);
    const uint32_t getHarmonicRegister(HarmonicRegister registerID);
    const uint32_t getConfigurationRegister(ConfigurationRegister registerID);
    const uint32_t getConfigurationPage(ConfigurationRegister registerID);
    const uint32_t getSensorRegister(SensorRegister registerID);
    const uint32_t getSensorPage(SensorRegister registerID);

    void connectADMT();
    void disconnectADMT();
    int getChannelIndex(const char *deviceName, const char *channelName);
    double getChannelValue(const char *deviceName, const char *channelName, int bufferSize = 1);
    int getDeviceAttributeValue(const char *deviceName, const char *attributeName, double *returnValue);
    int setDeviceAttributeValue(const char *deviceName, const char *attributeName, double writeValue);
    QString calibrate(vector<double> PANG, int cycles = 11, int samplesPerCycle = 256);
    int writeDeviceRegistry(const char *deviceName, uint32_t address, uint32_t value);
    int readDeviceRegistry(const char *deviceName, uint32_t address, uint32_t *returnValue);
    void computeSineCosineOfAngles(const vector<double>& angles);
    uint16_t calculateHarmonicCoefficientMagnitude(double harmonicCoefficient, uint16_t originalValue, string key);
    uint16_t calculateHarmonicCoefficientPhase(double harmonicCoefficient, uint16_t originalValue);
    double getActualHarmonicRegisterValue(uint16_t registerValue, const string key);
    map<string, bool> getFaultRegisterBitMapping(uint16_t registerValue);
    map<string, int> getGeneralRegisterBitMapping(uint16_t registerValue);
    map<string, bool> getDigioenRegisterBitMapping(uint16_t registerValue);
    map<string, bool> getDiag1RegisterBitMapping_Register(uint16_t registerValue);
    map<string, double> getDiag1RegisterBitMapping_Afe(uint16_t registerValue);
    map<string, double> getDiag2RegisterBitMapping(uint16_t registerValue);
    uint16_t setGeneralRegisterBitMapping(uint16_t currentRegisterValue, map<string, int> settings);
    void postcalibrate(vector<double> PANG, int cycleCount, int samplesPerCycle);
    int getAbsAngleTurnCount(uint16_t registerValue);
private:
    iio_context *m_iioCtx;
    iio_buffer *m_iioBuffer;
    Connection *m_conn;
    QString uri;

    unsigned int bitReverse(unsigned int x, int log2n);
    template <typename Iter_T>
    void fft(Iter_T a, Iter_T b, int log2n);
    void performFFT(const vector<double>& angle_errors, vector<double>& angle_errors_fft, vector<double>& angle_errors_fft_phase, int cycleCount);
    int linear_fit(vector<double> x, vector<double> y, double* slope, double* intercept);
    int calculate_angle_error(vector<double> angle_meas, vector<double>& angle_error_ret, double* max_angle_err, int cycleCount, int samplesPerCycle);
    void getPreCalibrationFFT(const vector<double>& PANG, vector<double>& angle_errors_fft_pre, vector<double>& angle_errors_fft_phase_pre, int cycleCount, int samplesPerCycle);
    void getPostCalibrationFFT(const vector<double>& updated_PANG, vector<double>& angle_errors_fft_post, vector<double>& angle_errors_fft_phase_post, int cycleCount, int samplesPerCycle);
};
} // namespace scopy::admt

#endif // ADMTCONTROLLER_H